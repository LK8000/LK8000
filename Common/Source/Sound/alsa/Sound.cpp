/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sound.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on November 26, 2015, 00:19 AM
 */

#include "../Sound.h"
#include "externs.h"
#include "Util/ConstBuffer.hpp"
#include "resource_data.h"
#include <alsa/asoundlib.h>
#include <sndfile.h>
#include <optional>
#include "Thread/Thread.hpp"
#include "Thread/Cond.hpp"

#define PCM_DEVICE "default"

namespace {

bool bSoundFile = false;  // this is true only if "_System/_Sounds" directory exists.
snd_pcm_t* pcm_handle = nullptr;

class pcm_hw_params {
 public: 
  pcm_hw_params() {
    snd_pcm_hw_params_malloc(&params);
  }
  
  ~pcm_hw_params() {
    snd_pcm_hw_params_free(params);
  }

  operator snd_pcm_hw_params_t* () {
    return params;
  }

 private:
  snd_pcm_hw_params_t *params;
};

class pcm_sw_params {
 public: 
  pcm_sw_params() {
    snd_pcm_sw_params_malloc(&params);
  }
  
  ~pcm_sw_params() {
    snd_pcm_sw_params_free(params);
  }

  operator snd_pcm_sw_params_t* () {
    return params;
  }

 private:
  snd_pcm_sw_params_t *params;
};

void init_hw_params(const SF_INFO& sfinfo) {
  /* Allocate parameters object and fill it with default values*/
  pcm_hw_params hw_params;
  snd_pcm_hw_params_any(pcm_handle, hw_params);
  /* Set parameters */
  snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
  snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_channels(pcm_handle, hw_params, sfinfo.channels);
  snd_pcm_hw_params_set_rate(pcm_handle, hw_params, sfinfo.samplerate, 0);

  /* Makes sure buffer frames is even, or snd_pcm_hw_params will return invalid argument error. */
  snd_pcm_uframes_t buffer_frames;
  snd_pcm_hw_params_get_buffer_size_max(hw_params, &buffer_frames);
  buffer_frames &= ~0x01;
  snd_pcm_hw_params_set_buffer_size_max(pcm_handle, hw_params, &buffer_frames);

  /* set hw parameters and prepare */
  snd_pcm_hw_params(pcm_handle, hw_params);
}

void init_sw_params() {
  pcm_sw_params sw_params;
  snd_pcm_sw_params_current(pcm_handle, sw_params);
  snd_pcm_uframes_t boundary;
  snd_pcm_sw_params_get_boundary(sw_params, &boundary);
  snd_pcm_sw_params_set_stop_threshold(pcm_handle, sw_params, boundary);
  snd_pcm_sw_params_set_start_threshold(pcm_handle, sw_params, boundary);
  snd_pcm_sw_params_set_period_event(pcm_handle, sw_params, 0);
  snd_pcm_sw_params(pcm_handle, sw_params);
}

void wait_for_poll(snd_pcm_t* handle, struct pollfd* ufds, unsigned int count) {
  unsigned short revents;
  while (1) {
    poll(ufds, count, -1);
    snd_pcm_poll_descriptors_revents(handle, ufds, count, &revents);
    if (revents & POLLERR) {
      throw std::runtime_error("poll error");
    }
    if (revents & POLLOUT) {
      return;
    }
  }
}

int snd_recovery(snd_pcm_t* handle, int err) {
  if (err == -EPIPE) { /* under-run */
    err = snd_pcm_prepare(handle);
  }
  if (err < 0) {
    throw std::runtime_error(std::string("Can't recovery : ") + snd_strerror(err));
  }
  return err;
}

void alsa_play(SNDFILE* infile, const SF_INFO& sfinfo) {
  init_hw_params(sfinfo);
  init_sw_params();

  snd_pcm_prepare(pcm_handle);
  snd_pcm_start(pcm_handle);

  try {
    int count = snd_pcm_poll_descriptors_count(pcm_handle);
    auto ufds = std::make_unique<pollfd[]>(count);
    snd_pcm_poll_descriptors(pcm_handle, ufds.get(), count);

    snd_pcm_uframes_t to_write = sfinfo.frames;
    while (to_write > 0) {
      wait_for_poll(pcm_handle, ufds.get(), count);

      auto available = snd_pcm_avail(pcm_handle);
      if (available < 0) {
        available = snd_recovery(pcm_handle, available);
      }
      if (available <= 0) {
        continue;
      }

      const snd_pcm_channel_area_t* areas;
      snd_pcm_uframes_t offset;
      snd_pcm_uframes_t frames = to_write;
      int err = snd_pcm_mmap_begin(pcm_handle, &areas, &offset, &frames);
      if (err < 0) {
        err = snd_recovery(pcm_handle, available);
      }
      if (err < 0) {
        continue;
      }

      auto buff = static_cast<int16_t*>(areas->addr);
      ssize_t buffsize = snd_pcm_frames_to_bytes(pcm_handle, frames) / sizeof(int16_t);
      ssize_t buffoffset = snd_pcm_frames_to_bytes(pcm_handle, offset) / sizeof(int16_t);

      int readcount = sf_readf_short(infile, buff + buffoffset, buffsize);
      snd_pcm_mmap_commit(pcm_handle, offset, readcount);
      to_write -= readcount;
    }
    snd_pcm_drain(pcm_handle);
  } catch (std::exception& e) {
    fprintf(stderr, "PCM %s\n", e.what());
  }
  snd_pcm_hw_free(pcm_handle);
}

////////////////////////////////////////////////////////////////////
/// Functions for implementing custom read and write to memory files
////////////////////////////////////////////////////////////////////

struct MemoryInfos {
    const uint8_t* DataStart;
    const uint8_t* DataPtr;
    sf_count_t TotalSize;
};

sf_count_t vio_get_filelen(void* UserData) {
    MemoryInfos* Memory = static_cast<MemoryInfos*> (UserData);

    return Memory->TotalSize;
}

sf_count_t vio_read(void* Ptr, sf_count_t Count, void* UserData) {
    MemoryInfos* Memory = static_cast<MemoryInfos*> (UserData);

    sf_count_t Position = Memory->DataPtr - Memory->DataStart;
    if (Position + Count >= Memory->TotalSize) {
        Count = Memory->TotalSize - Position;
    }

    memcpy(Ptr, Memory->DataPtr, static_cast<std::size_t> (Count));

    Memory->DataPtr += Count;

    return Count;
}

sf_count_t vio_seek(sf_count_t Offset, int Whence, void* UserData) {
    MemoryInfos* Memory = static_cast<MemoryInfos*> (UserData);

    sf_count_t Position = 0;
    switch (Whence) {
        case SEEK_SET:
            Position = Offset;
            break;
        case SEEK_CUR:
            Position = Memory->DataPtr - Memory->DataStart + Offset;
            break;
        case SEEK_END:
            Position = Memory->TotalSize - Offset;
            break;
        default:
            Position = 0;
            break;
    }

    if (Position >= Memory->TotalSize) {
        Position = Memory->TotalSize - 1;
    } else if (Position < 0) {
        Position = 0;
    }
    Memory->DataPtr = Memory->DataStart + Position;

    return Position;
}

sf_count_t vio_tell(void* UserData) {
    MemoryInfos* Memory = static_cast<MemoryInfos*> (UserData);
    return Memory->DataPtr - Memory->DataStart;
}

sf_count_t vio_write(const void*, sf_count_t, void*) {
    return 0;
}

// Define the I/O custom functions for reading from memory
SF_VIRTUAL_IO VirtualIO = {
    &vio_get_filelen,
    &vio_seek,
    &vio_read,
    &vio_write,
    &vio_tell
};

void play_file(const std::string& name) {
  TCHAR srcfile[MAX_PATH];
  SystemPath(srcfile, TEXT(LKD_SOUNDS), name.c_str());

  SF_INFO sfinfo = {};
  SNDFILE* infile = sf_open(srcfile, SFM_READ, &sfinfo);
  if (infile) {
    alsa_play(infile, sfinfo);
    sf_close(infile);
  }
}

void play_resource(const std::string& name) {
  ConstBuffer<void> sndBuffer = GetNamedResource(name.c_str());
  if (!sndBuffer.IsEmpty()) {
    // Initialize the memory data
    MemoryInfos Memory = {
      static_cast<const uint8_t*>(sndBuffer.data),
      static_cast<const uint8_t*>(sndBuffer.data),
      static_cast<sf_count_t>(sndBuffer.size)
    };

    // Open the sound file
    SF_INFO sfinfo = {};
    SNDFILE* infile = sf_open_virtual(&VirtualIO, SFM_READ, &sfinfo, &Memory);
    if (infile) {
      alsa_play(infile, sfinfo);
      sf_close(infile);
    }
  }
}

enum class sound_type {
  ressource,
  file
};

struct sound_item {
  sound_type type; 
  std::string name;
};

void play_sound(const sound_item& item) {
  switch (item.type) {
    case sound_type::file:
      play_file(item.name);
      break;
    case sound_type::ressource:
      play_resource(item.name);
      break;
  }
}

class ThreadSound : public Thread {
public:
  ThreadSound() : Thread("Sound") {}

  bool Start() override {
    thread_stop = false;
    return Thread::Start();
  }

  void Queue(sound_type type, std::string name) {
    WithLock(queue_mtx, [&]() {
      queue = { type, std::move(name) };
    });
    queue_cv.Broadcast();
  }

  void Stop() {
    WithLock(queue_mtx, [&]() {
      thread_stop = true;
    });
    queue_cv.Broadcast();
  }

private:
  bool thread_stop = false;
  std::optional<sound_item> queue;
  Mutex queue_mtx;
  Cond queue_cv;

  void Run() override {
    while (true) {
      // get copy of queue
      auto sound = WithLock(queue_mtx, [&]() {
        return std::exchange(queue, std::nullopt);
      });

      if (!sound) {
        ScopeLock lock(queue_mtx);
        // no sound check for stop request
        if (thread_stop) {
          return;  // stop requested...
        }
        // wait for stop or sound
        queue_cv.Wait(queue_mtx);
      } else {
        play_sound(sound.value());
      }
    }
  }
};

ThreadSound thread_sound;

}  // namespace

void PlayResource(const TCHAR* lpName) {
  if (!lpName || !EnableSoundModes || !pcm_handle) {
    return;
  }
  thread_sound.Queue(sound_type::ressource, lpName);
}

void LKSound(const TCHAR* lpName) {
  if (!lpName || !bSoundFile || !EnableSoundModes || !pcm_handle) {
    return;
  }
  thread_sound.Queue(sound_type::file, lpName);
}

SoundGlobalInit::SoundGlobalInit() {
  TCHAR srcfile[MAX_PATH];
  SystemPath(srcfile, TEXT(LKD_SOUNDS), TEXT("_SOUNDS"));
  if (lk::filesystem::exist(srcfile)) {
    bSoundFile = true;
  } else {
    StartupStore(_T("ERROR NO SOUNDS DIRECTORY CHECKFILE <%s>") NEWLINE, srcfile);
  }

  /* Open the PCM device in playback mode */
  int pcmrc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
  if (pcmrc < 0) {
    StartupStore(_T("failed to open PCM device <%s>") NEWLINE, snd_strerror(pcmrc));
    pcm_handle = nullptr;
  }

  if (pcm_handle) {
    thread_sound.Start();
  }

  if (!pcm_handle) {
    StartupStore(_T("------ LK8000 SOUNDS NOT WORKING!") NEWLINE);
  }
}

SoundGlobalInit::~SoundGlobalInit() {
  if (thread_sound.IsDefined()) {
    thread_sound.Stop();
    thread_sound.Join();
  }

  if (pcm_handle) {
    snd_pcm_close(pcm_handle);
  }
  snd_config_update_free_global();
}

bool IsSoundInit() {
  return pcm_handle;
}

bool SetSoundVolume() {
  return false;
}
