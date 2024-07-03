/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2021 The XCSoar Project
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
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothStatusCodes;
import android.os.Build;
import android.util.Log;

import java.util.Arrays;

/**
 * This class helps with writing chunked data to a Bluetooth LE HM10
 * and compatible device.
 */
final class HM10WriteBuffer {
  private static final String TAG = "HM10WriteBuffer";

  private byte[][] pendingWriteChunks = null;
  private int nextWriteChunkIdx;
  private boolean lastChunkWriteError;

  @SuppressLint("MissingPermission")
  private static boolean writeCharacteritics(BluetoothGatt gatt, BluetoothGattCharacteristic dataCharacteristic, byte[] pendingWriteChunk) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
      return gatt.writeCharacteristic(dataCharacteristic, pendingWriteChunk,
              dataCharacteristic.getWriteType()) == BluetoothStatusCodes.SUCCESS;
    }
    else {
      dataCharacteristic.setValue(pendingWriteChunk);
      return gatt.writeCharacteristic(dataCharacteristic);
    }
  }

  synchronized boolean beginWriteNextChunk(BluetoothGatt gatt,
                                           BluetoothGattCharacteristic dataCharacteristic) {

    if (pendingWriteChunks == null)
      return false;

    if (!writeCharacteritics(gatt, dataCharacteristic, pendingWriteChunks[nextWriteChunkIdx])) {
      Log.e(TAG, "GATT characteristic write request failed");
      setError();
      return false;
    }

    ++nextWriteChunkIdx;
    if (nextWriteChunkIdx >= pendingWriteChunks.length) {
      /* writing is done */
      clear();
    }

    return true;
  }

  synchronized void clear() {
    pendingWriteChunks = null;
    notifyAll();
  }

  synchronized void setError() {
    lastChunkWriteError = true;
    clear();
  }

  synchronized boolean drain() {
    final long TIMEOUT = 5000;
    final long waitUntil = System.currentTimeMillis() + TIMEOUT;

    while (pendingWriteChunks != null) {
      final long timeToWait = waitUntil - System.currentTimeMillis();
      if (timeToWait <= 0)
        return false;

      try {
        wait(timeToWait);
      } catch (InterruptedException e) {
        return false;
      }
    }

    return true;
  }

  synchronized int write(BluetoothGatt gatt,
                         BluetoothGattCharacteristic dataCharacteristic,
                         AsyncCompletionQueue queueCommand,
                         byte[] data, int length, int maxChunkSize) {
    final long TIMEOUT = 5000;

    if (0 == length)
      return 0;

    if (!drain())
      return 0;

    if (dataCharacteristic == null)
      return 0;

    /* Write data in `maxChunkSize` byte large chunks at maximun. GATT devices do
       not support characteristic values which are larger than negotiated mtu size. */
    int writeChunksCount = (length + maxChunkSize - 1) / maxChunkSize;
    pendingWriteChunks = new byte[writeChunksCount][];
    nextWriteChunkIdx = 0;
    lastChunkWriteError = false;
    for (int i = 0; i < writeChunksCount; ++i) {
      final int from = i * maxChunkSize;
      final int to = Math.min((i + 1) * maxChunkSize, length);
      pendingWriteChunks[i] = Arrays.copyOfRange(data, from, to);
    }

    queueCommand.add(() -> {
      if (!beginWriteNextChunk(gatt, dataCharacteristic)) {
        setError();
        queueCommand.completed();
      }
    });

    if (pendingWriteChunks != null) {
      try {
        wait(TIMEOUT);
      } catch (InterruptedException e) {
        /* cancel the write on interruption */
        pendingWriteChunks = null;
        return 0;
      }
    }
    if (pendingWriteChunks != null && nextWriteChunkIdx == 0) {
      /* timeout */
      pendingWriteChunks = null;
      return 0;
    }

    return lastChunkWriteError ? 0 : length;
  }
}
