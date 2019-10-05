package org.LK8000;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.channels.FileChannel;

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

        String configurationDirectory = "_Configuration";


        if (BuildConfig.BUILD_TYPE.equals("beta") ) {
            configurationDirectory = "_Configuration_beta";
            if ( configurationDirIsEmpty("/LK8000/" + configurationDirectory)  ) {
                if ( distributionDirExist("LK8000/_Configuration"))  {
                    copyDirectory(Environment.getExternalStorageDirectory() + "/LK8000/_Configuration",
                            Environment.getExternalStorageDirectory() + "/LK8000/" + configurationDirectory);
                }
                else {
                    createDirectoryToExternalStorageIfAbsent("LK8000/" + configurationDirectory);
                }
            }
        }
        else if (BuildConfig.BUILD_TYPE.equals("debug") ) {
            configurationDirectory = "_Configuration_debug";
            if ( configurationDirIsEmpty("/LK8000/" + configurationDirectory)  ) {
                if ( distributionDirExist("LK8000/_Configuration"))  {
                    copyDirectory(Environment.getExternalStorageDirectory() + "/LK8000/_Configuration",
                            Environment.getExternalStorageDirectory() + "/LK8000/" + configurationDirectory);
                }
                else {
                    createDirectoryToExternalStorageIfAbsent("LK8000/" + configurationDirectory);
                }
            }
        }

        createDirectoryToExternalStorageIfAbsent("LK8000/" + configurationDirectory);
        createDirectoryToExternalStorageIfAbsent("LK8000");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Maps");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Airspaces");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Waypoints");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Logger");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Tasks");
        createDirectoryToExternalStorageIfAbsent("LK8000/_Polars");



        File f = new File(Environment.getExternalStorageDirectory(), "LK8000/" + configurationDirectory + "/DEFAULT_PROFILE.prf");
        boolean default_exist = f.exists();
        if (!default_exist || force ) {
            copyFileToExternalStorage(context, "distribution/configuration/DEMO.prf", "LK8000/" + configurationDirectory + "/DEFAULT_PROFILE.prf", false); // never forse DEFAULT_PROFILE.prf
            copyDirectoryToExternalStorage(context, "distribution/configuration", "LK8000/" + configurationDirectory, force);
            copyDirectoryToExternalStorage(context, "distribution/maps", "LK8000/_Maps", force);
            copyDirectoryToExternalStorage(context, "distribution/waypoints", "LK8000/_Waypoints", force);
            copyDirectoryToExternalStorage(context, "distribution/airspaces", "LK8000/_Airspaces", force);
            copyDirectoryToExternalStorage(context, "distribution/logger", "LK8000/_Logger", force);
            copyDirectoryToExternalStorage(context, "distribution/tasks", "LK8000/_Tasks", force);
            copyDirectoryToExternalStorage(context, "distribution/polars", "LK8000/_Polars", force);
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

    private static void copyDirectory(String srcDir, String dstDir) {

        try {
            File src = new File(srcDir);
            File dst = new File(dstDir);

            String files[] = src.list();
            int filesLength = files.length;
            for (int i = 0; i < filesLength; i++) {
                File src1 = new File(src, files[i]);
                File dst1 =new File(dst, files[i]);
                copyFile(src1, dst1);

            }


        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    private static void copyFile(File sourceFile, File destFile) throws IOException {
        if (!destFile.getParentFile().exists())
            destFile.getParentFile().mkdirs();

        if (!destFile.exists()) {
            destFile.createNewFile();
        }

        FileChannel source = null;
        FileChannel destination = null;

        try {
            source = new FileInputStream(sourceFile).getChannel();
            destination = new FileOutputStream(destFile).getChannel();
            destination.transferFrom(source, 0, source.size());
        } finally {
            if (source != null) {
                source.close();
            }
            if (destination != null) {
                destination.close();
            }
        }
    }


    private  static  boolean distributionDirExist(String extpath)
    {
        String dir_path =  Environment.getExternalStorageDirectory() + "/" + extpath;
        boolean ret = false;
        File dir = new File(dir_path);
        if(dir.exists() && dir.isDirectory())
            ret = true;
        return ret;
    }

    private  static  boolean configurationDirIsEmpty(String extpath)
    {
        String dir_path =  Environment.getExternalStorageDirectory() + "/" + extpath;
        File dir = new File(dir_path);
        if(dir.exists() && dir.isDirectory()) {
            String[] files = dir.list();
            return (files == null || files.length == 0);
        }
        return true;
    }


}
