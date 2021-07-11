/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */


#include "externs.h"
#include "LKObjects.h"
#include "DoInits.h"
#include "Screen/PenReference.h"
#include "Screen/BrushReference.h"
#include "Bitmaps.h"
#include "Util/Clamp.hpp"
#include "Calc/Vario.h"

#define BOXTHICK 1
#define PIXELSEPARATE 1
#define POSCOLOR 6

void MapWindow::LKDrawVario(LKSurface& Surface, const RECT& rc) {

    static PixelRect vrc, mrc, hrc, htrc, hbrc;

    static BrushReference greenBrush, darkyellowBrush, orangeBrush, redBrush;
    static BrushReference lakeBrush, blueBrush, indigoBrush;

    static PenReference borderPen; // Pen for border of vario bar and vario bricks ( white or black)
    static BrushReference forgroundBrush; // Brush used for draw middle thick or monochrome brick ( same color of borderPen )

    /* 
     * this array define vario Value for each brick, ( positive value )
     *  Number of brick for positive value is defined by this array size.
     */
    static const double positive_vario_step[] = {0.05, 0.25, 0.50, 0.75, 1.00, 1.25, 1.50, 1.75, 2.00, 2.50, 3.00, 3.50, 4.50, 5.00, 6.00, 7.00};
    static const unsigned positive_brick_count = std::size(positive_vario_step);

    static BrushReference positiveBrush[positive_brick_count];
    static PixelRect positiveBricks[positive_brick_count];

    /* 
     * this array define vario Value for each brick, ( negative value )
     *  Number of brick for negative value is defined by this array size.
     */
    static const double negative_vario_step[] = {-0.05, -0.25, -0.50, -0.75, -1.00, -1.25, -1.50, -1.75, -2.00, -2.50, -3.00, -3.50, -4.50, -5.00, -6.00, -7.00};
    static const unsigned negative_brick_count = std::size(negative_vario_step);

    static BrushReference negativeBrush[negative_brick_count];
    static PixelRect negativeBricks[negative_brick_count];

    static short startInitCounter = 0;
    static bool dogaugeinit = true;
    static double max_positiveGload;
    static double max_negativeGload;

    if (DoInit[MDI_DRAWVARIO]) {

        const int boxthick = IBLSCALE(BOXTHICK);
        const int hpixelseparate = (LKVarioBar > vBarVarioGR) ? 0 : IBLSCALE(PIXELSEPARATE);
        const int vpixelseparate = IBLSCALE(PIXELSEPARATE);
        const int variowidth = LKVarioSize;

        startInitCounter = 0;
        dogaugeinit = true;

        // initial fullscale G loads for 2D driving.
        // These values are then rescaled (only increased) automatically.
        max_positiveGload = 0.1;
        max_negativeGload = -0.1;


        borderPen = (BgMapColor > POSCOLOR) ? LKPen_White_N0 : LKPen_Black_N0;
        forgroundBrush = (BgMapColor > POSCOLOR) ? LKBrush_White : LKBrush_Black;
        
        greenBrush = LKBrush_Green;
        darkyellowBrush = LKBrush_DarkYellow2;
        orangeBrush = LKBrush_Orange;
        redBrush = LKBrush_Red;
        lakeBrush = LKBrush_Lake;
        blueBrush = LKBrush_Blue;
        indigoBrush = LKBrush_Indigo;


        const short lkvariobar = (LKVarioBar > vBarVarioGR) ? LKVarioBar - vBarVarioGR : LKVarioBar;
        switch (lkvariobar) {
            default:
                LKASSERT(false); // wrong config value or disabled, in any case, it's BUG
                // no break; for avoid to have unitialized Brush array.
            case vBarVarioColor:
                // set default background in case of missing values
                std::fill(std::begin(positiveBrush), std::end(positiveBrush), forgroundBrush);
                std::fill(std::begin(negativeBrush), std::end(negativeBrush), forgroundBrush);
                
                static_assert(std::size(positiveBrush)> 15, "\"positiveBrush\" size must be greater than 15, check \"positive_vario_step\" size");
                
                positiveBrush[15] = redBrush;
                positiveBrush[14] = redBrush;
                positiveBrush[13] = redBrush;
                positiveBrush[12] = redBrush;
                positiveBrush[11] = orangeBrush;
                positiveBrush[10] = orangeBrush;
                positiveBrush[9] = orangeBrush;
                positiveBrush[8] = orangeBrush;
                positiveBrush[7] = darkyellowBrush;
                positiveBrush[6] = darkyellowBrush;
                positiveBrush[5] = darkyellowBrush;
                positiveBrush[4] = darkyellowBrush;
                positiveBrush[3] = greenBrush;
                positiveBrush[2] = greenBrush;
                positiveBrush[1] = greenBrush;
                positiveBrush[0] = greenBrush;

                negativeBrush[0] = lakeBrush;
                negativeBrush[1] = lakeBrush;
                negativeBrush[2] = lakeBrush;
                negativeBrush[3] = lakeBrush;
                negativeBrush[4] = blueBrush;
                negativeBrush[5] = blueBrush;
                negativeBrush[6] = blueBrush;
                negativeBrush[7] = blueBrush;
                negativeBrush[8] = indigoBrush;
                negativeBrush[9] = indigoBrush;
                negativeBrush[10] = indigoBrush;
                negativeBrush[11] = indigoBrush;
                negativeBrush[12] = forgroundBrush;
                negativeBrush[13] = forgroundBrush;
                negativeBrush[14] = forgroundBrush;
                negativeBrush[15] = forgroundBrush;

                static_assert(std::size(negativeBrush)> 15, "\"negativeBrush\" size must be greater than 15, check \"negative_vario_step\" size");

                break;
            case vBarVarioMono:
                std::fill(std::begin(positiveBrush), std::end(positiveBrush), forgroundBrush);
                std::fill(std::begin(negativeBrush), std::end(negativeBrush), forgroundBrush);
                break;
            case vBarVarioRB:
                std::fill(std::begin(positiveBrush), std::end(positiveBrush), redBrush);
                std::fill(std::begin(negativeBrush), std::end(negativeBrush), blueBrush);
                break;
            case vBarVarioGR:
                std::fill(std::begin(positiveBrush), std::end(positiveBrush), greenBrush);
                std::fill(std::begin(negativeBrush), std::end(negativeBrush), redBrush);
                break;
        }

        // vario paint area
        vrc.left = rc.left;
        vrc.top = rc.top;
        vrc.right = vrc.left + variowidth;
        vrc.bottom = rc.bottom - BottomSize;

        // meter area
        mrc.left = vrc.left + hpixelseparate;
        mrc.top = vrc.top + vpixelseparate;
        mrc.right = vrc.right - hpixelseparate;
        mrc.bottom = vrc.bottom - vpixelseparate;

        // half vario separator for positive and negative values
        const double vmiddle_height = NIBLSCALE(2) - ((mrc.bottom - mrc.top) % 2);
        const double vmiddle = ((mrc.bottom - mrc.top) / 2.0) + mrc.top;

        hrc.top = vrc.top + vmiddle - (vmiddle_height / 2);
        hrc.bottom = vrc.top + vmiddle + (vmiddle_height / 2);
        hrc.left = vrc.left;
        hrc.right = vrc.right;

        // half top meter area
        htrc.left = mrc.left;
        htrc.right = mrc.right;
        htrc.bottom = hrc.top - vpixelseparate;
        htrc.top = mrc.top + vpixelseparate;

        // half bottom meter area
        hbrc.left = mrc.left;
        hbrc.right = mrc.right;
        hbrc.top = hrc.bottom + vpixelseparate;
        hbrc.bottom = mrc.bottom - vpixelseparate;

        // pixel height of each positive brick
        const int positive_brick_size = (htrc.bottom - htrc.top - (boxthick * (positive_brick_count - 1))) / positive_brick_count;
        const int positive_brick_advance = positive_brick_size + boxthick;

        // Pre-calculate brick positions for half top
        for (unsigned i = 0; i < positive_brick_count; ++i) {
            RECT& brc = positiveBricks[i];
            brc.left = htrc.left;
            brc.right = htrc.right - NIBLSCALE(4);
            brc.bottom = htrc.bottom - (i * positive_brick_advance);
            brc.top = brc.bottom - positive_brick_size;
        }
        // update last box for hide rounding artefact 
        positiveBricks[positive_brick_count - 1].top = htrc.top;

        // pixel height of each negative brick
        const int negative_brick_size = (hbrc.bottom - hbrc.top - (boxthick * (negative_brick_count - 1))) / negative_brick_count;
        const int negative_brick_advance = negative_brick_size + boxthick;

        // Pre-calculate brick positions for half bottom
        for (unsigned i = 0; i < negative_brick_count; ++i) {
            RECT& brc = negativeBricks[i];
            brc.left = hbrc.left;
            brc.right = hbrc.right - NIBLSCALE(4);
            brc.top = hbrc.top + (i * negative_brick_advance);
            brc.bottom = brc.top + negative_brick_size;
        }
        // update last box for hide rounding artefact 
        negativeBricks[negative_brick_count - 1].bottom = hbrc.bottom;

        DoInit[MDI_DRAWVARIO] = false;
    } // END of INIT

    double vario_value = 0; // can be vario, vario netto or STF offset, depending of config and map mode
    double mc_value = 0; // current MacCready value, used only for Vario or VarioNetto.

    if (ISCAR && DrawInfo.Speed > 0) {
        // Heading is setting Gload, but Heading is not calculated while steady!
        // For this case, we force vario_value to 0.
        //
        // Since we use currently a scale 0-6 for vario, we can use 0-2 for cars.
        // This accounts for an acceleration topscale of 0-100kmh in 13.9 seconds.
        // Not a big acceleration, but very good for normal car usage.
        // We make this concept dynamical, and different for positive and negative accelerations.
        // Because negative accelerations are much higher, on a car. Of course!
        //
        if (DerivedDrawInfo.Gload > 0) {
            if (DerivedDrawInfo.Gload > max_positiveGload) {
                max_positiveGload = DerivedDrawInfo.Gload;
                StartupStore(_T("..... NEW MAXPOSITIVE G=%f\n"), max_positiveGload);
            }
            LKASSERT(max_positiveGload > 0);
            vario_value = (DerivedDrawInfo.Gload / max_positiveGload)*6;
            //StartupStore(_T("Speed=%f G=%f max=%f val=%f\n"),DrawInfo.Speed, DerivedDrawInfo.Gload, max_positiveGload,vario_value);
        }
        if (DerivedDrawInfo.Gload < 0) {
            if (DerivedDrawInfo.Gload < max_negativeGload) {
                max_negativeGload = DerivedDrawInfo.Gload;
                StartupStore(_T("..... NEW MAXNEGATIVE G=%f\n"), max_negativeGload);
            }
            LKASSERT(max_negativeGload < 0);
            vario_value = (DerivedDrawInfo.Gload / max_negativeGload)*-6;
            //StartupStore(_T("Speed=%f G=%f max=%f val=%f\n"),DrawInfo.Speed, DerivedDrawInfo.Gload, max_negativeGload,vario_value);
        }

    } else if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) || LKVarioVal == vValVarioVario) {
        if (VarioAvailable(DrawInfo)) {
            // UHM. I think we are not painting values correctly for knots &c.
            //vario_value = LIFTMODIFY*DrawInfo.Vario;
            vario_value = DrawInfo.Vario;
        } else {
            vario_value = DerivedDrawInfo.Vario;
        }
        mc_value = MACCREADY;
    } else {
        switch (LKVarioVal) {
            default:
            case vValVarioNetto:
                vario_value = DerivedDrawInfo.NettoVario;
                // simple hack for avoid to used polar curve : glider_sink_rate = Vario - NettoVario;
                mc_value = MACCREADY + (DerivedDrawInfo.Vario - DerivedDrawInfo.NettoVario);
                break;
            case vValVarioSoll:
                double ias;
                if (DrawInfo.AirspeedAvailable && VarioAvailable(DrawInfo))
                    ias = DrawInfo.IndicatedAirspeed;
                else
                    ias = DerivedDrawInfo.IndicatedAirspeedEstimated;

                // m/s 0-nnn autolimit to 20m/s full scale (72kmh diff)
                vario_value = Clamp(DerivedDrawInfo.VOpt - ias, -20., 20.);
                vario_value /= 3.3333; // 0-20  -> 0-6
                vario_value *= -1; // if up, push down
                break;
        }
    }


    // Backup selected Brush & Pen
    LKSurface::OldPen oldPen = Surface.SelectObject(LK_NULL_PEN);
    LKSurface::OldBrush oldBrush = Surface.SelectObject(LKBrush_Hollow);
    
    // draw Vario box ( only if not transparent )
    if (LKVarioBar <= vBarVarioGR) {
        Surface.SelectObject(borderPen);
        Surface.SelectObject(hInvBackgroundBrush[BgMapColor]);
        Surface.Rectangle(vrc.left, vrc.top, vrc.right, vrc.bottom);
    }
    // draw middle separator for 0 scale indicator
    Surface.FillRect(&hrc, forgroundBrush);

    Surface.SelectObject(borderPen);
    if (dogaugeinit) {

        // this is causing problems on emulators and condor and most of the times when the gps has no valid date
        // so we don't use seconds, but loop counter
        if (startInitCounter++ > 2) {
            dogaugeinit = false;
        }

        // Demo show all bricks
        for (unsigned i = 0; i < positive_brick_count; ++i) {
            const RECT& brc = positiveBricks[i];
            Surface.SelectObject(positiveBrush[i]);
            Surface.Rectangle(brc.left, brc.top, brc.right, brc.bottom);
        }

        for (unsigned i = 0; i < negative_brick_count; ++i) {
            const RECT& brc = negativeBricks[i];
            Surface.SelectObject(negativeBrush[i]);
            Surface.Rectangle(brc.left, brc.top, brc.right, brc.bottom);
        }

    } else {
        // Draw Real Vario Data

        // Draw Positive Brick 
        for (unsigned i = 0; i < positive_brick_count && vario_value >= positive_vario_step[i]; ++i) {
            const RECT& brc = positiveBricks[i];
            Surface.SelectObject(positiveBrush[i]);
            Surface.Rectangle(brc.left, brc.top, brc.right, brc.bottom);
        }

        // Draw Negative Brick 
        for (unsigned i = 0; i < negative_brick_count && vario_value <= negative_vario_step[i]; ++i) {
            const PixelRect& brc = negativeBricks[i];
            Surface.SelectObject(negativeBrush[i]);
            Surface.Rectangle(brc.left, brc.top, brc.right, brc.bottom);
        }

        // Draw MacCready Indicator
        const auto step_iterator = std::upper_bound(std::begin(positive_vario_step), std::end(positive_vario_step), mc_value);
        const size_t mc_brick_idx = std::distance(std::begin(positive_vario_step), step_iterator);
        if (mc_brick_idx > 1 && mc_brick_idx < std::size(positiveBricks)) {
            const PixelRect& brc_next = positiveBricks[mc_brick_idx];
            const PixelRect& brc_Prev = positiveBricks[mc_brick_idx-1];
            
            const PixelSize IconSize = hMcVario.GetSize();
            const PixelSize DrawSize = {
                vrc.GetSize().cx,
                IconSize.cy * vrc.GetSize().cx / IconSize.cx
            };
            const RasterPoint DrawPos = {
                vrc.left,
                brc_Prev.top + ((brc_next.bottom - brc_Prev.top) / 2) + (IconSize.cy / 2)
            };
            hMcVario.Draw(Surface, DrawPos.x, DrawPos.y, DrawSize.cx, DrawSize.cy);
        }
    }
    // cleanup
    Surface.SelectObject(oldPen);
    Surface.SelectObject(oldBrush);
}
