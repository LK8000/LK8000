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

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.os.Build;
import android.util.Log;

import androidx.annotation.NonNull;

import java.util.Arrays;
import java.util.Set;
import java.util.UUID;

/**
 * AndroidPort implementation for Bluetooth Low Energy devices using the
 * GATT protocol.
 */
public class BluetoothGattClientPort
    extends BluetoothGattCallback
    implements AndroidPort  {
  private static final String TAG = "BluetoothGattClientPort";

  private static final int STATE_READY = 0;
  private static final int STATE_FAILED = 1;
  private static final int STATE_LIMBO = 2;

  private static final UUID GENERIC_ACCESS_SERVICE =
          UUID.fromString("00001800-0000-1000-8000-00805F9B34FB");

  private static final UUID DEVICE_NAME_CHARACTERISTIC_UUID =
          UUID.fromString("00002A00-0000-1000-8000-00805F9B34FB");

  private static final UUID CLIENT_CHARACTERISTIC_CONFIGURATION =
          UUID.fromString("00002902-0000-1000-8000-00805F9B34FB");
  /**
   * The HM-10 and compatible bluetooth modules use a GATT characteristic
   * with this UUID for sending and receiving data.
   */
  private static final UUID HM10_SERVICE =
          UUID.fromString("0000FFE0-0000-1000-8000-00805F9B34FB");

  private static final UUID RX_TX_CHARACTERISTIC_UUID =
          UUID.fromString("0000FFE1-0000-1000-8000-00805F9B34FB");


  private static final Set<UUID> supported_service = Set.of(
          UUID.fromString("0000180D-0000-1000-8000-00805F9B34FB"),  // Heart Rate
          UUID.fromString("0000181A-0000-1000-8000-00805F9B34FB"),  // Environmental Sensing Service
          UUID.fromString("00001819-0000-1000-8000-00805F9B34FB"),  // Location and Navigation service
          HM10_SERVICE);  // HM-10 and compatible bluetooth modules

  public static boolean isServiceSupported(UUID uuid) {
    return supported_service.contains(uuid);
  }

  /* Maximum number of milliseconds to wait for disconnected state after
     calling BluetoothGatt.disconnect() in close() */
  private static final int DISCONNECT_TIMEOUT = 500;

  private PortListener portListener = null;
  private volatile InputListener listener = null;

  private BluetoothGatt gatt = null;
  private BluetoothGattCharacteristic hm10DataCharacteristic;
  private BluetoothGattCharacteristic deviceNameCharacteristic;
  private volatile boolean shutdown = false;

  private int maxChunkSize = 20;

  private final HM10WriteBuffer writeBuffer = new HM10WriteBuffer();

  private volatile int portState = STATE_LIMBO;

  private final Object gattStateSync = new Object();
  private int gattState = BluetoothGatt.STATE_DISCONNECTED;

  private ScanCallback callback = null;

  private final AsyncCompletionQueue queueCommand = new AsyncCompletionQueue();

  void startLeScan(Context context, String address) {
    if (callback == null) {
      callback = new LeScanCallback() {
        @Override
        public void onScanResult(ScanResult result) {
          BluetoothDevice device = result.getDevice();
          if (device.getAddress().equals(address)) {
            stopLeScan();
            connectDevice(context, device);
          }
        }
      };
      BluetoothHelper.startLeScan(address, callback);
    }
  }

  void stopLeScan() {
    ScanCallback tmp = callback;
    callback = null;
    if (tmp != null) {
      BluetoothHelper.stopLeScan(tmp);
    }
  }

  public BluetoothGattClientPort(Context context, BluetoothDevice device) {
    startLeScan(context, device.getAddress());
  }

  private void connectDevice(Context context, BluetoothDevice device) {
    try {
      if (Build.VERSION.SDK_INT > Build.VERSION_CODES.O) {
        // handler usage is broken until SDK 27
        // https://android.googlesource.com/platform/frameworks/base/+/eb6b3da4fc54ca4cfcbb8ee3b927391eed981725%5E%21/#F0
        gatt = device.connectGatt(context, true, this,
                BluetoothDevice.TRANSPORT_LE, BluetoothDevice.PHY_LE_1M_MASK,
                queueCommand.queueHandler);
      }
      else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
        gatt = device.connectGatt(context, true, this,
                BluetoothDevice.TRANSPORT_LE);
      }
      else {
        gatt = device.connectGatt(context, true, this);
      }
    } catch (SecurityException e) {
      e.printStackTrace();
    }
  }

  @SuppressLint("MissingPermission")
  @Override
  public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
    int newPortState = STATE_LIMBO;
    if (BluetoothProfile.STATE_CONNECTED == newState) {
      if (!gatt.discoverServices()) {
        Log.e(TAG, "Discovering GATT services request failed");
        newPortState = STATE_FAILED;
      }
    } else {
      hm10DataCharacteristic = null;
      deviceNameCharacteristic = null;
      if ((BluetoothProfile.STATE_DISCONNECTED == newState) && !shutdown) {
        if (!gatt.connect()) {
          Log.w(TAG,
              "Received GATT disconnected event, and reconnect attempt failed");
          newPortState = STATE_FAILED;
        }
      }
    }
    writeBuffer.clear();
    portState = newPortState;
    stateChanged();
    synchronized (gattStateSync) {
      gattState = newState;
      gattStateSync.notifyAll();
    }
  }

  @SuppressLint("MissingPermission")
  void requestMtu(BluetoothGatt gatt) {
    maxChunkSize = 20; // default mtu - 3
    if (!gatt.requestMtu(517)) {
      queueCommand.completed();
    }
  }

  @Override
  public void onServicesDiscovered(BluetoothGatt gatt, int status) {
    if (BluetoothGatt.GATT_SUCCESS == status) {
      maxChunkSize = 20; // default mtu - 3
      queueCommand.add(() -> requestMtu(gatt));
      queueCommand.add(() -> configureCharacteristics(gatt));
    } else {
      Log.e(TAG, "Discovering GATT services failed");
      portState = STATE_FAILED;
      stateChanged();
    }
  }

  @Override
  public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
    super.onMtuChanged(gatt, mtu, status);
    if (BluetoothGatt.GATT_SUCCESS == status) {
      maxChunkSize = mtu - 3;
    }
    queueCommand.completed();
  }

  private boolean doEnableNotification(UUID service, UUID characteristic) {
    if (listener != null) {
      return listener.doEnableNotification(service, characteristic);
    }
    return false;
  }

  private void configureCharacteristics(BluetoothGatt gatt) {
    queueCommand.completed();

    BluetoothGattService hm10Service = gatt.getService(HM10_SERVICE);
    if (hm10Service != null) {
      hm10DataCharacteristic = hm10Service.getCharacteristic(RX_TX_CHARACTERISTIC_UUID);
    } else {
      hm10DataCharacteristic = null;
    }

    for (BluetoothGattService service : gatt.getServices()) {
      for (BluetoothGattCharacteristic characteristic : service.getCharacteristics()) {
        if (doEnableNotification(service.getUuid(), characteristic.getUuid())) {
          int properties = characteristic.getProperties();
          if (( properties & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0) {
            queueCommand.add(() -> enableNotification(gatt, characteristic));
          }
          else if (( properties & BluetoothGattCharacteristic.PROPERTY_READ) != 0) {
            queueCommand.add(() -> readCharacteristic(gatt, characteristic));
          }
        }
      }
    }
    portState = STATE_READY;
    stateChanged();
  }

  @SuppressLint("MissingPermission")
  private void readCharacteristic(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
    if (!gatt.readCharacteristic(characteristic)) {
      Log.e(TAG, "GATT characteristic read request failed");
      queueCommand.completed();
    }
  }

  @SuppressLint("MissingPermission")
  private void enableNotification(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
    if (characteristic == null) {
      queueCommand.completed();
      return;
    }
    if (gatt.setCharacteristicNotification(characteristic, true)) {
      enableDescriptor(gatt, characteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIGURATION));
    } else {
      queueCommand.completed();
    }
  }

  @SuppressLint("MissingPermission")
  private void enableDescriptor(BluetoothGatt gatt, BluetoothGattDescriptor descriptor) {
    if (descriptor == null) {
      queueCommand.completed();
      return;
    }
    if (!LeGattHelper.writeDescriptor(gatt, descriptor, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)) {
      queueCommand.completed();
    }
  }

  @Override
  public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
    if (BluetoothGatt.GATT_SUCCESS == status) {
      onCharacteristicChanged(gatt, characteristic, characteristic.getValue());
    }
    // Notify Completion
    queueCommand.completed();
  }

  @Override
  public void onCharacteristicRead(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattCharacteristic characteristic, @NonNull byte[] value, int status) {
    if (BluetoothGatt.GATT_SUCCESS == status) {
      onCharacteristicChanged(gatt, characteristic, value);
    }
    // Notify Completion
    queueCommand.completed();
  }

  @Override
  public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
    if (BluetoothGatt.GATT_SUCCESS == status) {
      if (characteristic.equals(hm10DataCharacteristic)) {
        writeBuffer.beginWriteNextChunk(gatt, hm10DataCharacteristic);
      }
    }
    // Notify Completion
    queueCommand.completed();
  }

  @Override
  public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
    // Notify Completion
    queueCommand.completed();
  }

  @Override
  public void onDescriptorRead(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattDescriptor descriptor, int status, @NonNull byte[] value) {
    // Notify Completion
    queueCommand.completed();
  }

  @Override
  public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
    // Notify Completion
    queueCommand.completed();
  }

  @Override
  public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
    if (listener != null) {
      onCharacteristicChanged(gatt, characteristic, characteristic.getValue());
    }
  }

  @Override
  public void onCharacteristicChanged(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattCharacteristic characteristic, @NonNull byte[] value) {
    if (listener != null) {
      final UUID service = characteristic.getService().getUuid();
      final UUID uuid = characteristic.getUuid();
      listener.onCharacteristicChanged(service, uuid, value, value.length);
    }
  }

  @Override
  public void setListener(PortListener _listener) {
    portListener = _listener;
  }

  @Override
  public void setInputListener(InputListener _listener) {
    listener = _listener;
  }

  @SuppressLint("MissingPermission")
  @Override
  public void close() {
    stopLeScan();
    shutdown = true;
    writeBuffer.clear();
    if (gatt != null) {
      gatt.disconnect();
      waitForClose();
      gatt.close();
      gatt = null;
    }
  }

  private void waitForClose() {
    synchronized (gattStateSync) {
      long waitUntil = System.currentTimeMillis() + DISCONNECT_TIMEOUT;
      while (gattState != BluetoothGatt.STATE_DISCONNECTED) {
        long timeToWait = waitUntil - System.currentTimeMillis();
        if (timeToWait <= 0) {
          Log.e(TAG, "GATT disconnect timeout");
          break;
        }
        try {
          gattStateSync.wait(timeToWait);
        } catch (InterruptedException e) {
          Log.e(TAG, "GATT disconnect timeout", e);
          break;
        }
      }
    }
  }

  @Override
  public int getState() {
    return portState;
  }

  @Override
  public boolean drain() {
    return writeBuffer.drain();
  }

  @Override
  public int getBaudRate() {
    return 0;
  }

  @Override
  public boolean setBaudRate(int baud) {
    return true;
  }

  @Override
  public int write(byte[] data, int length) {
    return writeBuffer.write(gatt, hm10DataCharacteristic,
            queueCommand, data, length, maxChunkSize);
  }

  @Override
  public void writeGattCharacteristic(UUID service, UUID characteristic, byte[] data, int length) {
    final BluetoothGattService sv = gatt.getService(service);
    final BluetoothGattCharacteristic ch = sv.getCharacteristic(characteristic);

    queueCommand.add(() -> {
      if (!LeGattHelper.writeCharacteristic(gatt, ch, Arrays.copyOf(data, length))) {
        queueCommand.completed();
      }
    });
  }

  @Override
  public void readGattCharacteristic(UUID service, UUID characteristic) {
    final BluetoothGattService sv = gatt.getService(service);
    if (sv == null) {
      return;
    }
    final BluetoothGattCharacteristic ch = sv.getCharacteristic(characteristic);
    if (ch == null) {
      return;
    }

    queueCommand.add(() -> gatt.readCharacteristic(ch));
  }

  protected final void stateChanged() {
    final PortListener portListener = this.portListener;
    if (portListener != null) {
      portListener.portStateChanged();
    }
  }
}
