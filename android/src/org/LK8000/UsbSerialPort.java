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
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.util.Log;
import android.os.Build;

import com.felhr.usbserial.UsbSerialDevice;
import com.felhr.usbserial.UsbSerialInterface;


@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
public final class UsbSerialPort implements AndroidPort {
    private static final String TAG = "UsbSerialPort";

    public UsbSerialPort(UsbDevice device) {
        _UsbDevice = device;
    }

    private UsbDevice _UsbDevice;
    private UsbDeviceConnection _UsbConnection;
    private UsbSerialDevice _SerialPort;
    private PortListener portListener;
    private InputListener inputListener;

    public synchronized void open(UsbManager manager) {
        Log.v(TAG, "open()");

        _UsbConnection = manager.openDevice(_UsbDevice);
        if (_UsbConnection != null) {
            _SerialPort = UsbSerialDevice.createUsbSerialDevice(_UsbDevice, _UsbConnection);
            if (_SerialPort != null) {
                if (_SerialPort.open()) {
                    _SerialPort.setBaudRate(9600);
                    _SerialPort.setDataBits(UsbSerialInterface.DATA_BITS_8);
                    _SerialPort.setStopBits(UsbSerialInterface.STOP_BITS_1);
                    _SerialPort.setParity(UsbSerialInterface.PARITY_NONE);
                    _SerialPort.setFlowControl(UsbSerialInterface.FLOW_CONTROL_OFF);
                    _SerialPort.read(_ReadCallback);
                }
                stateChanged();
            }
        }
    }


    public synchronized void close() {
        Log.v(TAG, "close()");
        if( _SerialPort != null) {
            _SerialPort.close();
            _SerialPort = null;
        }

        if (_UsbConnection != null) {
            _UsbConnection.close();
            _UsbConnection = null;
        }

        stateChanged();
    }

    @Override
    public void setListener(PortListener listener) {
        portListener = listener;
    }

    @Override
    public void setInputListener(InputListener listener) {
        inputListener = listener;
    }

    @Override
    public int getState() {
        return (_SerialPort != null) ? STATE_READY : STATE_LIMBO;
    }

    @Override
    public boolean drain() {
        return false;
    }

    @Override
    public int getBaudRate() {
        return 0;
    }

    @Override
    public boolean setBaudRate(int baud) {
        return false;
    }

    @Override
    public int write(byte[] data, int length) {
        return 0;
    }

    private UsbSerialInterface.UsbReadCallback _ReadCallback = new UsbSerialInterface.UsbReadCallback() {
        @Override
        public void onReceivedData(byte[] arg0) {
            InputListener listner = inputListener;
            if(listner != null) {
                listner.dataReceived(arg0, arg0.length);
            }
        }
    };

    protected final void stateChanged() {
        PortListener portListener = this.portListener;
        if (portListener != null)
            portListener.portStateChanged();
    }
}
