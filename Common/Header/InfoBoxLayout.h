#ifndef INFOBOXLAYOUT_H
#define INFOBOXLAYOUT_H

#include "Screen/FontReference.h"

namespace ButtonLabel {

    void CreateButtonLabels(const PixelRect& rcScreen);
    void SetFont(FontReference Font);
    void Destroy();

    bool IsVisible();

    void SetLabelText(unsigned MenuID, const TCHAR *text);

    bool IsVisible(unsigned MenuID);
    bool IsEnabled(unsigned MenuID);

    unsigned GetNextMenuId(unsigned MenuID);
    unsigned GetPrevMenuId(unsigned MenuID);
}

#endif
