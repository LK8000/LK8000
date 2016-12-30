package org.LK8000;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   DemoData.java
 * Copy demo data from assets to external storage
 * Author: Tonino Tarsi 30/12/2016.
 *
 */

public class DemoData {

    public static void createDirectoryIfAbsent(String strDir) {
        File f = new File(Environment.getExternalStorageDirectory(), "LK8000" + java.io.File.separator + strDir);
        if (!f.exists()) {
            f.mkdirs();
        }
    }

    public static void copyDemoData(Context context) {
        createDirectoryIfAbsent("_Maps");
        createDirectoryIfAbsent("_Airspaces");
        createDirectoryIfAbsent("_Waypoints");
        createDirectoryIfAbsent("_Configuration");
        File f = new File(Environment.getExternalStorageDirectory(), "LK8000" + java.io.File.separator + "_Configuration" + java.io.File.separator + "DEFAULT_PROFILE.prf");
        if (!f.exists()) {
            copyToExternalStorage(context, "DEMO.prf", "LK8000/_Configuration" + java.io.File.separator + "DEFAULT_PROFILE.prf",false);
            copyToExternalStorage(context, "DEMO.DEM", "LK8000/_Maps" + java.io.File.separator + "DEMO.DEM",false);
            copyToExternalStorage(context, "DEMO.LKM", "LK8000/_Maps" + java.io.File.separator + "DEMO.LKM",false);
            copyToExternalStorage(context, "DEMO.cup", "LK8000/_Waypoints" + java.io.File.separator + "DEMO.cup",false);
            copyToExternalStorage(context, "WAYNOTES.TXT", "LK8000/_Waypoints" + java.io.File.separator + "WAYNOTES.TXT",false);
            copyToExternalStorage(context, "DEMO.txt", "LK8000/_Airspaces" + java.io.File.separator + "DEMO.txt",false);
        }
    }

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }

    public static void copyToExternalStorage(Context context, String inFileName, String outFileName,boolean overWrite) {
        if ( !overWrite   ) {
            File file = new File(Environment.getExternalStorageDirectory().toString() + "/" + outFileName);
            if(file.exists())
                return;
        }
        AssetManager assetManager = context.getAssets();
        InputStream in = null;
        OutputStream out = null;
        try {
            in = assetManager.open("demodata/" + inFileName);
            out = new FileOutputStream(Environment.getExternalStorageDirectory().toString() + "/" + outFileName);
            copyFile(in, out);
            in.close();
            in = null;
            out.flush();
            out.close();
            out = null;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
