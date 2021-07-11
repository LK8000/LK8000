/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * Created by Bruno de Lacheisserie on 21/10/2019.
 */

#ifndef _TERRAIN_STHEIGHTBUFFER_H_
#define _TERRAIN_STHEIGHTBUFFER_H_
#include "Compiler.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <assert.h>

class CSTHeightBuffer final {
public:
    CSTHeightBuffer(size_t w, size_t height);

    void Resize(size_t width, size_t height);

    int16_t* GetRow(size_t row) const {
        assert(row * width < allocated);
        return &data[row * width];
    }

    int16_t* GetBuffer() const {
        return data.get();
    }

    size_t GetWidth() const {
        return width;
    }

    size_t GetHeight() const {
        return height;
    }

private:

    std::unique_ptr<int16_t[]> data;

    size_t width;
    size_t height;
    size_t allocated;
};

#endif //_TERRAIN_STHEIGHTBUFFER_H_
