/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * Created by Bruno de Lacheisserie on 4/20/17.
 */

#ifndef ANDROID_BLUETOOTHLESCAN_H
#define ANDROID_BLUETOOTHLESCAN_H


#include "externs.h"
#include "device.h"
#include "Android/LeScanCallback.hpp"
#include <functional>

class WndForm;

class BluetoothLeScan : public LeScanCallback {

    using callback_t = std::function<void(WndForm *, const char *, const char *)>;

public:

    BluetoothLeScan(WndForm *pWndForm, callback_t callback) : _pWndForm(pWndForm), _callback(callback), le_callback() {

        JNIEnv *env = Java::GetEnv();
        if (BluetoothHelper::HasLe(env)) {
            // Start Bluetooth LE device scan before Open Dialog
            le_callback = BluetoothHelper::StartLeScan(env, *this);
        }
    }

    ~BluetoothLeScan() {
        if(le_callback) {
            BluetoothHelper::StopLeScan(le_callback);
        }
    }

    void OnLeScan(const char *address, const char *name) override {
        _callback(_pWndForm, address, name);
    }

protected:
    WndForm *_pWndForm;
    callback_t _callback;
    Java::LocalObject le_callback;
};

#endif //ANDROID_BLUETOOTHLESCAN_H
