package org.LK8000;

import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.util.Log;

import java.util.List;

abstract class LeScanCallback extends ScanCallback {
    static final String TAG = "LeScanCallback";

    public abstract void onScanResult(ScanResult result);

    @Override
    public void onBatchScanResults(List<ScanResult> results) {
        for (ScanResult r : results) {
            onScanResult(r);
        }
    }

    @Override
    public void onScanResult(int callbackType, ScanResult result) {
        onScanResult(result);
    }

    @Override
    public void onScanFailed(int errorCode) {
        Log.e(TAG, "onScanFailed " + errorCode);
    }
}
