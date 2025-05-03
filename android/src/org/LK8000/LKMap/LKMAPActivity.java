/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKMAPActivity.java
 */

package org.LK8000.LKMap;

import android.app.AlertDialog;
import android.app.DownloadManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.view.MenuItem;
import android.webkit.MimeTypeMap;
import android.webkit.URLUtil;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import org.LK8000.R;
import org.LK8000.databinding.ActivityLkmapBinding;

import java.io.File;

public class LKMAPActivity extends AppCompatActivity {
    final String TAG = "LKMAPActivity";

    private String currentURL = "";
    private ActivityLkmapBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);

        binding = ActivityLkmapBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        ViewCompat.setOnApplyWindowInsetsListener(binding.getRoot(), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        //To have the back button!!
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

        WebView webView = binding.WebView;

        webView.getSettings().setJavaScriptEnabled(true);
        webView.getSettings().setDomStorageEnabled(true);
        webView.getSettings().setAllowFileAccessFromFileURLs(true);
        webView.getSettings().setAllowUniversalAccessFromFileURLs(true);
        webView.getSettings().setAllowFileAccess(true);

        webView.setWebViewClient(new WebViewClient() {

            @Override
            public void onPageFinished(WebView view, String url) {
                currentURL = URLUtil.guessFileName(url, null, null);
            }

            @Override
            public boolean shouldOverrideUrlLoading(WebView view, final String url) {
                String type = MimeTypeMap.getFileExtensionFromUrl(url ).toLowerCase();
                if ( type.equals("html") || type.equals("htm")) {
                    return false;
                }
                else if (type.equals("dem") || type.equals("lkm")) {
                    downloadFile(url,"_Maps");
                    return true;
                }
                else if (  ( type.equals("fln") ) ) {
                    downloadFile(url,"_Configuration");
                    return true;
                }
                else if ( currentURL.equals("waypoints.html") && (type.equals("cup") || type.equals("aip"))) {
                    downloadFile(url,"_Waypoints");
                    return true;
                }
                else if ( currentURL.equals("airspaces.html") && (type.equals("txt") || type.equals("aip"))) {
                    downloadFile(url,"_Airspaces");
                    return true;
                }
                else if ( currentURL.equals("configuration.html") && (type.equals("prf")
                        || type.equals("dvc"))) {
                    downloadFile(url,"_Configuration");
                    return true;
                }
                return false;
            }
        });

        webView.loadUrl("file:///android_asset/lkmap/index.html");
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onBackPressed() {
        WebView webView = binding.WebView;
        if(webView.isFocused() && webView.canGoBack()) {
            webView.goBack();
        }
        else {
            super.onBackPressed();
        }
    }

    private void downloadFile(String url,String dest) {
        File dir = new File(getExternalFilesDir(null), dest);
        if (dir.exists() || dir.mkdirs()) {

            String filename = URLUtil.guessFileName(url, null, null);
            File dstFile = new File(dir, filename);
            if (dstFile.exists()) {
                showConfirmDialog(url, dest, filename, dstFile);
            } else {
                downloadFile(url, dest, filename, dstFile);
            }
        }
    }

    private void downloadFile(String url, String dest, String filename, File dstFile) {

        Uri destUri = Uri.fromFile(dstFile);
        Uri srcUri = Uri.parse(url);

        final DownloadManager downloadManager =
                (DownloadManager)getSystemService(DOWNLOAD_SERVICE);

        cancelPendingDownload(downloadManager, url);

        if (dstFile.exists()) {
            dstFile.delete(); // to overwrite existing files
        }


        downloadManager.enqueue(new DownloadManager.Request(srcUri)
                        .setAllowedNetworkTypes(DownloadManager.Request.NETWORK_WIFI |
                                DownloadManager.Request.NETWORK_MOBILE)
                        .setAllowedOverRoaming(true)
                        .setTitle(getString(R.string.download_title))
                        .setDescription(new File(dest, filename).getAbsolutePath())
                        .setDestinationUri(destUri));
    }

    interface Callback {
        void execute(Cursor cursor);
    }

    private void ForEachPendingDownload(DownloadManager downloadManager, String url, Callback callback) {
        try (Cursor cursor = downloadManager.query(new DownloadManager.Query())) {
            int idxUri = cursor.getColumnIndex(DownloadManager.COLUMN_URI);
            while (cursor.moveToNext()) {
                if (url.equals(cursor.getString(idxUri))) {
                    callback.execute(cursor);
                }
            }
        }
    }

    private void cancelPendingDownload(DownloadManager downloadManager, String url) {
        ForEachPendingDownload(downloadManager, url, cursor -> {
            int idxId = cursor.getColumnIndex(DownloadManager.COLUMN_ID);
            downloadManager.remove(cursor.getInt(idxId));
        });
    }

    private void showConfirmDialog(String url, String dest, String filename, File dstFile)  {
        AlertDialog.Builder dlg = new AlertDialog.Builder(this)
                .setMessage(R.string.overwrite_download)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setPositiveButton(android.R.string.ok, (dialog, which) -> {
                    downloadFile(url, dest, filename, dstFile);
                })
                .setNegativeButton(android.R.string.cancel, (dialog, which) -> {
                    // nothing to do...
                });

        dlg.show();
    }
}
