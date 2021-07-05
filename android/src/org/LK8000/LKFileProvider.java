package org.LK8000;

import android.content.Context;
import android.net.Uri;

import androidx.core.content.FileProvider;

import java.io.File;

public class LKFileProvider extends FileProvider {
    public final static String authorities = BuildConfig.APPLICATION_ID + ".file_provider";

    public static Uri getUriForFile(Context context, File file) {
        return getUriForFile(context, authorities, file);
    }
}
