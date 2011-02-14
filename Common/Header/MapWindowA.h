 // the folowing code is included inside MapWindow class
 
 // declaration of interface for alpha blended drawing 
 public:
   
  // check if alpha blending is supported (valid after calling AlphaBlendInit())
  static bool AlphaBlendSupported() {
    return AlphaBlendF != NULL;
  }    
 
 private:
   
  typedef BOOL (WINAPI *TAlphaBlendF)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION); 

  // pointer to AlphaBlend() function (initialized in AlphaBlendInit())
  static TAlphaBlendF AlphaBlendF;
  
  // tries to locate AlphaBlend() function and initializes some data needed for alpha blending;
  // sets pointer to AlphaBlend function (AlphaBlendF) 
  // (returns false when AlphaBlending is not supported)
  static bool AlphaBlendInit();
  
  // release resources used for alpha blending
  static void AlphaBlendDestroy();
  
  // performs AlphaBlend
  static void DoAlphaBlend(HDC dstHdc, const RECT dstRect, HDC srcHdc, const RECT srcRect, BYTE globalOpacity);

 // declaration of interface for alpha blended air space drawing
 public:
  
  enum EAirspaceFillType
  {
    asp_fill_none    = 0,  // airspace drawing using no filling
    asp_fill_pattern = 1,  // airspace drawing using bitmap patterns
    asp_fill_ablend  = 2,  // airspace drawing using alpha blend 
  };
  
  // set airspace drawing type
  static void SetAirSpaceFillType(int fillType) {
    AirspaceFillType = fillType;
  }

  // get airspace drawing type
  static int GetAirSpaceFillType(void) {
    return AirspaceFillType;
  }
  
  // set alpha blended airspace opacity (0..100)
  static void SetAirSpaceOpacity(int opacity) {
    AirspaceOpacity = opacity;
  }

  // get alpha blended airspace opacity
  static int GetAirSpaceOpacity(void) {
    return AirspaceOpacity;    
  }
  
 private:

  // airspace drawing type
  static int AirspaceFillType;
  
  // alpha blended airspace opacity (0..100)
  static BYTE AirspaceOpacity;
 
  // solid brushes for airspace drawing (initialized in InitAirSpaceSldBrushes())
  static HBRUSH hAirSpaceSldBrushes[NUMAIRSPACECOLORS];
 
  // initialize solid color brushes for airspace drawing (initializes hAirSpaceSldBrushes[])
  static void InitAirSpaceSldBrushes(const COLORREF colours[]);

  static HBRUSH GetAirSpaceSldBrushByClass(int i) {
    return hAirSpaceSldBrushes[iAirspaceColour[i]];
  }

  // draw airspace using alpha blending
  static void ClearTptAirSpace(HDC hdc, const RECT rc);
  
  // draw airspace using alpha blending
  static void DrawTptAirSpace(HDC hdc, const RECT rc);
