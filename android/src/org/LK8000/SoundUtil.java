/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

package org.LK8000;

import android.annotation.TargetApi;
import android.content.res.AssetFileDescriptor;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.content.Context;
import android.media.SoundPool;
import android.media.SoundPool.OnLoadCompleteListener;
import android.os.Build;
import androidx.annotation.Nullable;
import android.util.Log;

import java.util.HashMap;

public class SoundUtil {

    private static final String TAG = "SoundUtil";

    private static SoundUtil instance;

    private SoundPool soundPool;
    private HashMap<String, Integer> loadedSound = new HashMap<>();

    public static void Initialise() {
        instance = new SoundUtil();
    }

    public static void Deinitialise() {
        if (instance != null) {
            instance = null;
        }
    }

    public static boolean play(Context context, String name) {
        if (instance != null) {
            return instance._play(context, name);
        }
        return false;
    }

    @Nullable @TargetApi(21)
    private static SoundPool createSoundPool() {
        AudioAttributes attributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_NOTIFICATION_EVENT)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build();
        return new SoundPool.Builder()
                .setAudioAttributes(attributes)
                .setMaxStreams(5) // only five simultaneous sound ...
                .build();
    }

    @Nullable @SuppressWarnings("deprecation")
    private static SoundPool createSoundPoolCompat() {
        return new SoundPool(5, AudioManager.STREAM_MUSIC, 0);
    }


    SoundUtil() {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                soundPool = createSoundPool();
            } else {
                soundPool = createSoundPoolCompat();
            }

            soundPool.setOnLoadCompleteListener(new OnLoadCompleteListener() {
                public void onLoadComplete(SoundPool soundPool, int sampleId, int status) {
                    soundPool.play(sampleId, 1.0f, 1.0f, 0, 0, 1.0f);
                }
            });

        } catch (Exception e) {
            Log.e(TAG, e.toString());
            soundPool = null;
        }
    }

    private boolean _play(Context context, String name) {
        if (soundPool == null) {
            return false;
        }
        Integer soundId = loadedSound.get(name);
        if (soundId == null) {
            try {
                AssetFileDescriptor descriptor = context.getAssets().openFd("sounds/" + name);
                soundId = soundPool.load(descriptor, 0);
                loadedSound.put(name, soundId);
            } catch (Exception e) {
                Log.e(TAG, e.toString());
                return false;
            }
        } else {
            soundPool.play(soundId.intValue(), 1.0f, 1.0f, 0, 0, 1.0f);
        }
        return true;
    }

}
