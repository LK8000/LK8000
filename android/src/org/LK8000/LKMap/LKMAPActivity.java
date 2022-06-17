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
import android.net.Uri;
import android.os.Bundle;
import android.view.MenuItem;
import android.webkit.MimeTypeMap;
import android.webkit.URLUtil;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

import org.LK8000.R;
import org.LK8000.databinding.ActivityLkmapBinding;

import java.io.File;

public class LKMAPActivity extends AppCompatActivity {

    private String currentURL = "";
    private ActivityLkmapBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityLkmapBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

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
        if (dstFile.exists()) {
           dstFile.delete(); // to overwrite existing files
        }

        Uri destUri = Uri.fromFile(dstFile);
        Uri srcUri = Uri.parse(url);

        final DownloadManager downloadManager =
                (DownloadManager)getSystemService(DOWNLOAD_SERVICE);

        downloadManager.enqueue(new DownloadManager.Request(srcUri)
                        .setAllowedNetworkTypes(DownloadManager.Request.NETWORK_WIFI |
                                DownloadManager.Request.NETWORK_MOBILE)
                        .setAllowedOverRoaming(false)
                        .setTitle(getString(R.string.download_title))
                        .setDescription(new File(dest, filename).getAbsolutePath())
                        .setDestinationUri(destUri));
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
