/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   AsyncCompletionQueue.java
 * Author: Bruno de Lacheisserie
 */
package org.LK8000;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

import java.util.LinkedList;
import java.util.Queue;

public class AsyncCompletionQueue {
  private final Queue<Runnable> commandQueue = new LinkedList<>();
  private final HandlerThread handlerThread;
  public final Handler queueHandler;
  private final Object commandQueueLock = new Object();
  private boolean commandQueueBusy = false;

  AsyncCompletionQueue() {
    handlerThread = new HandlerThread("BLE Handler queue");
    handlerThread.start();
    queueHandler = new Handler(handlerThread.getLooper());
  }

  public void add(Runnable run) {
    synchronized (commandQueueLock) {
      commandQueue.add(run);
    }
    runNext();
  }

  public void completed() {
    synchronized (commandQueueLock) {
      commandQueueBusy = false;
    }
    runNext();
  }

  private void runNext() {
    synchronized (commandQueueLock) {
      // If there is still a command being executed then bail out
      if (commandQueueBusy) {
        return;
      }
      if (!commandQueue.isEmpty()) {
        final Runnable command = commandQueue.poll();
        if (command != null) {
          commandQueueBusy = true;
          queueHandler.post(command);
        }
      }
    }
  }
}
