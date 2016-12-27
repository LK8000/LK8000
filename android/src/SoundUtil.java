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

import android.content.res.AssetFileDescriptor;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.content.Context;
import android.util.Log;

public class SoundUtil {

    private static final String TAG = "LK8000";

    private static  class playSoundThread implements Runnable {
        Context _context;
        String _name;
        public playSoundThread(Context context, String name) {
            _context = context;
            _name = name;
        }

        public void run() {
            try {
                MediaPlayer m = new MediaPlayer();
                AssetFileDescriptor descriptor = _context.getAssets().openFd("sounds/" + _name );
                m.setDataSource(descriptor.getFileDescriptor(),descriptor.getStartOffset(), descriptor.getLength());
                descriptor.close();

                m.prepare();
                m.setLooping(false);
                m.setAudioStreamType(AudioManager.STREAM_MUSIC);
                m.start();

                while (m.isPlaying()) {
                    Thread.sleep(10);
                }
                Thread.sleep(10);
                m.release();
                m = null;

            } catch (Exception e) {
                Log.e(TAG, e.toString());
                return ;
            }
        }
    }


  public static boolean play(Context context, String name) {
    Runnable r = new playSoundThread(context,name);
    new Thread(r).start();
    return true;
  }
}
