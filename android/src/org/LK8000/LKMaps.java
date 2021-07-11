/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKMaps.java
 * Author: Tonino Tarsi 21/01/2017.
 *
 */

package org.LK8000;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;

public class LKMaps {

    final static String appPackageName = "org.lk8000.lkmap";

    public static void openLKMaps(Context context) {

        if ( !isPackageInstalled(context)  ) {
            try {
                context.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=" + appPackageName)));
            } catch (android.content.ActivityNotFoundException anfe) {
                context.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("https://play.google.com/store/apps/details?id=" + appPackageName)));
            }
        }
        else {
            Intent launchIntent = context.getPackageManager().getLaunchIntentForPackage(appPackageName);
            if (launchIntent != null) {
                context.startActivity(launchIntent);
            }
        }
    }

    private static boolean isPackageInstalled(Context context) {
        try {
            PackageManager pm = context.getPackageManager();
            pm.getPackageInfo(appPackageName, PackageManager.GET_ACTIVITIES);
            return true;
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }
    }

}
