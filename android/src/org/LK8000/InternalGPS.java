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
import android.content.Context;
import android.content.Intent;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.OnNmeaMessageListener;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.util.Log;

/**
 * Code to support the internal GPS receiver via #LocationManager.
 */
public class InternalGPS
  implements LocationListener, Runnable {
  private static final String TAG = "LK8000";

  private static Handler handler;

  /**
   * Global initialization of the class.
   */
  public static void Initialize() {
    // Handler object must be bound to MainThread
    handler = new Handler(Looper.getMainLooper());
  }

  /** the index of this device in the global list */
  private final int index;

  /** the name of the currently selected location provider */
  private String locationProvider = LocationManager.GPS_PROVIDER;
  //String locationProvider = LocationManager.NETWORK_PROVIDER;

  private LocationManager locationManager;
  private static boolean queriedLocationSettings = false;


  private final SafeDestruct safeDestruct = new SafeDestruct();

  InternalGPS(Context context, int _index) {
    index = _index;

    try {
      locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
      if (locationManager == null ||
              locationManager.getProvider(locationProvider) == null) {
      /* on the Nook Simple Touch, LocationManager.isProviderEnabled()
         can crash, but LocationManager.getProvider() appears to be
         safe, therefore we're first checking the latter; if the
         device does have a GPS, it returns non-null even when the
         user has disabled GPS */
        locationProvider = null;
      } else if (!locationManager.isProviderEnabled(locationProvider) &&
              !queriedLocationSettings) {
        // Let user turn on GPS, XCSoar is not allowed to.
        Intent myIntent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
        context.startActivity(myIntent);
        queriedLocationSettings = true;
      }
    } catch (Exception e) {
      locationProvider = null;
      Log.e(TAG, "GPS Unavailable", e);
    }

    update();
  }

  private GpsStatus.NmeaListener listener = null;
  private OnNmeaMessageListener listener_N = null;

  /**
   * Called by the #Handler, indirectly by update().  Updates the
   * LocationManager subscription inside the main thread.
   */
  @SuppressLint("MissingPermission")
  @Override public void run() {
    Log.d(TAG, "Updating GPS subscription...");
    locationManager.removeUpdates(this);

    if(Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
      if(listener != null) {
        locationManager.removeNmeaListener(listener);
      }
    } else {
      if(listener_N != null)  {
        locationManager.removeNmeaListener(listener_N);
      }
    }


    if (locationProvider != null) {
      Log.d(TAG, "Subscribing to GPS updates.");

      try {
        // Must be Bound to MainThread
        locationManager.requestLocationUpdates(locationProvider,1000, 0, this, Looper.getMainLooper());
      } catch (IllegalArgumentException e) {
      /* this exception was recorded on the Android Market, message
         was: "provider=gps" - no idea what that means */
        setConnectedSafe(false);
        return;
      }

      if(Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
        if(listener == null) {
          listener = (timestamp, nmea) -> InternalGPS.this.parseNMEA(nmea);
        }
        locationManager.addNmeaListener(listener);
      } else {
        if(listener_N == null) {
          listener_N = (nmea, timestamp) -> InternalGPS.this.parseNMEA(nmea);
        }
        // Must be Bound to MainThread
        locationManager.addNmeaListener(listener_N, new Handler(Looper.getMainLooper()));
      }

      setConnectedSafe(true); // waiting for fix
    } else {
      Log.d(TAG, "Unsubscribing from GPS updates.");
      setConnectedSafe(false); // not connected
    }
    Log.d(TAG, "Done updating GPS subscription...");
  }

  /**
   * Update the LocationManager subscription.  May be called from any
   * thread.
   */
  private void update() {
    Log.d(TAG, "Triggering GPS subscription update...");
    handler.removeCallbacks(this);
    handler.post(this);
  }

  private void setLocationProvider(String _locationProvider) {
    locationProvider = _locationProvider;
    update();
  }

  public void close() {
    safeDestruct.beginShutdown();
    setLocationProvider(null);
    safeDestruct.finishShutdown();
  }

  private native void setConnected(boolean connected);

  private native void parseNMEA(String nmea);

  private void setConnectedSafe(boolean connected) {
    if (!safeDestruct.Increment())
      return;

    try {
      setConnected(connected);
    } finally {
      safeDestruct.Decrement();
    }
  }

  /** from LocationListener */
  @Override public void onProviderDisabled(String provider) {
    setConnectedSafe(false); // not connected
  }

  /** from LocationListener */
  @Override public void onProviderEnabled(String provider) {
    setConnectedSafe(true); // waiting for fix
  }

  /** from LocationListener (unused) */
  @Override public void onLocationChanged(Location newLocation) {
  }

}
