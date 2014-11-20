/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKPen.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 octobre 2014
 */

#ifndef LKPEN_H
#define	LKPEN_H

class LKColor;

enum enumType {
    PEN_SOLID = 0,
    PEN_DASH = 1
};


class LKPen {
public:
    LKPen():_Pen() {}

    LKPen(LKPen&& Pen);
    LKPen& operator= (LKPen&& Pen);
    
    LKPen(enumType Type, unsigned Size, const LKColor& Color);
    ~LKPen();

    void Create(enumType Type, unsigned Size, const LKColor& Color);
    void Release();
    
#ifdef WIN32
public:
	explicit LKPen(HPEN Pen) : _Pen(Pen) {}

    static LKPen MakeStock(int fnObject) {
      return LKPen((HPEN)GetStockObject(fnObject));
    }

	operator HPEN() const { return _Pen; }
	operator bool() const { return (_Pen != NULL); }
    
protected:
    HPEN _Pen;
#else
    operator bool() const;
#endif
    

};

extern const LKPen LK_NULL_PEN;
extern const LKPen LK_BLACK_PEN;
extern const LKPen LK_WHITE_PEN;

#endif	/* LKPEN_H */

