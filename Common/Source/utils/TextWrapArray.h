/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   TextWrapArray.h
 * Author: Bruno de Lacheisserie
 *
 * Created on April 9, 2015, 12:19 AM
 */

#ifndef TEXTWRAPARRAY_H
#define	TEXTWRAPARRAY_H

#include "tchar.h"
#include <vector>
#include <stddef.h>
#include "Compiler.h"

class LKSurface;

class TextWrapArray final {
public:
    TextWrapArray();
    ~TextWrapArray();

    TextWrapArray(TextWrapArray&& src) = delete;
    TextWrapArray& operator= (TextWrapArray&& src) = delete;
    
    TextWrapArray(const TextWrapArray&) = delete;
    TextWrapArray& operator= (const TextWrapArray&) = delete;

    void update(LKSurface& Surface, int MaxWidth, const TCHAR* sText);
    
    void clear();
    
    inline size_t size() const { return _array.size(); }
    
    const TCHAR* operator[](size_t idx) const { return _array[idx]; }
    
private:
    TCHAR* _szText;
    std::vector<const TCHAR*> _array;
};

#endif	/* TEXTWRAPARRAY_H */

