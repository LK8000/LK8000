/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   MathFunctions.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 23 octobre 2014
 */

#ifndef MATHFUNCTIONS_H
#define	MATHFUNCTIONS_H

#include "Math/Util.hpp" // for iround and uround

void rotate(double &xin, double &yin, const double &angle);
void rotatescale(double &xin, double &yin, const double &angle, const double &scale);

void frotate(float &xin, float &yin, const float &angle);
void frotatescale(float &xin, float &yin, const float &angle, const float &scale);

void irotate(int &xin, int &yin, const double &angle);
void irotatescale(int &xin, int &yin, const double &angle, const double &scale, double &x, double &y);

void protateshift(POINT &pin, const double &angle, const int &x, const int &y);

double AngleLimit360(double theta);
double AngleLimit180(double theta);

double Reciprocal(double InBound);
double AngleDifference(double angle1, double angle0);
bool AngleInRange(double Angle0, double Angle1, double x, bool is_signed=false);

double HalfAngle(double Angle0, double Angle1);
double BiSector(double InBound, double OutBound);

double ScreenAngle(int x1, int y1, int x2, int y2);

// Fast trig functions
void InitSineTable(void);

#define DEG_TO_INT(x) (static_cast<int>((x)*(65536.0/360.0))>>4) & 0x0FFF
#define DEG_TO_INT_COS(x) ((static_cast<int>((x)*(65536.0/360.0))>>4)+1024) & 0x0FFF

extern double SINETABLE[4096];
extern double INVCOSINETABLE[4096];
extern short ISINETABLE[4096];

#define invfastcosine(x) INVCOSINETABLE[DEG_TO_INT(x)]
#define ifastcosine(x) ISINETABLE[DEG_TO_INT_COS(x)]
#define ifastsine(x) ISINETABLE[DEG_TO_INT(x)]
#define fastcosine(x) SINETABLE[DEG_TO_INT_COS(x)]
#define fastsine(x) SINETABLE[DEG_TO_INT(x)]


#if defined(__i386__) || defined(__x86_64__) || defined(__ARM_FP)
  /* x86 FPUs are extremely fast */
inline
unsigned int isqrt4(unsigned long val) {
  return (unsigned)sqrtf((float)val);
}
#else
unsigned int isqrt4(unsigned long val);
#endif

int  roundupdivision(int a, int b);

double LowPassFilter(double y_last, double x_in, double fact);

#undef MulDiv
template<typename T>
inline T _MulDiv(T nNumber, T nNumerator, T nDenominator) {
    T res = nNumber;
    res *= nNumerator;
    res /= nDenominator;
    return res;
}

template<>
inline int _MulDiv<int>(int nNumber, int nNumerator, int nDenominator) {
    int64_t res = nNumber;
    res *= nNumerator;
    res /= nDenominator;
    return res;
}

template<>
inline short _MulDiv<short>(short nNumber, short nNumerator, short nDenominator) {
    int res = nNumber;
    res *= nNumerator;
    res /= nDenominator;
    return res;
}

inline unsigned int CombinedDivAndMod(unsigned int &lx) {
  unsigned int ox = lx & 0xff;
  // JMW no need to check max since overflow will result in
  // beyond max dimensions
  lx = lx>>8;
  return ox;
}


#endif	/* MATHFUNCTIONS_H */
