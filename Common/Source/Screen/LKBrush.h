/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBrush.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 23 octobre 2014, 20:28
 */

#ifndef LKBRUSH_H
#define	LKBRUSH_H
class LKColor;
class LKBitmap;


class LKBrush {
public:
    LKBrush();
	
    LKBrush(LKBrush&& Brush); // tranfert ownership
    LKBrush& operator= (LKBrush&& Brush); // tranfert ownership

    explicit LKBrush(const LKColor& Color);
    
    virtual ~LKBrush();
    
    void Create(const LKColor& Color);
    void Create(const LKBitmap& Bitmap);

    void Release();
    
    operator bool() const;
    
#ifdef WIN32
public:
    explicit LKBrush(HBRUSH Brush) : _Brush(Brush), _Destroy(false) {}

    static LKBrush MakeStock(int fnObject) {
      return LKBrush((HBRUSH)GetStockObject(fnObject));
    }

    operator HBRUSH() const { return _Brush; }

protected:
    HBRUSH _Brush;
    bool _Destroy;
#endif

};

#ifdef WIN32
inline LKBrush::operator bool() const { return (_Brush != NULL); }
#endif

extern const LKBrush  LK_WHITE_BRUSH;
extern const LKBrush  LK_BLACK_BRUSH;
extern const LKBrush  LK_HOLLOW_BRUSH;

#endif	/* LKBRUSH_H */

