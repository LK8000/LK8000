/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sonar.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 26/04/2017.
 */

package org.LK8000;

import android.annotation.TargetApi;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.util.Log;

import com.felhr.usbserial.UsbSerialDevice;

import java.util.HashMap;
import java.util.Map;


@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
public class UsbSerialHelper extends BroadcastReceiver {

    private static final String TAG = "UsbSerialHelper";
    public static final String ACTION_USB_PERMISSION = "org.LK8000.otg.action.USB_PERMISSION";

    private static UsbSerialHelper _instance;

    private Context _Context;

    private HashMap<String, UsbDevice> _AvailableDevices = new HashMap<>();
    private HashMap<UsbDevice, UsbSerialPort> _PendingConnection = new HashMap<>();

    static synchronized void Initialise(Context context) {
        _instance = new UsbSerialHelper();
        _instance.onCreate(context);
    }

    static synchronized void Deinitialise(Context context) {
        if (_instance != null) {
            _instance.onDestroy();
            _instance = null;
        }
    }

    public static boolean isEnabled() {
        return (_instance != null);
    }

    static AndroidPort connect(String name) {
        assert(_instance != null);
        return _instance.connectDevice(name);
    }

    static String[] list() {
        assert(_instance != null);
        return _instance.listDevices();
    }

    @Override
    public synchronized void onReceive(Context context, Intent intent) {
        synchronized (this) {
            String action = intent.getAction();
            if (!intent.hasExtra(UsbManager.EXTRA_DEVICE)) {
                return;
            }

            UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (device == null) {
                return;
            }
            if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                AddAvailable(device);
            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                RemoveAvailable(device);
            } else if (ACTION_USB_PERMISSION.equals(action)) {
                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                    Log.d(TAG, "permission granted for device " + device);

                    UsbSerialPort port = _PendingConnection.get(device);
                    _PendingConnection.remove(device);
                    if(port != null) {
                        UsbManager usbmanager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
                        if(usbmanager != null) {
                            port.open(usbmanager);
                        }
                    }
                }
            }
        }
    }

    private void AddAvailable(UsbDevice device) {
        if (device != null && UsbSerialDevice.isSupported(device)) {
            if (device.getVendorId() == 5840 && device.getProductId() == 2985) {
                Log.v(TAG,"UsbDevice Found : GPSBip " + device);
                _AvailableDevices.put(device.getDeviceName(), device);
            }
        }
    }

    private UsbDevice GetAvailable(String name) {
        for (Map.Entry<String, UsbDevice> entry : _AvailableDevices.entrySet()) {
            if(name.contentEquals(getDeviceName(entry.getValue()))) {
                return entry.getValue();
            }
        }
        return null;
    }

    private void RemoveAvailable(UsbDevice device) {
        Log.v(TAG,"UsbDevice disconnected : " + device);
        _AvailableDevices.remove(device.getDeviceName());
    }


    private void onCreate(Context context) {
        Log.v(TAG, "onCreate()");
        _Context = context;
        UsbManager usbmanager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        if(usbmanager != null) {

            HashMap<String, UsbDevice> devices = usbmanager.getDeviceList();
            for (Map.Entry<String, UsbDevice> entry : devices.entrySet()) {
                AddAvailable(entry.getValue());
            }
        }

        registerReceiver();
    }

    private void onDestroy() {
        Log.v(TAG, "onDestroy()");
        unregisterReceiver();
    }

    private void registerReceiver() {
        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        _Context.registerReceiver(this, filter);
    }

    private void unregisterReceiver() {
        _Context.unregisterReceiver(this);
    }

    private AndroidPort connectDevice(String name) {

        UsbManager usbmanager = (UsbManager) _Context.getSystemService(Context.USB_SERVICE);
        UsbSerialPort port = null;

        if(usbmanager != null) {

            UsbDevice device = GetAvailable(name);
            if (device != null) {
                port = new UsbSerialPort(device);
                if (usbmanager.hasPermission(device)) {
                    port.open(usbmanager);

                } else {
                    _PendingConnection.put(device, port);

                    PendingIntent pi = PendingIntent.getBroadcast(_Context, 0, new Intent(UsbSerialHelper.ACTION_USB_PERMISSION), 0);

                    usbmanager.requestPermission(device, pi);

                }
            }
        }
        return port;
    }

    public String[] listDevices() {

        String[] device_names = new String[_AvailableDevices.size()];
        int n = 0;
        for (Map.Entry<String, UsbDevice> entry : _AvailableDevices.entrySet()) {
            UsbDevice device = entry.getValue();
            device_names[n++] = getDeviceName(device);
        }
        return device_names;
    }

    static String getDeviceName(UsbDevice device) {
        return String.format("%04X:%04X", device.getVendorId(), device.getProductId());
    }
}
