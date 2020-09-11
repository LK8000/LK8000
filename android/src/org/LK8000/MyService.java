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

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.graphics.BitmapFactory;
import androidx.core.app.NotificationCompat;

/**
 * All this Service implementation does is put itself in foreground.
 * It shall reduce the risk of getting killed by the Android Activity
 * Manager, because foreground services will only be killed under
 * extreme pressure.  Since this services runs in-process, Android
 * will also not terminate our main Activity.
 *
 * LK8000 needs high reliability, because it usually runs in
 * foreground as the only active application and should not be killed
 * by an incoming phone call.
 *
 * This is bad style for general-purpose Android apps, don't imitate
 * unless you have an excuse as good as ours ;-)
 */
public class MyService extends Service {
  private static final String TAG = "LK8000";

  /**
   * Hack: this is set by onCreate(), to support the "testing"
   * package.
   */
  protected static Class<?> mainActivityClass;

  @Override public void onCreate() {
    if (mainActivityClass == null)
      mainActivityClass = LK8000.class;

    super.onCreate();
  }

  @Override public int onStartCommand(Intent intent, int flags, int startId) {

    /* add an icon to the notification area while LK8000 runs, to
   remind the user that we're sucking his battery empty */

    final String CHANNEL_ID = getApplicationContext().getPackageName() + "_NotificationChannel";

    NotificationManager manager =  (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      // Support for Android Oreo: Notification Channels
      NotificationChannel channel = manager.getNotificationChannel(CHANNEL_ID);
      if(channel == null) {
        channel = new NotificationChannel(
                CHANNEL_ID,
                "LK8000",
                NotificationManager.IMPORTANCE_LOW);

        manager.createNotificationChannel(channel);
      }
    }

    Intent intent2 = new Intent(this, mainActivityClass);
    PendingIntent contentIntent =
      PendingIntent.getActivity(this, 0, intent2, 0);

    NotificationCompat.Builder notification = new NotificationCompat.Builder(this, CHANNEL_ID);
    notification.setSmallIcon(R.drawable.notification_icon);
    notification.setLargeIcon(BitmapFactory.decodeResource( getResources(), getApplicationInfo().icon));
    notification.setContentTitle("LK8000 is running");
    notification.setContentText("Touch to open");
    notification.setContentIntent(contentIntent);
    notification.setWhen(System.currentTimeMillis());
    notification.setShowWhen(false);
    notification.setOngoing(true);
    notification.setOnlyAlertOnce(true);

    startForeground(1, notification.build());

    /* We want this service to continue running until it is explicitly
       stopped, so return sticky */
    return START_STICKY;
  }

  @Override public void onDestroy() {

    stopForeground(true);

    super.onDestroy();
  }

  @Override public IBinder onBind(Intent intent) {
    return null;
  }
}
