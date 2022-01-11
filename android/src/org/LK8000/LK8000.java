/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

package org.LK8000;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.provider.Settings;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import org.LK8000.QRCode.QRCodeScannerActivity;

import java.io.File;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.TimeZone;

public class LK8000 extends Activity {
  private static final String TAG = "LK8000";

  private static NativeView nativeView;

  PowerManager.WakeLock wakeLock;

  BatteryReceiver batteryReceiver;

  @Override protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    Log.d(TAG, "ABI=" + Build.CPU_ABI);
    Log.d(TAG, "PRODUCT=" + Build.PRODUCT);
    Log.d(TAG, "MANUFACTURER=" + Build.MANUFACTURER);
    Log.d(TAG, "MODEL=" + Build.MODEL);
    Log.d(TAG, "DEVICE=" + Build.DEVICE);
    Log.d(TAG, "BOARD=" + Build.BOARD);
    Log.d(TAG, "FINGERPRINT=" + Build.FINGERPRINT);

    if (!Loader.loaded) {
      TextView tv = new TextView(this);
      tv.setText(getString(R.string.error_native_library) + "\n" +
                 "ABI=" + Build.CPU_ABI + "\n" +
                 "PRODUCT=" + Build.PRODUCT + "\n" +
                 "FINGERPRINT=" + Build.FINGERPRINT + "\n" +
                 "error=" + Loader.error);
      setContentView(tv);
      return;
    }

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
      AudioManager myAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
      final String sampleRateStr = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
      final int sampleRate = Integer.parseInt(sampleRateStr);
      final String framesPerBurstStr = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
      final int framesPerBurst = Integer.parseInt(framesPerBurstStr);
      setDefaultStreamValues(sampleRate, framesPerBurst);
    }

    initialiseNative();

    setHasKeyboard(hasKeyboard());

    NetUtil.initialise(this);
    InternalGPS.Initialize();
    NonGPSSensors.Initialize();

    IOIOHelper.onCreateContext(this);

    BluetoothHelper.Initialize(this);
    DownloadUtil.Initialise(getApplicationContext());
    UsbSerialHelper.Initialise(this);

    SoundUtil.Initialise();

    // fullscreen mode
    requestWindowFeature(Window.FEATURE_NO_TITLE);
    getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN|
                         WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

    /* Workaround for layout problems in Android KitKat with immersive full
       screen mode: Sometimes the content view was not initialized with the
       correct size, which caused graphics artifacts. */
    if (android.os.Build.VERSION.SDK_INT >= 19) {
      getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN|
                           WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS|
                           WindowManager.LayoutParams.FLAG_LAYOUT_INSET_DECOR|
                           WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
    }

    enableImmersiveModeIfSupported();

    TextView tv = new TextView(this);
    tv.setText(R.string.loading);
    setContentView(tv);

    batteryReceiver = new BatteryReceiver();
    registerReceiver(batteryReceiver, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));

    SharedPreferences settings = getSharedPreferences("LK8000", 0);
    int screenOrientation = settings.getInt("screenOrientation", 0);
    switch (screenOrientation ) {
      case 0 :
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR);
        break;
      case 1:
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT );
        break;
      case 2:
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        break;
      case 3:
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT);
        break;
      case 4:
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
        break;
      default:
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR);
    }

    getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

  }

  private void quit() {
    Log.d(TAG, "in quit()");

    nativeView = null;

    Log.d(TAG, "stopping service");
    stopService(new Intent(this, MyService.class));

    TextView tv = new TextView(LK8000.this);
    tv.setText(R.string.quit);
    setContentView(tv);

    Log.d(TAG, "finish()");
    finish();
  }

  final Handler quitHandler = new Handler(Looper.getMainLooper()) {
    public void handleMessage(Message msg) {
      quit();
    }
  };

  final Handler errorHandler = new Handler(Looper.getMainLooper()) {
    public void handleMessage(Message msg) {
      nativeView = null;
      TextView tv = new TextView(LK8000.this);
      tv.setText(msg.obj.toString());
      setContentView(tv);

    }
  };

  public void initSDL() {
    if (!Loader.loaded)
      return;

    /* check if external storage is available; LK8000 doesn't work as
       long as external storage is being forwarded to a PC */
    String state = Environment.getExternalStorageState();
    Log.d(TAG, "getExternalStorageState() = " + state);
    if (!Environment.MEDIA_MOUNTED.equals(state)) {
      TextView tv = new TextView(this);
      tv.setText(R.string.error_external_storage);
      setContentView(tv);
      return;
    }

    nativeView = new NativeView(this, quitHandler, errorHandler);
    setContentView(nativeView);
    // Receive keyboard events
    nativeView.setFocusableInTouchMode(true);
    nativeView.setFocusable(true);
    nativeView.requestFocus();

    // Obtain an instance of the Android PowerManager class
    PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);

    // Create a WakeLock instance to keep the screen from timing out
    final String LKTAG = "LK8000:" + BuildConfig.BUILD_TYPE;
    wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK|
            PowerManager.ACQUIRE_CAUSES_WAKEUP, LKTAG);


    // Activate the WakeLock
    wakeLock.acquire();
  }

  @Override protected void onPause() {
    if (nativeView != null)
      nativeView.onPause();
    super.onPause();
  }

  private void getHapticFeedbackSettings() {
    boolean hapticFeedbackEnabled;
    try {
      hapticFeedbackEnabled =
        (Settings.System.getInt(getContentResolver(),
                                Settings.System.HAPTIC_FEEDBACK_ENABLED)) != 0;
    } catch (Settings.SettingNotFoundException e) {
      hapticFeedbackEnabled = false;
    }

    if (nativeView != null)
      nativeView.setHapticFeedback(hapticFeedbackEnabled);
  }

  private void enableImmersiveModeIfSupported() {
    // Set / Reset the System UI visibility flags for Immersive Full Screen Mode, if supported
    if (android.os.Build.VERSION.SDK_INT >= 19)
      ImmersiveFullScreenMode.enable(getWindow().getDecorView());
  }

  @Override protected void onResume() {
    super.onResume();

    Intent intent = new Intent(this, MyService.class);

    try {
      // startForegroundService was introduced in Android Oreo (API 26).
      // Use reflection to maintain compatibility with API < 14.
      Method method = Context.class.getMethod("startForegroundService", Intent.class);
      method.invoke(this, intent);

    } catch (Throwable x) {
      // fallback to start #startService on Android API < 26
      startService(intent);
	}


    if (nativeView != null)
      nativeView.onResume();
    else
      initSDL();
    getHapticFeedbackSettings();
  }

  @Override protected void onDestroy()
  {
    Log.d(TAG, "in onDestroy()");

    if(batteryReceiver != null) {
      unregisterReceiver(batteryReceiver);
      batteryReceiver = null;
    }

    DownloadUtil.Deinitialise(getApplicationContext());
    UsbSerialHelper.Deinitialise(this);
    SoundUtil.Deinitialise();

    if (nativeView != null) {
      nativeView.exitApp();
      nativeView = null;
    }

    // Release the WakeLock instance to re-enable screen timeouts
    if (wakeLock != null) {
      wakeLock.release();
      wakeLock = null;
    }

    IOIOHelper.onDestroyContext();

    deinitialiseNative();

    super.onDestroy();
    Log.d(TAG, "System.exit()");
    System.exit(0);
  }

  @Override public boolean onKeyDown(int keyCode, final KeyEvent event) {
    // Overrides Back key to use in our app
    if (nativeView != null) {
      nativeView.onKeyDown(keyCode, event);
      return true;
    } else
      return super.onKeyDown(keyCode, event);
  }

  @Override public boolean onKeyUp(int keyCode, final KeyEvent event) {
    if (nativeView != null) {
      nativeView.onKeyUp(keyCode, event);
      return true;
    } else
      return super.onKeyUp(keyCode, event);
  }

  @Override public void onWindowFocusChanged(boolean hasFocus) {
    enableImmersiveModeIfSupported();
    super.onWindowFocusChanged(hasFocus);
  }

  @Override public boolean dispatchTouchEvent(final MotionEvent ev) {
    if (nativeView != null) {
      nativeView.onTouchEvent(ev);
      return true;
    } else
      return super.dispatchTouchEvent(ev);
  }

  @Override
  public void onConfigurationChanged(@NonNull Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
    boolean hasKeyboard = hasKeyboard();
    if (nativeView != null) {
      nativeView.setHasKeyboard(hasKeyboard);
    }
    setHasKeyboard(hasKeyboard);
  }

  private boolean hasKeyboard() {
    return getResources().getConfiguration().keyboard != Configuration.KEYBOARD_NOKEYS;
  }

  private native void setHasKeyboard(boolean b);

  /**
   * permissions request code
   */
  private final static int REQUEST_CODE_ASK_PERMISSIONS = 1;
  /**
   * Permissions that need to be explicitly requested from end user.
   */
  static final ArrayList<String> REQUIRED_SDK_PERMISSIONS = new ArrayList<>();

  {
    REQUIRED_SDK_PERMISSIONS.add(Manifest.permission.BLUETOOTH);
    REQUIRED_SDK_PERMISSIONS.add(Manifest.permission.BLUETOOTH_ADMIN);
    REQUIRED_SDK_PERMISSIONS.add(Manifest.permission.ACCESS_FINE_LOCATION);
    REQUIRED_SDK_PERMISSIONS.add(Manifest.permission.WAKE_LOCK);
    REQUIRED_SDK_PERMISSIONS.add(Manifest.permission.INTERNET);
    REQUIRED_SDK_PERMISSIONS.add(Manifest.permission.ACCESS_NETWORK_STATE);
    REQUIRED_SDK_PERMISSIONS.add(Manifest.permission.VIBRATE);

    if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
      REQUIRED_SDK_PERMISSIONS.add(Manifest.permission.FOREGROUND_SERVICE);
    }
  }


  /**
   * Checks the dynamically-controlled permissions and requests missing permissions from end user.
   */
  protected void checkPermissions() {
    final List<String> missingPermissions = new ArrayList<String>();
    // check all required dynamic permissions
    for (final String permission : REQUIRED_SDK_PERMISSIONS) {
      final int result = ContextCompat.checkSelfPermission(this, permission);
      if (result != PackageManager.PERMISSION_GRANTED) {
        missingPermissions.add(permission);
      }
    }
    if (!missingPermissions.isEmpty()) {
      // request all missing permissions
      final String[] permissions = missingPermissions
              .toArray(new String[missingPermissions.size()]);
      ActivityCompat.requestPermissions(this, permissions, REQUEST_CODE_ASK_PERMISSIONS);
    } else {
      final int[] grantResults = new int[REQUIRED_SDK_PERMISSIONS.size()];
      Arrays.fill(grantResults, PackageManager.PERMISSION_GRANTED);
      final String[] permissions = REQUIRED_SDK_PERMISSIONS
              .toArray(new String[REQUIRED_SDK_PERMISSIONS.size()]);
      onRequestPermissionsResult(REQUEST_CODE_ASK_PERMISSIONS, permissions,
              grantResults);
    }
  }

  @Override
  public void onRequestPermissionsResult(int requestCode, @NonNull String permissions[],
                                         @NonNull int[] grantResults) {
    switch (requestCode) {
      case REQUEST_CODE_ASK_PERMISSIONS:
        for (int index = permissions.length - 1; index >= 0; --index) {
          if (grantResults[index] != PackageManager.PERMISSION_GRANTED) {
            // exit the app if one permission is not granted
            onRuntimePermissionDenied();
            return;
          }
        }
        // all permissions are granted
        onRuntimePermissionGranted();
        break;
    }
  }

  private void onRuntimePermissionGranted() {
    LKDistribution.copyLKDistribution(this, false);

    onRuntimePermissionGrantedNative();
  }

  static public int getSystemUTCOffset() {
      return TimeZone.getDefault().getOffset(System.currentTimeMillis()) / 1000;
  }

  private native void onRuntimePermissionDenied();
  private native void onRuntimePermissionGrantedNative();

  private native void initialiseNative();
  private native void deinitialiseNative();

  private static native void setDefaultStreamValues(int SampleRate, int FramesPerBurst);

  private final int SCAN_QRCODE = 0;

  void scanQRCode() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      Intent intent = new Intent(this, QRCodeScannerActivity.class);
      startActivityForResult(intent, SCAN_QRCODE);
    }
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
    super.onActivityResult(requestCode, resultCode, data);
    if (requestCode == SCAN_QRCODE) {
      if (resultCode == Activity.RESULT_OK && data != null) {
        String data_string = data.getStringExtra("data");
        if (data_string != null) {
          loadQRCodeData(data_string);
        }
      }
    }
  }

  private static native void loadQRCodeData(String data_string);

  void shareFile(String path) {
    try {
      Context context = getApplicationContext();
      File file = new File(path);
      String mimeType = FileUtils.getDocumentType(file);
      Uri uri = LKFileProvider.getUriForFile(context, file);

      Intent sendIntent = new Intent(Intent.ACTION_SEND);
      sendIntent.setDataAndType(uri, mimeType);
      sendIntent.putExtra(Intent.EXTRA_STREAM, LKFileProvider.getUriForFile(context, file));
      sendIntent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);

      startActivity(Intent.createChooser(sendIntent, null));

    } catch (Exception e) {
      // Todo: display user friendly error
      e.printStackTrace();
    }
  }
}
