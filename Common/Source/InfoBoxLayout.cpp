/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

  $Id: InfoBoxLayout.cpp,v 8.4 2010/12/11 00:28:41 root Exp root $
*/
#include "StdAfx.h"
#include "Sizes.h"
#include "MapWindow.h"
#include "InfoBoxLayout.h"
#include "Dialogs.h"
#include "Utils.h"
#include "externs.h"

#if USEIBOX
#include "InfoBox.h"
#endif

#include "lk8000.h"
#include "utils/heapcheck.h"
using std::min;
using std::max;

#if USEIBOX
extern InfoBox *InfoBoxes[MAXINFOWINDOWS];
extern HWND hWndMainWindow; // Main Windows
extern HINSTANCE hInst; // The current instance

int InfoBoxLayout::InfoBoxGeometry = 0;
int InfoBoxLayout::ControlWidth;
int InfoBoxLayout::ControlHeight;
int InfoBoxLayout::TitleHeight;

int InfoBoxLayout::scale = 1;
double InfoBoxLayout::dscale=1.0;
bool InfoBoxLayout::IntScaleFlag=false;

bool gnav = false;
bool geometrychanged = false;

bool InfoBoxLayout::landscape = false;
bool InfoBoxLayout::fullscreen = false;
bool InfoBoxLayout::square = false;

void InfoBoxLayout::GetInfoBoxPosition(int i, RECT rc, 
				       int *x, int *y,
				       int *sizex, int *sizey) {
  TCHAR reggeompx[50];
  TCHAR reggeompy[50];
  TCHAR reggeomsx[50];
  TCHAR reggeomsy[50];
  DWORD Temp=0;

  wsprintf(reggeompx, TEXT("InfoBoxPositionPosX%d"), i);
  wsprintf(reggeompy, TEXT("InfoBoxPositionPosY%d"), i);
  wsprintf(reggeomsx, TEXT("InfoBoxPositionSizeX%d"), i);
  wsprintf(reggeomsy, TEXT("InfoBoxPositionSizeY%d"), i);

  GetFromRegistry(reggeompx,&Temp); *x = Temp;
  GetFromRegistry(reggeompy,&Temp); *y = Temp;
  GetFromRegistry(reggeomsx,&Temp); *sizex = Temp;
  GetFromRegistry(reggeomsy,&Temp); *sizey = Temp;

  if (*sizey != ControlHeight) {
    geometrychanged = true;
  }
  if (*sizex != ControlWidth) {
    geometrychanged = true;
  }

  if ((*sizex==0)||(*sizey==0)||geometrychanged) {
    // not defined in registry so go with defaults
    // these will be saved back to registry

    switch (InfoBoxGeometry) {
    case 0:
      if (i<numInfoWindows/2) {
	*x = i*ControlWidth;
	*y = rc.top;
      } else {
	*x = (i-numInfoWindows/2)*ControlWidth;
	*y = rc.bottom-ControlHeight;
      }
      break;
    case 1:
      if (i<numInfoWindows/2) {
	*x = i*ControlWidth;
	*y = rc.bottom-ControlHeight*2;
      } else {
	*x = (i-numInfoWindows/2)*ControlWidth;
	*y = rc.bottom-ControlHeight;
      }
      break;
    case 2:
      if (i<numInfoWindows/2) {
	*x = i*ControlWidth;
	*y = rc.top;;
      } else {
	*x = (i-numInfoWindows/2)*ControlWidth;
	*y = rc.top+ControlHeight;
      }
      break;
      
    case 3:
      if (i<numInfoWindows/2) {
	*x = rc.left;
	*y = rc.top+ControlHeight*i;
      } else {
	*x = rc.right-ControlWidth;
	*y = rc.top+ControlHeight*(i-numInfoWindows/2);
      }
      break;
    case 4:
      if (i<numInfoWindows/2) {
	*x = rc.left;
	*y = rc.top+ControlHeight*i;
      } else {
	*x = rc.left+ControlWidth;
	*y = rc.top+ControlHeight*(i-numInfoWindows/2);
      }
      break;
    case 5:
      if (i<numInfoWindows/2) {
	*x = rc.right-ControlWidth*2;
	*y = rc.top+ControlHeight*i;
      } else {
	*x = rc.right-ControlWidth;
	*y = rc.top+ControlHeight*(i-numInfoWindows/2);
      }
      break;
    case 6:
      if (i<3) {
	*x = rc.right-ControlWidth*2;
	*y = rc.top+ControlHeight*i;
      } else {
	if (i<6) {
	  *x = rc.right-ControlWidth*2;
	  *y = rc.top+ControlHeight*(i-3)+ControlHeight*3;
	} else {
	  *x = rc.right-ControlWidth;
	  *y = rc.top+ControlHeight*(i-6)+ControlHeight*3;
	}
      }
      break;
    case 7:
      *x = rc.right-ControlWidth;
      *y = rc.top+ControlHeight*i;
      break;
    };

    *sizex = ControlWidth;
    *sizey = ControlHeight;

    SetToRegistry(reggeompx,*x);
    SetToRegistry(reggeompy,*y);
    SetToRegistry(reggeomsx,*sizex);
    SetToRegistry(reggeomsy,*sizey);

  };
}

void InfoBoxLayout::ScreenGeometry(RECT rc) {

  TCHAR szRegistryInfoBoxGeometry[]=  TEXT("InfoBoxGeometry");
  TCHAR szRegistryInfoBoxGeom[]=  TEXT("AppInfoBoxGeom"); 

  DWORD Temp=0;
  GetFromRegistry(szRegistryInfoBoxGeometry,&Temp);
  InfoBoxGeometry = Temp;

// VENTA-ADDON GEOM
  GetFromRegistry(szRegistryInfoBoxGeom,&Temp);
  if ((unsigned)InfoBoxGeometry != Temp) {
    StartupStore(_T(". Geometry was changed in config, applying%s"),NEWLINE);
    InfoBoxGeometry=Temp;
  }

  // JMW testing only
  geometrychanged = true;

  int maxsize=0;
  int minsize=0;
  maxsize = max(rc.right-rc.left,rc.bottom-rc.top);
  minsize = min(rc.right-rc.left,rc.bottom-rc.top);

  dscale = max(1.0,minsize/240.0); // always start w/ shortest dimension

  if (maxsize == minsize)  // square should be shrunk
  {
    dscale *= 240.0 / 320.0;  
  }

  scale = (int)dscale;
  if ( ((double)scale) == dscale)
    IntScaleFlag=true;
  else
    IntScaleFlag=false;

  #if (WINDOWSPC>0)
  if (maxsize==720) {
	scale=2; // force rescaling with Stretch
  }
  #endif

  int i;
  if (IntScaleFlag) {
  	for (i=0; i<=MAXIBLSCALE; i++) LKIBLSCALE[i]=(int)(i*scale);
  } else {
  	for (i=0; i<=MAXIBLSCALE;i++) LKIBLSCALE[i]=(int)(i*dscale);
  }

  if (rc.bottom<rc.right) {
    // landscape mode
    landscape = true;
    if (InfoBoxGeometry<4) {
      geometrychanged = true;

      // JMW testing
      // VENTA 090814 warning, we are forcing geom 6 for landscape mode if geom previously <4
      if (1) {
	// we set default to 0, top+bottom in portrait, and in this case it becomes 7 for landscape (5 in the right)
	InfoBoxGeometry = 7; // 091105
      } else {
	InfoBoxGeometry+= 3;
      }
    }

  } else if (rc.bottom==rc.right) {
    landscape = false;
    square = true;
    if (InfoBoxGeometry<7) {
      geometrychanged = true;
    }
    InfoBoxGeometry = 7;
  } else {
    landscape = false;
    // portrait mode

    gnav = false;
    if (InfoBoxGeometry>=3) {
      InfoBoxGeometry= 0;

      geometrychanged = true;
    }
  }

  SetToRegistry(szRegistryInfoBoxGeometry,InfoBoxGeometry);

  // JMW testing
  if (InfoBoxGeometry==6) {
    gnav = true;
  }

  if (gnav) {
    numInfoWindows = 9;
  } else if (square) {
    numInfoWindows = 5;
  } else {
    numInfoWindows = 8;
  }
}


void InfoBoxLayout::GetInfoBoxSizes(RECT rc) {

  switch (InfoBoxGeometry) {
  case 0: // portrait
    // calculate control dimensions
    ControlWidth = 2*(rc.right - rc.left) / numInfoWindows;
    ControlHeight = (int)((rc.bottom - rc.top) / CONTROLHEIGHTRATIO);
    TitleHeight = (int)(ControlHeight/TITLEHEIGHTRATIO); 
    
    // calculate small map screen size
    
    MapWindow::MapRect.top = rc.top+ControlHeight;
    MapWindow::MapRect.left = rc.left;
    MapWindow::MapRect.bottom = rc.bottom-ControlHeight;
    MapWindow::MapRect.right = rc.right;
    break;

  case 1: // not used
    // calculate control dimensions
    
    ControlWidth = 2*(rc.right - rc.left) / numInfoWindows;
    ControlHeight = (int)((rc.bottom - rc.top) / CONTROLHEIGHTRATIO);
    TitleHeight = (int)(ControlHeight/TITLEHEIGHTRATIO); 
    
    // calculate small map screen size
    
    MapWindow::MapRect.top = rc.top;
    MapWindow::MapRect.left = rc.left;
    MapWindow::MapRect.bottom = rc.bottom-ControlHeight*2;
    MapWindow::MapRect.right = rc.right;
    break;

  case 2: // not used
    // calculate control dimensions
    
    ControlWidth = 2*(rc.right - rc.left) / numInfoWindows;
    ControlHeight = (int)((rc.bottom - rc.top) / CONTROLHEIGHTRATIO);
    TitleHeight = (int)(ControlHeight/TITLEHEIGHTRATIO); 
    
    // calculate small map screen size
    
    MapWindow::MapRect.top = rc.top+ControlHeight*2;
    MapWindow::MapRect.left = rc.left;
    MapWindow::MapRect.bottom = rc.bottom;
    MapWindow::MapRect.right = rc.right;
    break;

  case 3: // not used
    // calculate control dimensions
    
    ControlWidth = (int)((rc.right - rc.left) / CONTROLHEIGHTRATIO*1.3);
    ControlHeight = (int)(2*(rc.bottom - rc.top) / numInfoWindows);
    TitleHeight = (int)(ControlHeight/TITLEHEIGHTRATIO); 
    
    // calculate small map screen size
    
    MapWindow::MapRect.top = rc.top;
    MapWindow::MapRect.left = rc.left+ControlWidth;
    MapWindow::MapRect.bottom = rc.bottom;
    MapWindow::MapRect.right = rc.right-ControlWidth;
    break;

  case 4:
    // calculate control dimensions
    
    ControlWidth = (int)((rc.right - rc.left) / CONTROLHEIGHTRATIO*1.3);
    ControlHeight = (int)(2*(rc.bottom - rc.top) / numInfoWindows);
    TitleHeight = (int)(ControlHeight/TITLEHEIGHTRATIO); 
    
    // calculate small map screen size
    
    MapWindow::MapRect.top = rc.top;
    MapWindow::MapRect.left = rc.left+ControlWidth*2;
    MapWindow::MapRect.bottom = rc.bottom;
    MapWindow::MapRect.right = rc.right;
    break;

  case 5: // not used
    // calculate control dimensions
    
    ControlWidth = (int)((rc.right - rc.left) / CONTROLHEIGHTRATIO*1.3);
    ControlHeight = (int)(2*(rc.bottom - rc.top) / numInfoWindows);
    TitleHeight = (int)(ControlHeight/TITLEHEIGHTRATIO); 
    
    // calculate small map screen size
    
    MapWindow::MapRect.top = rc.top;
    MapWindow::MapRect.left = rc.left;
    MapWindow::MapRect.bottom = rc.bottom;
    MapWindow::MapRect.right = rc.right-ControlWidth*2;
    break;

  case 6: // landscape
    // calculate control dimensions
    
    ControlHeight = (int)((rc.bottom - rc.top)/6);
    ControlWidth=(int)(ControlHeight*1.44); // preserve relative shape
    TitleHeight = (int)(ControlHeight/TITLEHEIGHTRATIO); 
    
    // calculate small map screen size
    
    MapWindow::MapRect.top = rc.top;
    MapWindow::MapRect.left = rc.left;
    MapWindow::MapRect.bottom = rc.bottom;
    MapWindow::MapRect.right = rc.right-ControlWidth*2;

    break;

  case 7: // square
    // calculate control dimensions
    
    ControlWidth = (int)((rc.right - rc.left)*0.2);
    ControlHeight = (int)((rc.bottom - rc.top)/5);
    TitleHeight = (int)(ControlHeight/TITLEHEIGHTRATIO); 
    
    // calculate small map screen size
    
    MapWindow::MapRect.top = rc.top;
    MapWindow::MapRect.left = rc.left;
    MapWindow::MapRect.bottom = rc.bottom;
    MapWindow::MapRect.right = rc.right-ControlWidth;

    break;
  };

}

void InfoBoxLayout::Paint(void) {
  int i;
  for (i=0; i<numInfoWindows; i++) 
    InfoBoxes[i]->Paint();

  if (!fullscreen) {
    InfoBoxes[numInfoWindows]->SetVisible(false);
    for (i=0; i<numInfoWindows; i++) 
      InfoBoxes[i]->PaintFast();
  } else {
    InfoBoxes[numInfoWindows]->SetVisible(true);
    for (i=0; i<numInfoWindows; i++) {
      int x, y;
      int rx, ry;
      int rw;
      int rh;
      double fw, fh;
      if (landscape) {
        rw = 84;
        rh = 68;
      } else {
        rw = 120;
        rh = 80;
      }
      fw = rw/(double)ControlWidth;
      fh = rh/(double)ControlHeight;
      double f = min(fw, fh);
      rw = (int)(f*ControlWidth);
      rh = (int)(f*ControlHeight);

      if (landscape) {
        rx = i % 3;
        ry = i / 3;

        x = (rw+4)*rx;
        y = (rh+3)*ry;

      } else {
        rx = i % 2;
        ry = i / 4;

        x = (rw)*rx;
        y = (rh)*ry;

      }
      InfoBoxes[i]->PaintInto(InfoBoxes[numInfoWindows]->GetHdcBuf(), 
                              IBLSCALE(x), IBLSCALE(y), IBLSCALE(rw), IBLSCALE(rh));
    }
    InfoBoxes[numInfoWindows]->PaintFast();
  }
}

void InfoBoxLayout::CreateInfoBoxes(RECT rc) {
  int i;
  int xoff, yoff, sizex, sizey;

  GetInfoBoxSizes(rc);

  // JMW created full screen infobox mode
  xoff=0;
  yoff=0;
  sizex=rc.right-rc.left;
  sizey=rc.bottom-rc.top;

  InfoBoxes[numInfoWindows] = new InfoBox(hWndMainWindow, xoff, yoff, sizex, sizey);
  InfoBoxes[numInfoWindows]->SetBorderKind(0);

  // create infobox windows

  for(i=0;i<numInfoWindows;i++)
    {
      GetInfoBoxPosition(i, rc, &xoff, &yoff, &sizex, &sizey);

      InfoBoxes[i] = new InfoBox(hWndMainWindow, xoff, yoff, sizex, sizey);

      int Border=0;
      if (gnav){
        if (i>0)
          Border |= BORDERTOP;
        if (i<6)
          Border |= BORDERRIGHT;
        InfoBoxes[i]->SetBorderKind(Border);
      } else
      if (!landscape) {
        Border = 0;
        if (i<4) {
          Border |= BORDERBOTTOM;
        } else {
          Border |= BORDERTOP;
        }
        Border |= BORDERRIGHT;
        InfoBoxes[i]->SetBorderKind(Border);
      }
    }

}

void InfoBoxLayout::DestroyInfoBoxes(void){
  int i;
  for(i=0; i<numInfoWindows+1; i++){
    delete (InfoBoxes[i]);
  }

}
#endif // USEIBOX

HWND ButtonLabel::hWndButtonWindow[NUMBUTTONLABELS];
bool ButtonLabel::ButtonVisible[NUMBUTTONLABELS];
bool ButtonLabel::ButtonDisabled[NUMBUTTONLABELS];

#if USEIBOX
// was used only to know if landscape mode, we now use ScreenLandscape
int ButtonLabel::ButtonLabelGeometry = 0; 
#endif


void ButtonLabel::GetButtonPosition(int i, RECT rc,
				    int *x, int *y,
				    int *sizex, int *sizey) {

#if USEIBOX
  TCHAR reggeompx[50];
  TCHAR reggeompy[50];
  TCHAR reggeomsx[50];
  TCHAR reggeomsy[50];
  DWORD Temp=0;

  wsprintf(reggeompx, TEXT("ScreenButtonPosX%d"), i);
  wsprintf(reggeompy, TEXT("ScreenButtonPosY%d"), i);
  wsprintf(reggeomsx, TEXT("ScreenButtonSizeX%d"), i);
  wsprintf(reggeomsy, TEXT("ScreenButtonSizeY%d"), i);

  GetFromRegistry(reggeompx,&Temp); *x = Temp;
  GetFromRegistry(reggeompy,&Temp); *y = Temp;
  GetFromRegistry(reggeomsx,&Temp); *sizex = Temp;
  GetFromRegistry(reggeomsy,&Temp); *sizey = Temp;

  if ((*sizex==0)||(*sizey==0)||geometrychanged) {
    // not defined in registry so go with defaults
    // these will be saved back to registry
#else
  if (1) {	// always calculate positions in LK 2.4
#endif

    int hwidth = (rc.right-rc.left)/4;
    int hheight = (rc.bottom-rc.top)/4;

#if USEIBOX
    switch (ButtonLabelGeometry) {
#else
    switch (ScreenLandscape) {
#endif

	case 0: // PORTRAIT MODE ONLY

		if (i==0) {
			*sizex = NIBLSCALE(52);
			*sizey = NIBLSCALE(37);
			*x = rc.left-(*sizex); // JMW make it offscreen for now
			*y = (rc.bottom-(*sizey));
		} else {
			if (i<5) {
				// BOTTOM MENU NAV INFO CONFIG DISPLAY
				//*sizex = NIBLSCALE(52);
				//*sizey = NIBLSCALE(40);
				*sizex = NIBLSCALE(57);
				*sizey = NIBLSCALE(45);
				*x = rc.left+2+hwidth*(i-1);
				*y = (rc.bottom-(*sizey)-NIBLSCALE(2));
			} else {
				*sizex = NIBLSCALE(80);
				*sizey = NIBLSCALE(40);
				*x = rc.right-(*sizex);
				int k = rc.bottom-rc.top-NIBLSCALE(46); 

				*y = (rc.top+(i-5)*k/6+(*sizey/2+NIBLSCALE(3)));
			}
		}
		break;

	case 1: // LANDSCAPE MODE ONLY

		hwidth = (rc.right-rc.left)/5;
		hheight = (rc.bottom-rc.top)/(5);

		int hoffset, voffset;

		// menu buttons 0-4
		// LEFT menu 0 is unused
		if (i==0) {
			*sizex = NIBLSCALE(52);
			*sizey = NIBLSCALE(20);
			*x = rc.left-(*sizex); // JMW make it offscreen for now
			*y = (rc.top);
		} else {
			if (i<5) {
				// RIGHT menu items ( NAV DISPLAY CONFIG INFO ... )
				// new buttons landscape on the right 
				switch(ScreenSize) {
					case ss800x480:
						*sizex = NIBLSCALE(75);
						*sizey = 78;
						voffset= 40;
						// offset distance from right margin
						hoffset=*sizex+5;
						break;
					case ss640x480:
						*sizex = NIBLSCALE(60);
						*sizey = 72-2;
						voffset= 40;
						hoffset=*sizex+3;
						break;
					case ss480x272:
						*sizex = NIBLSCALE(77);
						*sizey = 48-1;
						voffset= 26; 
						hoffset=*sizex+3;
						break;
					case ss480x234:
						*sizex = NIBLSCALE(82);
						*sizey = 41-1;
						voffset= 20;
						hoffset=*sizex+5;
						break;
					case ss320x240:
						*sizex = NIBLSCALE(58);
						*sizey = 34-1;
						voffset= 16;
						hoffset=*sizex+1;
						break;
					case ss400x240:
						*sizex = NIBLSCALE(75);
						*sizey = 39;
						voffset= 20;
						hoffset=*sizex+2;
						break;
#if (WINDOWSPC>0)
					case ss720x408:
						*sizex = NIBLSCALE(77); // watch out MAXIBLSCALE
						*sizey = 72-1;
						voffset= 39;
						hoffset=*sizex+3;
						break;
					case ss896x672:
						*sizex = NIBLSCALE(60);
						*sizey = 100-2;
						voffset= 56;
						hoffset=*sizex+4;
						break;
#endif
					default:
						*sizex = NIBLSCALE(58);
						*sizey = NIBLSCALE(40);
						voffset=0;
						hoffset=*sizex+5;
						break;
				}
				//*x = rc.right-3-*sizex;	
				//*y = (rc.top+hheight*i-(*sizey)/2)-(*sizey/2);
				*x = rc.right-3-hoffset;	
				*y = (rc.top+hheight*i-(*sizey)/2)-voffset;
			} else {
				// BOTTOM MENUS

				// warning, these values are a copy of those in Utils2, inside InitNewMap.
				// Since that function has not been called yet when we are here, we need to load them manually.
				// In any case, even changing those values, only cosmetic issue is rised.
				switch(ScreenSize) {
					case ss800x480:
						*sizex = NIBLSCALE(78);
						*sizey = 80-2;
						hoffset=NIBLSCALE(1);
						// distance from bottom
						voffset=1;
						break;
					case ss640x480:
						*sizex = NIBLSCALE(62);
						*sizey = 72-2;
						hoffset= 2;
						voffset=1;
						break;
					case ss480x272:
						*sizex = NIBLSCALE(82);
						*sizey = 48-1;
						hoffset= 2;
						voffset=1;
						break;
					case ss480x234:
						*sizex = NIBLSCALE(86);
						*sizey = 41-1;
						hoffset= 5;
						voffset=1;
						break;
					case ss320x240:
						*sizex = NIBLSCALE(60);
						*sizey = 34-1;
						hoffset= 2;
						voffset=1;
						break;
					case ss400x240:
						*sizex = NIBLSCALE(78);
						*sizey = 40-1;
						hoffset=NIBLSCALE(1);
						// distance from bottom
						voffset=1;
						break;
#if (WINDOWSPC>0)
					case ss720x408:
						*sizex = NIBLSCALE(82);
						*sizey = 72-1;
						hoffset= 2;
						voffset=1;
						break;
					case ss896x672:
						*sizex = NIBLSCALE(62);
						*sizey = 100-2;
						hoffset= 2;
						voffset=1;
						break;
#endif
					default:
						*sizex = NIBLSCALE(58);
						*sizey = NIBLSCALE(40);
						hoffset=0;
						voffset=1;
						break;
				}
				*x = rc.left+hwidth*(i-5)+ hoffset;
				*y = (rc.bottom-(*sizey))- voffset; 
			}
		}
		break;

	} 

#if USEIBOX
    SetToRegistry(reggeompx,*x);
    SetToRegistry(reggeompy,*y);
    SetToRegistry(reggeomsx,*sizex);
    SetToRegistry(reggeomsy,*sizey);
#endif

  };

}


void ButtonLabel::CreateButtonLabels(RECT rc) {
  int i;
  int x, y, xsize, ysize;

  int buttonWidth = NIBLSCALE(50);
  int buttonHeight = NIBLSCALE(15);

#if USEIBOX
  // set geometry for button correctly
  if (ScreenSize > (ScreenSize_t)sslandscape)
	ButtonLabelGeometry = 1; 
  else
	ButtonLabelGeometry = 0; 
#endif

  for (i=0; i<NUMBUTTONLABELS; i++) {

  #if (WINDOWSPC>0)

  hWndButtonWindow[i] = CreateWindow(
	TEXT("STATIC"), TEXT("\0"),
	WS_CHILD|WS_TABSTOP
	|SS_CENTER|SS_NOTIFY
	|WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER,
	rc.left, rc.top, 
	buttonWidth, buttonHeight, hWndMainWindow, NULL, hInst, NULL);

   #else

   switch(ScreenSize) {
	case ss800x480:
		hWndButtonWindow[i] = CreateWindowEx( WS_EX_OVERLAPPEDWINDOW,
			TEXT("STATIC"), TEXT("\0"),
			WS_CHILD|WS_TABSTOP
			|SS_CENTER|SS_NOTIFY
			|WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER|WS_SIZEBOX,
			rc.left, rc.top, 
			buttonWidth, buttonHeight, hWndMainWindow, NULL, hInst, NULL);
			break;
	default:
		hWndButtonWindow[i] = CreateWindow(
			TEXT("STATIC"), TEXT("\0"),
			WS_CHILD|WS_TABSTOP
			|SS_CENTER|SS_NOTIFY
			|WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER,
			rc.left, rc.top, 
			buttonWidth, buttonHeight, hWndMainWindow, NULL, hInst, NULL);
			break;
    }

    #endif


    GetButtonPosition(i, rc, &x, &y, &xsize, &ysize);

    SetWindowPos(hWndButtonWindow[i],HWND_TOP, x, y, xsize, ysize, SWP_SHOWWINDOW);
    ButtonVisible[i]= true;
    ButtonDisabled[i]= false;

    SetLabelText(i,NULL);
    SetWindowLong(hWndButtonWindow[i], GWL_USERDATA, 4);	  
  }

}

void ButtonLabel::SetFont(HFONT Font) {
  int i;
  for (i=0; i<NUMBUTTONLABELS; i++) {
    SendMessage(hWndButtonWindow[i], WM_SETFONT, (WPARAM)Font, MAKELPARAM(TRUE,0));
  }
}


void ButtonLabel::Destroy() {
  int i;
  for (i=0; i<NUMBUTTONLABELS; i++) {
    DestroyWindow(hWndButtonWindow[i]);

    // prevent setting of button details if it's been destroyed
    hWndButtonWindow[i] = NULL;
    ButtonVisible[i]= false;
    ButtonDisabled[i] = true;
  }
}


void ButtonLabel::SetLabelText(int index, const TCHAR *text) {
  // error! TODO enhancement: Add debugging
  if (index>= NUMBUTTONLABELS) 
    return;

  // don't try to draw if window isn't initialised
  if (hWndButtonWindow[index] == NULL) return;

  if ((text==NULL) || (*text==_T('\0'))||(*text==_T(' '))) {
    ShowWindow(hWndButtonWindow[index], SW_HIDE);
    ButtonVisible[index]= false;
  } else {

    TCHAR s[100];

    bool greyed = ExpandMacros(text, s, sizeof(s)/sizeof(s[0]));

    if (greyed) {
      SetWindowLong(hWndButtonWindow[index], GWL_USERDATA, 5);
      ButtonDisabled[index]= true;
    } else {
      SetWindowLong(hWndButtonWindow[index], GWL_USERDATA, 4);
      ButtonDisabled[index]= false;
    }

    if ((s[0]==_T('\0'))||(s[0]==_T(' '))) {
      ShowWindow(hWndButtonWindow[index], SW_HIDE);
      ButtonVisible[index]= false;
    } else {
      
      SetWindowText(hWndButtonWindow[index], gettext(s));
      
      SetWindowPos(hWndButtonWindow[index], HWND_TOP, 0,0,0,0,
                   SWP_NOMOVE | SWP_NOSIZE);

      ShowWindow(hWndButtonWindow[index], SW_SHOW);
      ButtonVisible[index]= true;
    }
  }

}

#include "InputEvents.h"

bool ButtonLabel::CheckButtonPress(HWND pressedwindow) {
  int i;

  for (i=0; i<NUMBUTTONLABELS; i++) {
    if (hWndButtonWindow[i]== pressedwindow) {
      if (!ButtonDisabled[i]) {
        InputEvents::processButton(i);
        return TRUE;
      } else {
        return FALSE;
      }
      return FALSE;
    }
  }
  return FALSE;
}


