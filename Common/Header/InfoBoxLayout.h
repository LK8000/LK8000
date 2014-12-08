#ifndef INFOBOXLAYOUT_H
#define INFOBOXLAYOUT_H

#include "Screen/FontReference.h"

namespace ButtonLabel {

    void CreateButtonLabels(const RECT& rc);
    void SetFont(FontReference Font);
    void Destroy();

    void SetLabelText(unsigned idx, const TCHAR *text);

    bool IsVisible(unsigned idx);
    bool IsEnabled(unsigned idx);

    void GetButtonPosition(unsigned idw, const RECT& rc, int *x, int *y, int *sizex, int *sizey);
};

#endif
