/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKFont.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 23 octobre 2014, 20:28
 */

#ifndef LKFONT_H
#define	LKFONT_H

class LKFont {
public:
    LKFont();
    LKFont(LKFont&& Font);
    LKFont(const LKFont& Font);
    virtual ~LKFont();

    LKFont& operator=(LKFont&& Font);
    LKFont& operator=(const LKFont& Font);
    void Release();
    
#ifdef WIN32        
public:
    explicit LKFont(HFONT Font) : _Font(Font), _Destroy(false) {}
    
    void Create(LOGFONT* pLogFont);
    
    operator HFONT() const { return _Font; } 
	operator bool() const { return (_Font != NULL); }
    
protected:
    HFONT _Font;
    bool _Destroy;
    
#endif
};

#endif	/* LKFONT_H */

