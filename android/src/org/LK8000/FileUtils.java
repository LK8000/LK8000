package org.LK8000;

import android.content.Context;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Build;
import android.provider.DocumentsContract;
import android.webkit.MimeTypeMap;

import androidx.annotation.Nullable;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Objects;

public class FileUtils {

    interface Filter {
        boolean apply(File src, File dst);
    }

    public static void updateMediaStore(Context context, String filePath) {
        MediaScannerConnection.scanFile(context, new String[]{filePath}, null, null);
    }

    public static boolean deleteDirectory(File directory) {
        try {
            if (directory.isDirectory()) {
                for (File child : Objects.requireNonNull(directory.listFiles())) {
                    deleteDirectory(child);
                }
            }
            return directory.delete();
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean isEmptyDirectory(File directory) {
        if (directory.exists()) {
            File[] list = directory.listFiles();
            if (list != null) {
                return list.length <= 0;
            }
        }
        return true;
    }

    public static void copyDirectory(File srcDir, File dstDir, Filter filter) {
        try {
            String[] list = srcDir.list();
            if (list != null) {
                for (String fileName : list) {
                    File src = new File(srcDir, fileName);
                    File dst = new File(dstDir, fileName);
                    if (filter != null && !filter.apply(src, dst)) {
                        continue;
                    }
                    if (src.isDirectory()) {
                        if (dst.mkdirs()) {

                            dst.setLastModified(src.lastModified());
                            dst.setWritable(src.canWrite());
                            dst.setReadable(src.canRead());
                            dst.setExecutable(src.canExecute());
                        }
                        copyDirectory(src, dst, filter);
                    } else {
                        copyFile(src, dst);
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void copyFile(File sourceFile, File destFile) throws IOException {
        File destDirectory = destFile.getParentFile();
        if (!destDirectory.exists()) {
            destDirectory.mkdirs();
        }

        if (!destFile.exists()) {
            destFile.createNewFile();
        }

        InputStream in = null;
        OutputStream out = null;
        try {
            in = new FileInputStream(sourceFile);
            out = new FileOutputStream(destFile);
            copyFile(in, out);
            out.flush();
        } finally {
            closeStream(in);
            closeStream(out);
        }
        destFile.setLastModified(sourceFile.lastModified());
        destFile.setWritable(sourceFile.canWrite());
        destFile.setReadable(sourceFile.canRead());
        destFile.setExecutable(sourceFile.canExecute());
    }

    public static void closeStream(@Nullable Closeable in) {
        try {
            if (in != null) {
                in.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }

    public static String getDocumentType(final File file) throws FileNotFoundException {
        if (file.isDirectory()) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                return DocumentsContract.Document.MIME_TYPE_DIR;
            } else {
                return "vnd.android.document/directory";
            }
        }
        if (file.getName().equalsIgnoreCase("runtime.log")) {
            return "text/plain";
        }

        final String extension = MimeTypeMap.getFileExtensionFromUrl(Uri.fromFile(file).toString());
        final String mime = MimeTypeMap.getSingleton().getMimeTypeFromExtension(extension);
        if (mime != null) {
            return mime;
        }

        if (extension.equalsIgnoreCase("igc")) {
            return "application/vnd.fai.igc";
        }

        if (extension.equalsIgnoreCase("cup")) {
            return "application/vnd.naviter.seeyou.cup";
        }

        if (extension.equalsIgnoreCase("wpt")) {
            try {
                FileReader reader = new FileReader(file);
                BufferedReader bufferedReader = new BufferedReader(reader);
                String header = bufferedReader.readLine();
                bufferedReader.close();
                reader.close();
                if (header.contains("OziExplorer Waypoint File")) {
                    return "application/vnd.oziexplorer.wpt";
                }
            } catch (IOException ignore) {
            }
        }

        if (extension.equalsIgnoreCase("gpx")) {
            return "application/gpx+xml";
        }

        return "application/octet-stream";
    }
}
