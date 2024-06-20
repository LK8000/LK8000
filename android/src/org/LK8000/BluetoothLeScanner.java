/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BluetoothLeScanner.java
 * Author: Bruno de Lacheisserie
 */

package org.LK8000;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanRecord;
import android.bluetooth.le.ScanResult;
import android.os.ParcelUuid;

import java.util.List;

public class BluetoothLeScanner extends LeScanCallback {
    private static final String TAG = "BluetoothLeScanner";

    private NativeLeScanCallback listener = null;

    public synchronized void start(NativeLeScanCallback l) {
        listener = l;
        BluetoothHelper.startLeScan(this);
    }

    public synchronized void stop() {
        BluetoothHelper.stopLeScan(this);
        listener = null;
    }

    @Override
    public synchronized void onScanResult(ScanResult result) {
        if (listener == null) {
            return;
        }

        ScanRecord record = result.getScanRecord();
        BluetoothDevice device = result.getDevice();
        if (record == null || device == null) {
            return;
        }

        List<ParcelUuid> puuids = record.getServiceUuids();
        if (puuids != null) {
            for (ParcelUuid puuid : puuids) {
                if (BluetoothGattClientPort.isSupported(puuid.getUuid())) {
                    listener.onLeScan(device.getAddress(), record.getDeviceName());
                }
            }
        } else {
            // workaround : some device don't provide their service uuid...
            listener.onLeScan(device.getAddress(), record.getDeviceName());
        }
    }
}
