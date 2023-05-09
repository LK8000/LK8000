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

import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.util.Log;

import com.felhr.usbserial.UsbSerialDevice;
import com.felhr.usbserial.UsbSerialInterface;

import java.util.Arrays;


public final class UsbSerialPort implements AndroidPort, UsbSerialInterface.UsbReadCallback {
    private static final String TAG = "UsbSerialPort";

    public UsbSerialPort(UsbDevice device,int baud) {
        _UsbDevice = device;
        _baudRate = baud;
    }

    private final UsbDevice _UsbDevice;
    private UsbDeviceConnection _UsbConnection;
    private UsbSerialDevice _SerialPort;
    private PortListener portListener;
    private InputListener inputListener;
    private int _baudRate;
    private int state = STATE_LIMBO;
    private final SafeDestruct safeDestruct = new SafeDestruct();

    public synchronized void open(UsbManager manager) {
        Log.v(TAG, "open()");

        _UsbConnection = manager.openDevice(_UsbDevice);
        if (_UsbConnection == null) {
            setState(STATE_FAILED);
            return;
        }
        _SerialPort = UsbSerialDevice.createUsbSerialDevice(_UsbDevice, _UsbConnection);
        if (_SerialPort == null) {
            setState(STATE_FAILED);
            return;
        }
        if (!_SerialPort.open()) {
            _SerialPort = null;
            setState(STATE_FAILED);
            return;
        }
        _SerialPort.setBaudRate(getBaudRate());
        _SerialPort.setDataBits(UsbSerialInterface.DATA_BITS_8);
        _SerialPort.setStopBits(UsbSerialInterface.STOP_BITS_1);
        _SerialPort.setParity(UsbSerialInterface.PARITY_NONE);
        _SerialPort.setFlowControl(UsbSerialInterface.FLOW_CONTROL_OFF);
        _SerialPort.read(this);

        setState(STATE_READY);
    }


    public synchronized void close() {
        Log.v(TAG, "close()");
        safeDestruct.beginShutdown();

        if( _SerialPort != null) {
            _SerialPort.close();
            _SerialPort = null;
        }

        if (_UsbConnection != null) {
            _UsbConnection.close();
            _UsbConnection = null;
        }

        safeDestruct.finishShutdown();
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
        return state;
    }

    @Override
    public boolean drain() {
        return false;
    }

    @Override
    public int getBaudRate() {
        return _baudRate;
    }

    @Override
    public synchronized boolean setBaudRate(int baud) {
        _SerialPort.setBaudRate(baud);
        _baudRate = baud;
        return true;
    }

    @Override
    public synchronized int write(byte[] data, int length) {
        _SerialPort.write(Arrays.copyOf(data, length));
        return length;
    }

    @Override
    public void onReceivedData(byte[] arg0) {
        if (arg0.length == 0) {
            error("Disconnected");
            return;
        }

        InputListener listener = inputListener;
        if(listener != null && safeDestruct.Increment()) {
            try {
                listener.dataReceived(arg0, arg0.length);
            } finally {
                safeDestruct.Decrement();
            }
        }
    }

    private void stateChanged() {
        PortListener portListener = this.portListener;
        if (portListener != null && safeDestruct.Increment()) {
            try {
                portListener.portStateChanged();
            } finally {
                safeDestruct.Decrement();
            }
        }
    }

    private void setState(int newState) {
        if (newState == state)
            return;

        state = newState;
        stateChanged();
    }

    private void error(String msg) {
        state = STATE_FAILED;

        PortListener portListener = this.portListener;
        if (portListener != null && safeDestruct.Increment()) {
            try {
                portListener.portError(msg);
            } finally {
                safeDestruct.Decrement();
            }
        }
    }
}
