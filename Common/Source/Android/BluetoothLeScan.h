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

    typedef std::function<void(WndForm *, const char *, const char *)> callback_t;

public:

    inline BluetoothLeScan(WndForm *pWndForm, callback_t callback) : _pWndForm(pWndForm), _callback(callback), le_callback() {

        if (BluetoothHelper::HasLe(Java::GetEnv())) {
            // Start Bluetooth LE device scan before Open Dialog
            le_callback = BluetoothHelper::StartLeScan(Java::GetEnv(), *this);
        }
    }

    inline ~BluetoothLeScan() {
        if(le_callback) {
            BluetoothHelper::StopLeScan(Java::GetEnv(), le_callback);
        }
    }

    inline void OnLeScan(const char *address, const char *name) override {
        _callback(_pWndForm, address, name);
    }

protected:
    WndForm *_pWndForm;
    callback_t _callback;
    jobject le_callback;
};

#endif //ANDROID_BLUETOOTHLESCAN_H
