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

import org.LK8000.LKMap.LKMAPActivity;

public class LKMaps {
    public static void openLKMaps(Context context) {
        Intent intent = new Intent(context, LKMAPActivity.class);
        context.startActivity(intent);
    }
}
