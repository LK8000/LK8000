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

#ifndef _OSCILLATORSQUARE_H_
#define _OSCILLATORSQUARE_H_

#include <cassert>

class OscillatorSquare {
private:
    float mSampleRate = 0.0f;

    float mFrequency = 440.0f;

    float mFractionFrequency = 1.0f; // mFrequency / mSampleRate
    float mPhase = 0.0f;

public:
    OscillatorSquare() = default;

    void SetSampleRate(float rate) {
        if (rate != mSampleRate) {
            mSampleRate = rate;
            mFractionFrequency = mFrequency / mSampleRate;
        }
    }

    float GetSampleRate() const {
        return mSampleRate;
    }

    void SetFrequency(float hz) {
        if (hz != mFrequency) {
            mFrequency = hz;
            mFractionFrequency = mFrequency / mSampleRate;
        }
    }

    const float Next() {
        assert(mSampleRate > 0.0f && "Samplerate must be greater than 0");
        assert(mFrequency < mSampleRate && "Frequency must be less than mSampleRate ");

        // Increase mPhase by +1 step and clamp to boundaries [0,1]
        mPhase += mFractionFrequency;
        if (mPhase > 1.0f) {
            mPhase = 0.0f;
        }
        return ((mPhase < 0.5f) ? -0.1f : 0.1f);
    }
};

#endif // _OSCILLATORSQUARE_H_
