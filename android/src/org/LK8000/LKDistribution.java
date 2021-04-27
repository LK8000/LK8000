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

package org.LK8000;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Environment;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import static org.LK8000.FileUtils.closeStream;
import static org.LK8000.FileUtils.copyDirectory;
import static org.LK8000.FileUtils.copyFile;
import static org.LK8000.FileUtils.deleteDirectory;
import static org.LK8000.FileUtils.isEmptyDirectory;

public class LKDistribution {

    private static void copyDirectoryToExternalStorage(Context context, String inDir, String outDir, boolean overWrite) {
        AssetManager assetManager = context.getAssets();

        try {
            String[] list = assetManager.list(inDir);
            if (list != null) {
                for (String filename : list) {
                    copyFileToExternalStorage(context, inDir + "/" + filename, outDir + "/" + filename, overWrite);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static boolean isOtherVersionInstalled(Context context) {
        PackageManager pm = context.getPackageManager();
        final String[] list = {
                "org.lk8000.debug",
                "org.lk8000.beta",
                "org.lk8000.test"
        };
        boolean result = false;
        for (String name : list) {
            if (!name.equals(BuildConfig.APPLICATION_ID)) {
                result |= isOtherVersionInstalled(pm, name);
            }
        }
        return result;
    }

    private static boolean isOtherVersionInstalled(PackageManager pm, String packageName) {
        try {
            PackageInfo packageInfo = pm.getPackageInfo(packageName, PackageManager.GET_META_DATA);
            return packageInfo != null;
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }
    }

    private static File getLegacyStorage() {
        return new File(Environment.getExternalStorageDirectory(), "LK8000/");
    }

    static class RecentFilter implements FileUtils.Filter {
        @Override
        public boolean apply(File src, File dst) {
            if (dst.exists()) {
                // don't overwrite newer file.
                return dst.lastModified() < src.lastModified();
            }
            return true;
        }
    }

    static class LegacyFilter extends RecentFilter {
        @Override
        public boolean apply(File src, File dst) {
            if (src.isDirectory()) {
                // _Configuration_xxxx will be copied later.
                return !src.getName().startsWith("_Configuration");
            }
            if (src.getName().equalsIgnoreCase("runtime.log")) {
                return false; // ignore runtime
            }
            return super.apply(src, dst);
        }
    }

    final static FileUtils.Filter legacyFilter = new LegacyFilter();
    final static FileUtils.Filter recentFilter = new RecentFilter();

    public static void copyLKDistribution(Context context, boolean force) {

        final File externalStorage = context.getExternalFilesDir(null);
        final File legacyStorage = getLegacyStorage();
        final File dstConfig = new File(externalStorage, "_Configuration");

        if (!dstConfig.exists() && !isEmptyDirectory(legacyStorage)) {
            // copy all files to external private directory
            //  excluding config directory
            copyDirectory(legacyStorage, externalStorage, legacyFilter);

            // move only current version directory "_Configuration"
            if (BuildConfig.BUILD_TYPE.equals("beta")) {
                moveConfig(dstConfig, "_Configuration_beta");
            } else if (BuildConfig.BUILD_TYPE.equals("debug")) {
                moveConfig(dstConfig, "_Configuration_debug");
            } else {
                moveConfig(dstConfig, "_Configuration");
            }

            boolean deleteAll = true;
            if (isOtherVersionInstalled(context)) {
                /*
                 * delete legacy storage only if all other installed version
                 * have already moved it's configuration files
                 */
                boolean otherExists = new File(legacyStorage, "_Configuration_beta").exists();
                otherExists |=  new File(legacyStorage, "_Configuration_debug").exists();
                otherExists |=  new File(legacyStorage, "_Configuration").exists();

                deleteAll = !otherExists;
            }

            // if all other version has already moved there files we can delete the wall legacy folder.
            if (deleteAll) {
                deleteDirectory(legacyStorage);
            }
        }

        // copy from assets to private external storage
        File f = new File(externalStorage, "_Configuration/DEFAULT_PROFILE.prf");
        boolean default_exist = f.exists();
        if (!default_exist || force) {
            // never force DEFAULT_PROFILE.prf
            copyFileToExternalStorage(context, "distribution/configuration/DEMO.prf", "_Configuration/DEFAULT_PROFILE.prf", false);
            copyDirectoryToExternalStorage(context, "distribution/configuration", "_Configuration", force);
            copyDirectoryToExternalStorage(context, "distribution/maps", "_Maps", force);
            copyDirectoryToExternalStorage(context, "distribution/waypoints", "_Waypoints", force);
            copyDirectoryToExternalStorage(context, "distribution/airspaces", "_Airspaces", force);
            copyDirectoryToExternalStorage(context, "distribution/logger", "_Logger", force);
            copyDirectoryToExternalStorage(context, "distribution/tasks", "_Tasks", force);
            copyDirectoryToExternalStorage(context, "distribution/polars", "_Polars", force);
        }
    }

    private static void moveConfig(File dstConfig, String config) {
        final File srcConfig = new File(getLegacyStorage(), config);
        copyDirectory(srcConfig, dstConfig, recentFilter);
        deleteDirectory(srcConfig);
    }

    public static void copyFileToExternalStorage(Context context, String inFileName, String outFileName, boolean overWrite) {
        AssetManager assetManager = context.getAssets();

        File outFile = new File(context.getExternalFilesDir(null), outFileName);
        if (!overWrite && outFile.exists()) {
            return;
        }

        File parentDir = outFile.getParentFile();
        if (parentDir.exists() || parentDir.mkdirs()) {

            InputStream in = null;
            OutputStream out = null;
            try {
                in = assetManager.open(inFileName);
                out = new FileOutputStream(outFile);
                copyFile(in, out);
                out.flush();
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                closeStream(in);
                closeStream(out);
            }
        }
    }
}
