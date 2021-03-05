package org.LK8000;

import android.media.MediaScannerConnection;
import android.content.Context;

public class FileUtils {

    public static void updateMediaStore(Context context, String filePath) {
        MediaScannerConnection.scanFile(context, new String[]{filePath}, null, null);
    }

}
