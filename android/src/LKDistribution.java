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
 * File:   LKDistribution.java
 * Copy LKDistribution data from assets to external storage
 * Author: Tonino Tarsi 30/12/2016.
 *
 */

public class LKDistribution {

    public static void createDirectoryToExternalStorageIfAbsent(String strDir) {
        File f = new File(Environment.getExternalStorageDirectory(), java.io.File.separator + strDir);
        if (!f.exists()) {
            f.mkdirs();
        }
    }

    public static void copyDirectoryToExternalStorage(Context context,String inDir, String outDir, boolean overWrite) {
        String[] files = null;
        AssetManager assetManager = context.getAssets();
        try {
            files = assetManager.list( inDir);
            for (String filename : files) {
                copyFileToExternalStorage(context, inDir + "/" + filename, outDir + "/" + filename ,overWrite);

            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void copyLKDistribution(Context context, boolean force) {
        createDirectoryToExternalStorageIfAbsent("LK8000");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Maps");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Airspaces");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Waypoints");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Configuration");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Logger");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Tasks");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Polars");

        File f = new File(Environment.getExternalStorageDirectory(), "LK8000/_Configuration/DEFAULT_PROFILE.prf");
        boolean default_exist = f.exists();
        if (!default_exist || force ) {
            copyFileToExternalStorage(context, "distribution/configuration/DEMO.prf", "LK8000/_Configuration/DEFAULT_PROFILE.prf",false); // never forse DEFAULT_PROFILE.prf
            copyDirectoryToExternalStorage(context,"distribution/configuration", "LK8000/_Configuration",force);
            copyDirectoryToExternalStorage(context,"distribution/maps", "LK8000/_Maps",force);
            copyDirectoryToExternalStorage(context,"distribution/waypoints", "LK8000/_Waypoints",force);
            copyDirectoryToExternalStorage(context,"distribution/airspaces", "LK8000/_Airspaces",force);
            copyDirectoryToExternalStorage(context,"distribution/logger", "LK8000/_Logger",force);
            copyDirectoryToExternalStorage(context,"distribution/tasks", "LK8000/_Tasks",force);
            copyDirectoryToExternalStorage(context,"distribution/polars", "LK8000/_Polars",force);

        }
    }

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }

    public static void copyFileToExternalStorage(Context context, String inFileName, String outFileName,boolean overWrite) {
        if ( !overWrite   ) {
            File file = new File(Environment.getExternalStorageDirectory().toString() + "/" + outFileName);
            if(file.exists())
                return;
        }
        AssetManager assetManager = context.getAssets();
        InputStream in = null;
        OutputStream out = null;
        try {
            in = assetManager.open(  inFileName);
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
