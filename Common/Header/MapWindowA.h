 // the folowing code is included inside MapWindow class

 // declaration of interface for alpha blended drawing
 public:

 // declaration of interface for alpha blended air space drawing
 public:

  typedef enum
  {
    asp_fill_border_only        = 0,  // airspace drawing using no filling
#ifdef HAVE_HATCHED_BRUSH
    asp_fill_patterns_full      = 1,  // airspace drawing using patterns
    asp_fill_patterns_borders   = 2,  // airspace drawing using patterns, borders
    asp_fill_ablend_full        = 3,  // airspace drawing using alpha blend
    asp_fill_ablend_borders     = 4,  // airspace drawing using alpha blend, borders
#else
    asp_fill_ablend_full        = 1,  // airspace drawing using alpha blend
    asp_fill_ablend_borders     = 2,  // airspace drawing using alpha blend, borders
#endif
  } EAirspaceFillType;

  // set airspace drawing type
  static void SetAirSpaceFillType(EAirspaceFillType fillType) {
    AirspaceFillType = fillType;
  }

  // get airspace drawing type
  static EAirspaceFillType GetAirSpaceFillType(void) {
    return AirspaceFillType;
  }

  // set alpha blended airspace opacity (0..100)
  static void SetAirSpaceOpacity(int opacity) {
    if(AirspaceOpacity != opacity) {
      AirspaceOpacity = opacity;
#ifdef ENABLE_OPENGL
      InitAirSpaceSldBrushes(Colours);
#endif
    }
  }

  // get alpha blended airspace opacity
  static int GetAirSpaceOpacity(void) {
    return AirspaceOpacity;
  }

  static const LKBrush& GetAirspaceSldBrush(int i) {
    return hAirSpaceSldBrushes[i];
  }

  static const LKBrush& GetAirSpaceSldBrushByClass(int i) {
    return GetAirspaceSldBrush(iAirspaceColour[i]);
  }


 private:

  // airspace drawing type
  static EAirspaceFillType AirspaceFillType;

  // alpha blended airspace opacity (0..100)
  static BYTE AirspaceOpacity;

  // draw airspace using alpha blending
  static void ClearTptAirSpace(LKSurface& Surface, const RECT& rc);

  // draw airspace using alpha blending
  static void DrawTptAirSpace(LKSurface& Surface, const RECT& rc);
