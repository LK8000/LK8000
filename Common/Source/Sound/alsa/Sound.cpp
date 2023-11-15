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

#define PCM_DEVICE "default"

static bool bSoundFile = false; // this is true only if "_System/_Sounds" directory exists.
static snd_pcm_t *pcm_handle = nullptr;

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

    if(!pcm_handle) {
        StartupStore(_T("------ LK8000 SOUNDS NOT WORKING!") NEWLINE);
    }
}

SoundGlobalInit::~SoundGlobalInit() {
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

namespace {

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

void alsa_play(SNDFILE* infile, SF_INFO& sfinfo) {

    /* Allocate parameters object and fill it with default values*/
    pcm_hw_params params;
    snd_pcm_hw_params_any(pcm_handle, params);
    /* Set parameters */
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, params, sfinfo.channels);
    snd_pcm_hw_params_set_rate(pcm_handle, params, sfinfo.samplerate, 0);

    unsigned int buffer_length_usec = 1000000;
    snd_pcm_hw_params_set_buffer_time_near(pcm_handle, params, &buffer_length_usec, nullptr);

    /* set hw parameters and prepare */
    snd_pcm_hw_params(pcm_handle, params);

    try {
        snd_pcm_uframes_t frames;
        /* Allocate buffer to hold sound */
        snd_pcm_hw_params_get_buffer_size(params, &frames);
        auto buf = std::make_unique<int16_t[]>(frames);
        int readcount;
        while ((readcount = sf_readf_short(infile, buf.get(), frames)) > 0) {

            int pcmrc = snd_pcm_writei(pcm_handle, buf.get(), readcount);
            if (pcmrc == -EPIPE) {
                fprintf(stderr, "PCM write underrun!\n");
                snd_pcm_prepare(pcm_handle);
            } else if (pcmrc < 0) {
                fprintf(stderr, "Error writing to PCM device: %s\n", snd_strerror(pcmrc));
            } else if (pcmrc != readcount) {
                fprintf(stderr, "PCM write differs from PCM read.\n");
            }
        }
        snd_pcm_drain(pcm_handle);
    }
    catch(std::exception& e) {
        fprintf(stderr, "PCM failed to allocate buffer : %s\n", e.what());
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

} // namespace

void PlayResource(const TCHAR* lpName) {
    if (!lpName || !EnableSoundModes || !pcm_handle) {
        return;
    }

    ConstBuffer<void> sndBuffer = GetNamedResource(lpName);
    if (!sndBuffer.IsEmpty()) {

        // Initialize the memory data
        MemoryInfos Memory = {
            static_cast<const uint8_t*> (sndBuffer.data),
            static_cast<const uint8_t*> (sndBuffer.data),
            static_cast<sf_count_t> (sndBuffer.size)
        };

        // Open the sound file
        SF_INFO sfinfo= {};
        SNDFILE* infile = sf_open_virtual(&VirtualIO, SFM_READ, &sfinfo, &Memory);
        if (infile) {
            alsa_play(infile, sfinfo);
            sf_close(infile);
        }
    }
}

void LKSound(const TCHAR *lpName) {
    if (!lpName || !bSoundFile || !EnableSoundModes || !pcm_handle) {
        return;
    }

    TCHAR srcfile[MAX_PATH];
    SystemPath(srcfile, TEXT(LKD_SOUNDS), lpName);

    SF_INFO sfinfo = {};
    SNDFILE* infile = sf_open(srcfile, SFM_READ, &sfinfo);
    if (infile) {
        alsa_play(infile, sfinfo);
        sf_close(infile);
    }
}
