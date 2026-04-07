/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "resource.h"
#include "ScreenGeometry.h"
#include "LocalPath.h"

#include "utils/filesystem.h"
#include <algorithm>
#include <cmath>
#include <vector>

#ifdef USE_GDI
#define IMG_EXT "BMP"
#else
#define IMG_EXT "PNG"
#endif

namespace {

void setFileSuffix(TCHAR* Out, size_t OutSize, PixelSize size);
LKBitmap LoadSplashBitmap(const TCHAR *path);

struct ScoredSplashCandidate {
    PixelSize size;
    double aspectError;
    double scaleError;
    bool orientationPenalty;
    bool upscalePenalty;
};

LKBitmap TryLoadSplashCandidate(const TCHAR* path, TCHAR* pSuffixStart, TCHAR* pSuffixEnd, PixelSize candidate) {
    setFileSuffix(pSuffixStart, std::distance(pSuffixStart, pSuffixEnd), candidate);
    return LoadSplashBitmap(path);
}

LKBitmap TryLoadSymbolicFallback(const TCHAR* path, TCHAR* pSuffixStart, TCHAR* pSuffixEnd) {
    const bool largeScreen = std::max(ScreenSizeX, ScreenSizeY) >= 800;
    const TCHAR* first = ScreenLandscape
        ? (largeScreen ? _T("_LB." IMG_EXT) : _T("_LS." IMG_EXT))
        : (largeScreen ? _T("_PB." IMG_EXT) : _T("_PS." IMG_EXT));

    size_t bufferSize = std::distance(pSuffixStart, pSuffixEnd);
    lk::snprintf(pSuffixStart, bufferSize, _T("%s"), first);
    LKBitmap bitmap = LoadSplashBitmap(path);
    if (bitmap.IsDefined()) {
        return bitmap;
    }

    const TCHAR* second = ScreenLandscape
        ? (largeScreen ? _T("_LS." IMG_EXT) : _T("_LB." IMG_EXT))
        : (largeScreen ? _T("_PS." IMG_EXT) : _T("_PB." IMG_EXT));

    lk::snprintf(pSuffixStart, bufferSize, _T("%s"), second);
    return LoadSplashBitmap(path);
}

bool IsBetterCandidate(const ScoredSplashCandidate& lhs, const ScoredSplashCandidate& rhs) {
    if (lhs.orientationPenalty != rhs.orientationPenalty) {
        return !lhs.orientationPenalty;
    }
    if (lhs.aspectError < rhs.aspectError) {
      return true;
    }
    if (rhs.aspectError < lhs.aspectError) {
      return false;
    }
    if (lhs.scaleError < rhs.scaleError) {
      return true;
    }
    if (rhs.scaleError < lhs.scaleError) {
      return false;
    }
    return !lhs.upscalePenalty && rhs.upscalePenalty;
}

void setFileSuffix(TCHAR* Out, size_t OutSize, PixelSize size) {
    lk::snprintf(Out, OutSize, _T("_%dx%d." IMG_EXT), size.cx, size.cy);
}

LKBitmap LoadSplashBitmap(const TCHAR *path) {
    LKBitmap bitmap;
#ifdef ANDROID
    bitmap.LoadAssetsFile(path);
#else
    bitmap.LoadFromFile(path);
#endif
    return bitmap;
}
 
} // namespace

LKBitmap LoadSplash(const TCHAR *splashfile) {

    TCHAR srcfile[MAX_PATH];
#ifdef ANDROID
    _tcscpy(srcfile, _T(LKD_BITMAPS DIRSEP));
#else
    SystemPath(srcfile, _T(LKD_BITMAPS DIRSEP));
#endif
    TCHAR* pSuffixStart = srcfile + _tcslen(srcfile); // end of path
    _tcscpy(pSuffixStart, splashfile); // add filename
    pSuffixStart += _tcslen(pSuffixStart);
    TCHAR* pSuffixEnd = std::end(srcfile);

    const PixelSize target = { 
        static_cast<PixelScalar>(ScreenSizeX),
        static_cast<PixelScalar>(ScreenSizeY)
    };
    const auto targetAspect = static_cast<double>(target.cx) / target.cy;

    // Scan available splash files and score each one against the target screen size
    const TCHAR* filters[] = { _T("." IMG_EXT) };
    const size_t prefixLen = _tcslen(splashfile);

    std::vector<ScoredSplashCandidate> scoredCandidates;

    const auto onFile = [&](const TCHAR* name, const TCHAR* /*relative_path*/) -> bool {
        // Match files named: splashfile + "_" + NNNxNNN + "." + ext
        if (_tcsnicmp(name, splashfile, prefixLen) != 0) {
            return true;
        }
        if (name[prefixLen] != _T('_')) {
            return true;
        }
        const TCHAR* p = name + prefixLen + 1;
        TCHAR* end = nullptr;
        int cx = _tcstol(p, &end, 10);
        if (end == p || *end != _T('x')) {
            return true;
        }

        TCHAR* heightEnd = nullptr;
        int cy = _tcstol(end + 1, &heightEnd, 10);
        if (heightEnd == end + 1 || *heightEnd != _T('.') || cx <= 0 || cy <= 0) {
            return true;
        }

        const bool candidateLandscape = cx >= cy;
        const double candidateAspect = static_cast<double>(cx) / cy;
        const double widthScale  = static_cast<double>(cx) / target.cx;
        const double heightScale = static_cast<double>(cy) / target.cy;

        scoredCandidates.push_back({
            PixelSize(cx, cy),
            std::fabs(candidateAspect - targetAspect),
            std::max(std::fabs(1.0 - widthScale), std::fabs(1.0 - heightScale)),
            candidateLandscape != ScreenLandscape,
            (widthScale < 1.0 || heightScale < 1.0),
        });

        return true;
    };

#ifdef ANDROID
    lk::filesystem::ScanZipDirectory(_T(LKD_SYS_BITMAPS), filters, 1, onFile);
#else
    {
        TCHAR srcdir[MAX_PATH];
        SystemPath(srcdir, _T(LKD_BITMAPS));
        lk::filesystem::ScanDirectories(srcdir, _T(""), filters, 1, onFile);
    }
#endif

    std::sort(scoredCandidates.begin(), scoredCandidates.end(), IsBetterCandidate);

    for (const ScoredSplashCandidate& candidate : scoredCandidates) {
        LKBitmap bitmap = TryLoadSplashCandidate(srcfile, pSuffixStart, pSuffixEnd, candidate.size);
        if (bitmap.IsDefined()) {
            return bitmap;
        }
    }

    return TryLoadSymbolicFallback(srcfile, pSuffixStart, pSuffixEnd);
}


void DrawSplash(LKSurface& Surface, const RECT& rcDraw, const LKBitmap& Bmp) {
    PixelRect rc(rcDraw);
    Surface.Blackness(rc.left,rc.top,rc.GetSize().cx,rc.GetSize().cy);
    if(Bmp) {
        const PixelSize bmSize = Bmp.GetSize();
        const PixelScalar cx = rc.GetSize().cx;
        Surface.DrawBitmap(rc.left,rc.top,cx,_MulDiv<short>(bmSize.cy,cx,bmSize.cx),Bmp,bmSize.cx,bmSize.cy);
    }
}
