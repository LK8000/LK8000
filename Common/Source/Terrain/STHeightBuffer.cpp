/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * Created by Bruno de Lacheisserie on 21/10/2019.
 */

#include "STHeightBuffer.h"
#include <memory>

CSTHeightBuffer::CSTHeightBuffer(size_t w, size_t h)
        : data(std::make_unique<int16_t[]>(w * h)),
        width(w), height(h), allocated(w*h)
{

}

void CSTHeightBuffer::Resize(size_t w, size_t h) {
    if((w * h) > allocated) {
        data = std::make_unique<int16_t[]>(w * h);
        allocated = w * h;
    }
    width = w;
    height = h;
}
