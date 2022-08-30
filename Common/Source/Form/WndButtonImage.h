/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndButtonImage.h
 * Author: Bruno de Lacheisserie
 *
 * Created on March 10, 2024
 */

#ifndef FORM_WNDBUTTONIMAGE_H
#define	FORM_WNDBUTTONIMAGE_H

#include "WindowControls.h"
#include "Screen/LKIcon.h"

class WndButtonImage : public WndButton {
public:
  using WndButton::WndButton;

protected:
  void Paint(LKSurface& Surface) override;

private:
  LKIcon Icon;
};

#endif // FORM_WNDBUTTONIMAGE_H
