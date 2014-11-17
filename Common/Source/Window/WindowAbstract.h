/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WindowAbstract.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 novembre 2014, 14:19
 */

#ifndef WINDOWABSTRACT_H
#define	WINDOWABSTRACT_H

class LKSurface;

class WindowAbstrat {
public:
    WindowAbstrat() {}

    WindowAbstrat( const WindowAbstrat& ) = delete;
    const WindowAbstrat& operator=( const WindowAbstrat& ) = delete;

    virtual void Close() = 0;
    virtual void Destroy() = 0;

    virtual void SetToForeground() = 0;
    virtual void SetFocus() = 0;

    virtual void Move(const RECT& rc, bool bRepaint) = 0;

    virtual RECT GetClientRect() const = 0;

    virtual void SetFont(const LKFont& Font) = 0;
    virtual void Redraw(const RECT& Rect) = 0;
    
    virtual void SetText(const TCHAR* lpszText) = 0;

protected:
    // Event Handling virtual function ( return true for ignore default process ) :
    virtual bool OnCreate(int x, int y, int cx, int cy) { return false; }
    virtual bool OnClose() { return false; }
    virtual bool OnDestroy() { return false; }
    virtual bool OnSize(int cx, int cy) { return false; }

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) { return false; }

    virtual bool OnKillFocus() { return false; }

	virtual bool OnMouseMove(const POINT& Pos) { return false; }

	virtual bool OnLButtonDown(const POINT& Pos) { return false; }
    virtual bool OnLButtonUp(const POINT& Pos) { return false; }

	virtual bool OnLButtonDblClick(const POINT& Pos) { return false; }

    virtual bool OnKeyDown(unsigned KeyCode) { return false; }

private:

};

#endif	/* WINDOWABSTRACT_H */
