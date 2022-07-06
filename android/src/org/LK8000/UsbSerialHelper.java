/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sonar.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 26/04/2017.
 */

package org.LK8000;

import static android.content.pm.PackageManager.FEATURE_USB_HOST;

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

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class UsbSerialHelper extends BroadcastReceiver {

    private static final String TAG = "UsbSerialHelper";
    public static final String ACTION_USB_PERMISSION = "org.LK8000.otg.action.USB_PERMISSION";

    private static UsbSerialHelper _instance;

    private Context _Context;

    private final HashMap<String, UsbDevice> _AvailableDevices = new HashMap<>();
    private final HashMap<UsbDevice, UsbSerialPort> _PendingConnection = new HashMap<>();

    static synchronized void Initialise(Context context) {
        if (context.getPackageManager().hasSystemFeature(FEATURE_USB_HOST)) {
            _instance = new UsbSerialHelper();
            _instance.onCreate(context);
        }
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

    static AndroidPort connect(String name,int baud) {
        assert(_instance != null);
        return _instance.connectDevice(name,baud);
    }

    static String[] list() {
        assert(_instance != null);
        return _instance.listDevices();
    }

    private static long[] supported_ids = createTable(
            createDevice(0x16D0, 0x0BA9), // GPSBip
            createDevice(0x0403, 0x6015), // Digifly AIR (FT X-Series)
            createDevice(0x0483, 0x5740), // SoftRF Dongle
            createDevice(0x239A, 0x8029), // SoftRF Badge
            createDevice(0x2341, 0x804d), // SoftRF Academy
            createDevice(0x1d50, 0x6089), // SoftRF ES
            createDevice(0x2e8a, 0x000a), // SoftRF Lego
            createDevice(0x2e8a, 0xf00a), // SoftRF Lego

            createDevice(0x0403, 0x6001), // FT232AM, FT232BM, FT232R FT245R,
            createDevice(0x0403, 0x6010), // FT2232D, FT2232H
            createDevice(0x0403, 0x6011), // FT4232H
            createDevice(0x0403, 0x6014), // FT232H

            createDevice(0x10C4, 0xEA60), // CP210x
            createDevice(0x1A86, 0x55D4), // CH9102

            createDevice(0x067B, 0x2303) // PL2303
    );

    static long createDevice(int vendorId, int productId) {
        return ((long) vendorId) << 32 | (productId & 0xFFFF_FFFFL);
    }

    static long[] createTable(long... entries) {
        Arrays.sort(entries);
        return entries;
    }

    static boolean exists(long[] devices, int vendorId, int productId) {
        return Arrays.binarySearch(devices, createDevice(vendorId, productId)) >= 0;
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
                    if (port != null) {
                        UsbManager usbmanager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
                        if (usbmanager != null) {
                            port.open(usbmanager);
                        }
                    } else {
                        AddAvailable(device);
                    }
                }
            }
        }
    }

    private void AddAvailable(UsbDevice device) {
        if (device != null && UsbSerialDevice.isSupported(device)) {
            int vid = device.getVendorId();
            int pid = device.getProductId();

            if (exists(supported_ids, vid, pid)) {
                try {
                    Log.v(TAG, "UsbDevice Found : " + device);
                    _AvailableDevices.put(device.getDeviceName(), device);
                } catch (SecurityException ignored) {
                    // Permission may be required to get serial number if app targets SDK >= 29
                    UsbManager usbmanager = (UsbManager) _Context.getSystemService(Context.USB_SERVICE);
                    Log.v(TAG, "UsbDevice request permission : " + device);
                    requestPermission(usbmanager, device);
                }
            } else {
                Log.v(TAG, "Unsupported UsbDevice : " + device);
            }
        }
    }

    private UsbDevice GetAvailable(String name) {
        for (Map.Entry<String, UsbDevice> entry : _AvailableDevices.entrySet()) {
            if(name.contentEquals(getDeviceName(entry.getValue()))) {
                return entry.getValue();
            }
        }

        // for backward compatibility try to find device using it's old name (VID:PID)
        for (Map.Entry<String, UsbDevice> entry : _AvailableDevices.entrySet()) {
            if(name.contentEquals(getLegacyDeviceName(entry.getValue()))) {
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
            try {
                HashMap<String, UsbDevice> devices = usbmanager.getDeviceList();
                for (Map.Entry<String, UsbDevice> entry : devices.entrySet()) {
                    AddAvailable(entry.getValue());
                }
            } catch (NullPointerException e) {
                Log.e(TAG, "onCreate()", e);
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

    private AndroidPort connectDevice (String name , int baud) {

        UsbManager usbmanager = (UsbManager) _Context.getSystemService(Context.USB_SERVICE);
        UsbSerialPort port = null;

        if(usbmanager != null) {

            UsbDevice device = GetAvailable(name);
            if (device != null) {
                port = new UsbSerialPort(device,baud);
                if (usbmanager.hasPermission(device)) {
                    port.open(usbmanager);

                } else {
                    _PendingConnection.put(device, port);
                    requestPermission(usbmanager, device);
                }
            }
        }
        return port;
    }

    private void requestPermission(UsbManager usbmanager, UsbDevice device) {
        int intent_flags = 0;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            intent_flags |= PendingIntent.FLAG_IMMUTABLE;
        }
        PendingIntent pi = PendingIntent.getBroadcast(_Context, 0, new Intent(UsbSerialHelper.ACTION_USB_PERMISSION), intent_flags);

        usbmanager.requestPermission(device, pi);
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
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            return String.format("%s_%s_%s", device.getManufacturerName(), device.getProductName(), device.getSerialNumber());
        } else {
            return getLegacyDeviceName(device);
        }
    }

    static String getLegacyDeviceName(UsbDevice device) {
        return String.format("%04X:%04X", device.getVendorId(), device.getProductId());
    }

}
