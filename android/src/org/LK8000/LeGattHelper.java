package org.LK8000;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothStatusCodes;
import android.os.Build;

public class LeGattHelper {

  @SuppressLint("MissingPermission")
  public static boolean writeCharacteristic(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic,
                                            byte[] value) {

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
      return gatt.writeCharacteristic(characteristic, value,
              characteristic.getWriteType()) == BluetoothStatusCodes.SUCCESS;
    }
    else {
      characteristic.setValue(value);
      return gatt.writeCharacteristic(characteristic);
    }
  }

  @SuppressLint("MissingPermission")
  public static boolean writeDescriptor(BluetoothGatt gatt,
                                        BluetoothGattDescriptor descriptor,
                                        byte[] value) {

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
      return gatt.writeDescriptor(descriptor, value) == BluetoothStatusCodes.SUCCESS;
    }
    else {
      descriptor.setValue(value);
      return gatt.writeDescriptor(descriptor);
    }
  }
}
