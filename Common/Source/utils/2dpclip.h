/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   2dpclip.h
 * Author: Bruno de Lacheisserie
 * 
 * Created on 28 octobre 2012
 */

/*
 * Polygon clipping based on paper 
 *      "A New, Fast Method For 2D Polygon Clipping" 
 *        by Patrick-Gilles Maillot
 */

/*
 * TODO : 
 * Line Clippping based on paper :
 *      "An Efficient Line Clipping Algorithm based on Cohen-Sutherland Line Clipping Algorithm" 
 *        by Mohammad Saber Iraji, Ayda Mazandarani and H. Motameni 
 */

/*
 * TODO :
 * remove degenerated edge : ignore turpoint surounded by two point with same code.
 */

#ifndef _2dpclip_h__
#define _2dpclip_h__

namespace LKGeom {

    template<typename polygon>
    class clipper {
        typedef typename polygon::value_type Upoint;
        typedef typename polygon::const_iterator const_iterator;

    public:

        inline clipper(const Upoint& TopLeft, const Upoint& BottomRight) {
            Clip_region[0].y = TopLeft.y - 1; //lower
            Clip_region[0].x = TopLeft.x - 1; //left

            Clip_region[1].y = TopLeft.y - 1; //lower
            Clip_region[1].x = BottomRight.x + 1; //right

            Clip_region[2].y = BottomRight.y + 1; // upper
            Clip_region[2].x = BottomRight.x + 1; // right

            Clip_region[3].y = BottomRight.y + 1; // upper
            Clip_region[3].x = TopLeft.x - 1; // left
        }

    private:
        /* Region Code */
        static const unsigned _INSIDE = 0; // 0000
        static const unsigned _RIGHT = 1; // 0001
        static const unsigned _TOP = 2; // 0010
        static const unsigned _LEFT = 4; // 0100
        static const unsigned _BOTTOM = 8; // 1000

        static const unsigned _TWOBITS = 0x0100; /* A flag to indicate a 2bit code. */

        static const unsigned _NOSEGM = 0; /* The line is rejected */
        static const unsigned _SEGM = 1; /* The line is visible (even partially) */
        static const unsigned _CLIP = 2; /* The line has been clipped */

        Upoint Cp_start; /* The start point of the line */
        unsigned M_code;
        Upoint Cp_end; /* The end point of the line */
        unsigned D_code;

        unsigned N_Code; /* Temp Code Save End point code for next Loop */

        Upoint Cp_Clip; /* Temp Point Used by Cohen-Sutherland Ligne Clipping */

        /* These are two look_up tables */
        /* used in finding the turning point */
        /* in the case 1-2. They should be */
        /* modified with the regions codes. */
        static const int Tcc[16];
        static const int Cra[16];
        /* Tcc is used to compute a correct */
        /* offset, while Cra gives an index in */
        /* the Clip_region array, for the turning */
        /* coordinates. */

        Upoint Clip_region[4]; /* Clipping region coordinates in 
						   * lower-left, lower-right, 
						   * upper-right and upper-left order
						   */

        inline bool IsSamePoint(const Upoint& pt1, const Upoint& pt2) const {
            return (pt1.x == pt2.x) && (pt1.y == pt2.y);
        }

        inline unsigned CP_space_code(const Upoint *point_to_code) const {
            if (point_to_code->x < Clip_region[0].x) {
                if (point_to_code->y > Clip_region[2].y) return (_LEFT | _TOP | _TWOBITS);
                if (point_to_code->y < Clip_region[0].y) return (_LEFT | _BOTTOM | _TWOBITS);
                return (_LEFT);
            }
            if (point_to_code->x > Clip_region[2].x) {
                if (point_to_code->y > Clip_region[2].y) return (_RIGHT | _TOP | _TWOBITS);
                if (point_to_code->y < Clip_region[0].y) return (_RIGHT | _BOTTOM | _TWOBITS);
                return (_RIGHT);
            }
            if (point_to_code->y > Clip_region[2].y) return (_TOP);
            if (point_to_code->y < Clip_region[0].y) return (_BOTTOM);
            return (_INSIDE);
        }

        //Cohen-Sutherland Line clipping
        int Cp_end_clip() {
            unsigned RetCode = _NOSEGM;

            // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
            unsigned CStart = M_code;
            unsigned CEnd = D_code;

            while (true) {
                if (!((CStart | CEnd)&~_TWOBITS)) { // Bitwise OR is 0. Trivially accept and get out of loop
                    RetCode |= _SEGM;
                    break;
                } else if (((CStart & CEnd)&~_TWOBITS)) { // Bitwise AND is not 0. Trivially reject and get out of loop
                    break;
                } else {
                    // failed both tests, so calculate the line segment to clip
                    // from an outside point to an intersection with clip edge

                    // At least one endpoint is outside the clip rectangle; pick it.
                    unsigned outcodeOut = CStart ? CStart : CEnd;


                    // Now find the intersection point;
                    // use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
                    if (outcodeOut & _TOP) { // point is above the clip rectangle
                        LKASSERT((Cp_end.y - Cp_start.y) != 0);
                        Cp_Clip.x = Cp_start.x + (Cp_end.x - Cp_start.x) * (Clip_region[2].y - Cp_start.y) / (Cp_end.y - Cp_start.y);
                        Cp_Clip.y = Clip_region[2].y;
                    } else if (outcodeOut & _BOTTOM) { // point is below the clip rectangle
                        LKASSERT((Cp_end.y - Cp_start.y) != 0);
                        Cp_Clip.x = Cp_start.x + (Cp_end.x - Cp_start.x) * (Clip_region[0].y - Cp_start.y) / (Cp_end.y - Cp_start.y);
                        Cp_Clip.y = Clip_region[0].y;
                    } else if (outcodeOut & _RIGHT) { // point is to the right of clip rectangle
                        LKASSERT((Cp_end.x - Cp_start.x) != 0);
                        Cp_Clip.y = Cp_start.y + (Cp_end.y - Cp_start.y) * (Clip_region[2].x - Cp_start.x) / (Cp_end.x - Cp_start.x);
                        Cp_Clip.x = Clip_region[2].x;
                    } else /*if (outcodeOut & _LEFT)*/ { // point is to the left of clip rectangle
                        LKASSERT((Cp_end.x - Cp_start.x) != 0);
                        Cp_Clip.y = Cp_start.y + (Cp_end.y - Cp_start.y) * (Clip_region[0].x - Cp_start.x) / (Cp_end.x - Cp_start.x);
                        Cp_Clip.x = Clip_region[0].x;
                    }

                    // Now we move outside point to intersection point to clip
                    // and get ready for next pass.
                    if (outcodeOut == CStart) {
                        Cp_start = Cp_Clip;
                        CStart = CP_space_code(&Cp_start);
                    } else {
                        Cp_end = Cp_Clip;
                        CEnd = CP_space_code(&Cp_end);
                    }
                    RetCode |= _CLIP;
                }
            }
            return RetCode;
        }


    public:

        void CP_2D_polygon_clip(const polygon& inPoly, polygon& outPoly) {
            register int j;
            register Upoint *pt_Cp_start = &Cp_start;
            register Upoint *pt_Cp_end = &Cp_end;

            register const Upoint *pt_lastTp = NULL;
            /*
             * Temporary data used in the case of a 2-2 bit.
             */
            Upoint Cp_t_start;
            Upoint Cp_t_end;
            Upoint Cp_A_point;
            unsigned A_code = 0;

            /*
             * Be sure the polygon is closed. and out polygon is empty
             */
            if (!outPoly.empty()) {
                outPoly.clear();
            }

            if (inPoly.empty() || !IsSamePoint(*inPoly.begin(), *inPoly.rbegin())) {
                return;
            }

            /* 
             * Init Output Polygon 
             */
            std::back_insert_iterator<polygon> out(outPoly);

            /*
             * Register First polygon point
             */
            const_iterator in = inPoly.begin();
            *pt_Cp_start = *in;

            /*
             * Next polygons points... We build a vector from the start
             * point to the end point.
             * Clip the line with a standard 2D line clipping method.
             */

            M_code = CP_space_code(pt_Cp_start);
            while (++in != inPoly.end()) {
                *pt_Cp_end = (*in);

                N_Code = D_code = CP_space_code(pt_Cp_end);
                j = Cp_end_clip();
                /*
                 * If the line is visible, then store the computed point(s), and
                 * jump to the basic turning point test.
                 */
                if (j & _SEGM) {
                    if (j & _CLIP) {
                        out = Cp_start;
                    }
                    out = Cp_end;
                    pt_lastTp = NULL;
                } else {
                    /*
                     * Here the line has been rejected... Apply the polygon clipping.
                     * Begin with a 2bit end point.
                     */
                    if (D_code & _TWOBITS) {
                        if (!(D_code & M_code & ~_TWOBITS)) {
                            /*
                             * If the start point is also a 2bit... Need some more information to
                             * make a decision! So do mid-point subdivision.
                             */
                            if (M_code & _TWOBITS) {
                                j = 1;
                                Cp_t_start = *pt_Cp_start;
                                Cp_t_end = *pt_Cp_end;
                                while (j) {
                                    Cp_A_point.x = (Cp_t_start.x + Cp_t_end.x) / 2;
                                    Cp_A_point.y = (Cp_t_start.y + Cp_t_end.y) / 2;
                                    A_code = CP_space_code(&Cp_A_point);
                                    if (A_code & _TWOBITS) {
                                        if (A_code == D_code) {
                                            Cp_t_end = Cp_A_point;
                                        } else if (A_code == M_code) {
                                            Cp_t_start = Cp_A_point;
                                        } else {
                                            j = 0;
                                        }
                                    } else {

                                        if (A_code & D_code) {
                                            A_code = M_code + Tcc[A_code & ~_TWOBITS];
                                        } else {
                                            A_code = D_code + Tcc[A_code & ~_TWOBITS];
                                        }
                                        j = 0;
                                    }
                                }
                            } else {
                                /*
                                 * This is for a 1 bit start point (2bit end point).
                                 */
                                A_code = D_code + Tcc[M_code];
                            }
                            if (pt_lastTp != &(Clip_region[Cra[A_code & ~_TWOBITS]])) {
                                pt_lastTp = &(Clip_region[Cra[A_code & ~_TWOBITS]]);
                                out = *pt_lastTp;
                            }
                        }
                    } else {
                        /*
                         * Here we have a 1bit end point.
                         */
                        if (M_code & _TWOBITS) {
                            if (!(M_code & D_code)) D_code = M_code + Tcc[D_code];
                        } else {
                            D_code |= M_code;
                            if (Tcc[D_code] == 1) D_code |= _TWOBITS;
                        }
                    }
                }
                /*
                 * The basic turning point test...
                 */
                if (D_code & _TWOBITS) {
                    if (pt_lastTp != &(Clip_region[Cra[D_code & ~_TWOBITS]])) {
                        pt_lastTp = &(Clip_region[Cra[D_code & ~_TWOBITS]]);
                        out = *pt_lastTp;
                    }
                }
                /*
                 * Copy the current point as the next starting point.
                 */
                *pt_Cp_start = (*in);
                M_code = N_Code;
            }
            if (!outPoly.empty()) {
                out = *outPoly.begin();
            }
        }
    };

    template<typename Upoint>
            const int clipper<Upoint>::Tcc[16] = {0, -3, -6, 1, 3, 0, 1, 0, 6, 1, 0, 0, 1, 0, 0, 0};
    template<typename Upoint>
            const int clipper<Upoint>::Cra[16] = {-1, -1, -1, 2, -1, -1, 3, -1, -1, 1, -1, -1, 0, -1, -1, -1};

    template<typename polygon, typename Upoint>
    inline void ClipPolygon(const Upoint& TopLeft, const Upoint& BottomRight, const polygon& inPoly, polygon& outPoly) {
        LKGeom::clipper<polygon > (TopLeft, BottomRight).CP_2D_polygon_clip(inPoly, outPoly);
    }
};
#endif // _2dpclip_h__
