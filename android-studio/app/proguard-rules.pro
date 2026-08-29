# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in /home/bruno/x-tools/android/android-sdk-linux/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# Keep all org.LK8000 classes and their members (accessed from native code via JNI)
-keep class org.LK8000.** {
    *;
}
