#ifndef INFOBOXLAYOUT_H
#define INFOBOXLAYOUT_H

#include "Screen/FontReference.h"

namespace ButtonLabel {

    void CreateButtonLabels(const PixelRect& rcScreen);
    void SetFont(FontReference Font);
    void Destroy();

    void SetLabelText(unsigned MenuID, const TCHAR *text);

    bool IsVisible(unsigned MenuID);
    bool IsEnabled(unsigned MenuID);

};

#endif
