package org.LK8000.QRCode;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.Preview;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.camera.view.PreviewView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import org.LK8000.databinding.ActivityQrcodeScannerBinding;
import com.google.common.util.concurrent.ListenableFuture;

import java.util.concurrent.ExecutionException;

@TargetApi(21)
public class QRCodeScannerActivity extends AppCompatActivity {
    private static final int PERMISSION_REQUEST_CAMERA = 0;

    private ActivityQrcodeScannerBinding binding;

    private ListenableFuture<ProcessCameraProvider> cameraProviderFuture;
    private int lensFacing = CameraSelector.LENS_FACING_BACK;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityQrcodeScannerBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        cameraProviderFuture = ProcessCameraProvider.getInstance(this);

        binding.switchCamera.setOnClickListener((view) -> {
            lensFacing = (CameraSelector.LENS_FACING_FRONT == lensFacing)
                    ? CameraSelector.LENS_FACING_BACK
                    : CameraSelector.LENS_FACING_FRONT;
            requestCamera();
        });

        requestCamera();
    }

    private void requestCamera() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
            startCamera();
        } else {
            if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.CAMERA)) {
                ActivityCompat.requestPermissions(QRCodeScannerActivity.this, new String[]{Manifest.permission.CAMERA}, PERMISSION_REQUEST_CAMERA);
            } else {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, PERMISSION_REQUEST_CAMERA);
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == PERMISSION_REQUEST_CAMERA) {
            if (grantResults.length == 1 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                startCamera();
            } else {
                Toast.makeText(this, "Camera Permission Denied", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void startCamera() {
        cameraProviderFuture.addListener(() -> {
            try {
                bindCameraPreview(cameraProviderFuture.get());
            } catch (ExecutionException | InterruptedException e) {
                Toast.makeText(this, "Error starting camera " + e.getMessage(), Toast.LENGTH_SHORT).show();
            }
        }, ContextCompat.getMainExecutor(this));
    }

    private void bindCameraPreview(ProcessCameraProvider cameraProvider) {
        binding.imagePreviewView.setImplementationMode(PreviewView.ImplementationMode.COMPATIBLE);

        Preview preview = new Preview.Builder()
                .build();

        CameraSelector cameraSelector =
                new CameraSelector.Builder()
                        .requireLensFacing(lensFacing)
                        .build();

        preview.setSurfaceProvider(binding.imagePreviewView.getSurfaceProvider());

        ImageAnalysis imageAnalysis =
                new ImageAnalysis.Builder()
                        .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                        .build();

        imageAnalysis.setAnalyzer(ContextCompat.getMainExecutor(this),
                new QRCodeImageAnalyzer(_qrCode -> {
                    Intent returnIntent = new Intent();
                    returnIntent.putExtra("data", _qrCode);
                    setResult(Activity.RESULT_OK, returnIntent);
                    finish();
                }));

        cameraProvider.unbindAll();
        cameraProvider.bindToLifecycle(this, cameraSelector, imageAnalysis, preview);
    }
}
