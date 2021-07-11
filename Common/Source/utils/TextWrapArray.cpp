/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   TextWrapArray.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on April 9, 2015, 12:19 AM
 */

#include "TextWrapArray.h"
#include <string.h>
#include "Screen/LKSurface.h"


TextWrapArray::TextWrapArray() : _szText(nullptr) {

}

void TextWrapArray::update(LKSurface& Surface, int MaxWidth, const TCHAR* sText) {
    clear();
           
    if (!sText) return;
    
    _szText = _tcsdup(sText);
    
    TCHAR* pStart = _szText;
    TCHAR* pLast = _szText+_tcslen(_szText);
    
    while(pStart < pLast) {
        TCHAR* pEnd = _tcschr(pStart, _T('\n'));
        if(pEnd) { // explicit line break;
            if((pEnd>pStart) && (*(pEnd-1) == _T('\r'))) {
                *(pEnd-1) = _T('\0'); // windows <cr><lf> 
            }
            *pEnd = _T('\0'); // unix <lf>
        } else {
            pEnd = _tcschr(pStart, _T('\r'));
            if(pEnd) {
                *pEnd = _T('\0'); // mac <cr>
            } else {
                pEnd = pStart+_tcslen(pStart);
            }
        }
        
        TCHAR* pPrevSpace = nullptr; 
        while(Surface.GetTextWidth(pStart) > MaxWidth) {
            pEnd = _tcsrchr(pStart, _T(' '));
            if(pEnd) {
                if(pPrevSpace) {
                    *pPrevSpace = _T(' ');
                }
                pPrevSpace = pEnd;
                *pEnd = _T('\0');
            } else {
                break; // first word is to large, wrap to first space.
            }
        }
        
        _array.push_back(pStart); // new line
        
        pStart+=_tcslen(pStart);
        if(!(*pStart)) {
            pStart++;
        }
    }
}

TextWrapArray::~TextWrapArray() {
    clear();
}

void TextWrapArray::clear() {
    _array.clear();
    if(_szText) {
        free(_szText);
        _szText = nullptr;
    }
}
