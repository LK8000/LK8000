// Copyright (c) 2020-2021, Bruno de Lacheisserie
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of  nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "VarioPlayer.h"
#include <algorithm>

namespace {

    constexpr double ClimbToneOnThreshold = 0.1;
    constexpr double ClimbToneOffThreshold = 0.05;
    constexpr double SinkToneOnThreshold = -3;
    constexpr double SinkToneOffThreshold = -3;

    struct VarioTone {
        float Frequency; // (Hz)
        float CycleTime; // (s)
        float DutyCycle;
    };

    struct vario_ramp_t {
        double Vz;
        VarioTone tone;
    };

    constexpr vario_ramp_t vario_ramp[] = {
        { -10.00, {  200, 0.100, 1.00 } },
        {  -3.00, {  280, 0.100, 1.00 } },
        {  -0.51, {  300, 0.500, 1.00 } },
        {  -0.50, {  200, 0.800, 0.05 } },
        {   0.09, {  400, 0.600, 0.10 } },
        {   0.10, {  400, 0.600, 0.50 } },
        {   1.16, {  550, 0.552, 0.52 } },
        {   2.67, {  763, 0.483, 0.55 } },
        {   4.24, {  985, 0.412, 0.58 } },
        {   6.00, { 1234, 0.322, 0.62 } },
        {   8.00, { 1517, 0.241, 0.66 } },
        {  10.00, { 1800, 0.150, 0.70 } }
    };

    float linear_interpolation(float a, float b, float f) {
        const float of = 1 - f;
        return (f * b + of * a);
    }

    VarioTone linear_interpolation(const VarioTone &a, const VarioTone &b, double f) {
        return {
                linear_interpolation(a.Frequency, b.Frequency, f),
                linear_interpolation(a.CycleTime, b.CycleTime, f),
                linear_interpolation(a.DutyCycle, b.DutyCycle, f)
        };
    }

    template<typename ramp_t, size_t size>
    VarioTone ToneLookup(const double v, const ramp_t(&ramp_tone)[size]) {

        const auto begin = std::begin(ramp_tone);
        const auto end = std::end(ramp_tone);

        // find first value with value > v
        auto it = std::upper_bound(begin, end, v, [](double v, const ramp_t &item) {
            return v < item.Vz;
        });

        if (it != begin) {
            auto prev = std::prev(it);
            if (it != end && it->Vz != prev->Vz) {
                // interpolate color
                const double f = (v - prev->Vz) / (it->Vz - prev->Vz);
                return linear_interpolation(prev->tone, it->tone, f);
            } else {
                return prev->tone; // last defined color or no need to interpolate
            }
        }
        return begin->tone; // first defined color
    }
}

void VarioPlayer::Open() {
    oboe::AudioStreamBuilder builder;

    builder.setSharingMode(oboe::SharingMode::Shared);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setChannelCount(oboe::ChannelCount::Mono);
    builder.setDataCallback(shared_from_this());

    oboe::Result result = builder.openStream(mOutStream);
    if (result == oboe::Result::OK) {
        assert(mOutStream->getChannelCount() == 1);

        mOutStream->setBufferSizeInFrames(mOutStream->getFramesPerBurst());
        mOscillator.SetSampleRate(mOutStream->getSampleRate());

        mOutStream->requestStart();
    }
}

VarioPlayer::~VarioPlayer() {
    Close();
}

void VarioPlayer::Close() {
    if (mOutStream) {
        mOutStream->stop();
        mOutStream->close();
    }
    mOutStream = nullptr;
}

void VarioPlayer::UpdateTone() {
    const double vz = mVz;

    // manage dead-band hysteresis
    if (mIsOn) {
        if (vz > SinkToneOffThreshold && vz < ClimbToneOffThreshold) {
            mIsOn = false;
            mPeriodTotal = 0;
            mPeriodOn = 0;
        }
    } else {
        if(vz < SinkToneOnThreshold || vz > ClimbToneOnThreshold) {
            mIsOn = true;
        }
    }

    if (mIsOn) {
        const VarioTone tone = ToneLookup(vz, vario_ramp);
        mOscillator.SetFrequency(tone.Frequency);

        mPeriodTotal = tone.CycleTime * mOscillator.GetSampleRate();
        mPeriodOn = mPeriodTotal * tone.DutyCycle;
    }
}

oboe::DataCallbackResult VarioPlayer::onAudioReady(oboe::AudioStream *Stream, void *Data, int32_t Frames) {
    auto begin = static_cast<float *>(Data);
    auto end = std::next(begin, Frames);

    std::for_each(begin, end, [&](float& data) {
        data = ((mPeriod++) < mPeriodOn && mIsOn) ? mOscillator.Next() : 0;
        if (mPeriod >= mPeriodTotal) {
            mPeriod = 0;
            UpdateTone();
        }
    });
    return oboe::DataCallbackResult::Continue;
}
