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
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Looper;
import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;
import android.os.Build;
import android.os.Environment;
import android.os.PowerManager;
import android.os.Handler;
import android.os.Message;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;
import android.provider.Settings;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.TimeZone;

import com.onyx.android.sdk.api.device.epd.EpdController;

public class LK8000 extends Activity {
  private static final String TAG = "LK8000";

  /**
   * Hack: this is set by onCreate(), to support the "testing"
   * package.
   */
  protected static Class<?> serviceClass;
  private static NativeView nativeView;

  PowerManager.WakeLock wakeLock;

  BatteryReceiver batteryReceiver;

  @Override protected void onCreate(Bundle savedInstanceState) {
    if (serviceClass == null)
      serviceClass = MyService.class;

    super.onCreate(savedInstanceState);

    Log.d(TAG, "ABI=" + Build.CPU_ABI);
    Log.d(TAG, "PRODUCT=" + Build.PRODUCT);
    Log.d(TAG, "MANUFACTURER=" + Build.MANUFACTURER);
    Log.d(TAG, "MODEL=" + Build.MODEL);
    Log.d(TAG, "DEVICE=" + Build.DEVICE);
    Log.d(TAG, "BOARD=" + Build.BOARD);
    Log.d(TAG, "FINGERPRINT=" + Build.FINGERPRINT);


    try {
      // change Onyx eInk device to 'A2' update mode
      EpdController.applyApplicationFastMode(getApplicationInfo().packageName, true, true);
    } catch (Exception ignored) { }

    if (!Loader.loaded) {
      TextView tv = new TextView(this);
      tv.setText("Failed to load the native LK8000 libary.\n" +
                 "Report this problem to us, and include the following information:\n" +
                 "ABI=" + Build.CPU_ABI + "\n" +
                 "PRODUCT=" + Build.PRODUCT + "\n" +
                 "FINGERPRINT=" + Build.FINGERPRINT + "\n" +
                 "error=" + Loader.error);
      setContentView(tv);
      return;
    }

    initialiseNative();

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
    tv.setText("Loading LK8000...");
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
    stopService(new Intent(this, serviceClass));

    TextView tv = new TextView(LK8000.this);
    tv.setText("Shutting down LK8000...");
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
      tv.setText("External storage is not available (state='" + state
                 + "').  Please turn off USB storage.");
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

    Intent intent = new Intent(this, serviceClass);

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

  /**
   * permissions request code
   */
  private final static int REQUEST_CODE_ASK_PERMISSIONS = 1;
  /**
   * Permissions that need to be explicitly requested from end user.
   */
  private static final String[] REQUIRED_SDK_PERMISSIONS = new String[]{
          Manifest.permission.BLUETOOTH,
          Manifest.permission.BLUETOOTH_ADMIN,
          Manifest.permission.WRITE_EXTERNAL_STORAGE,
          Manifest.permission.ACCESS_FINE_LOCATION,
          Manifest.permission.WAKE_LOCK,
          Manifest.permission.INTERNET,
          Manifest.permission.ACCESS_NETWORK_STATE,
          Manifest.permission.VIBRATE
  };

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
      final int[] grantResults = new int[REQUIRED_SDK_PERMISSIONS.length];
      Arrays.fill(grantResults, PackageManager.PERMISSION_GRANTED);
      onRequestPermissionsResult(REQUEST_CODE_ASK_PERMISSIONS, REQUIRED_SDK_PERMISSIONS,
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
    LKDistribution.copyLKDistribution(this.getApplication(), false);

    onRuntimePermissionGrantedNative();
  }

  static public int getSystemUTCOffset() {
      return TimeZone.getDefault().getOffset(System.currentTimeMillis()) / 1000;
  }

  private native void onRuntimePermissionDenied();
  private native void onRuntimePermissionGrantedNative();

  private native void initialiseNative();
  private native void deinitialiseNative();

}
