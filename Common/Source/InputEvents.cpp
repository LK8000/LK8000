/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

  $Id: InputEvents.cpp,v 8.11 2011/01/01 23:21:36 root Exp root $
*/

#include "externs.h"
#include "InputEvents.h"
#include "LKInterface.h"
#include "Terrain.h"
#include "LKProfiles.h"
#include "InfoBoxLayout.h"
#include "LKProcess.h"
#include "Atmosphere.h"
#include "Waypointparser.h"
#include "Message.h"
#include "AATDistance.h"
#include "DoInits.h"
#include "Logger.h"
#ifdef LXMINIMAP
#include "Modeltype.h"
#endif
#include "utils/stl_utils.h"
#include "RasterTerrain.h"
#include "Multimap.h"
#include "Dialogs.h"
#include "Sideview.h"
#include "TraceThread.h"
#include "CTaskFileHelper.h"
#include "utils/stringext.h"
#include "utils/openzip.h"
#include "Asset.hpp"
#include "Event/Event.h"
#include "MapWindow.h"
#include "Sound/Sound.h"
#include "OS/RotateScreen.h"
#include "Time/PeriodClock.hpp"

// uncomment for show all menu button with id as Label.
//#define TEST_MENU_LAYOUT

// Sensible maximums
#define MAX_MODE 100
#define MAX_MODE_STRING 25
#define MAX_KEY 255
#define MAX_EVENTS 2048  // max number of menu events items
#define MAX_TEXT2EVENTS_COUNT  256
#define MAX_LABEL NUMBUTTONLABELS

extern AATDistance aatdistance;
extern bool ForceRenderMap;

// Current modes - map mode to integer (primitive hash)
static TCHAR mode_current[MAX_MODE_STRING] = TEXT("default");	// Current mode
static TCHAR mode_map[MAX_MODE][MAX_MODE_STRING];		// Map mode to location
static int mode_map_count = 0;

// Key to Event - Keys (per mode) mapped to events
#ifndef USE_GDI
// Some linux keycode are defined on 4 Byte, use map instead of array.
static std::map<int,int> Key2Event[MAX_MODE];
#else
static int Key2Event[MAX_MODE][MAX_KEY];
#endif

// Glide Computer Events
static int GC2Event[MAX_MODE][GCE_COUNT];

// NMEA Triggered Events
static int N2Event[MAX_MODE][NE_COUNT];

// Events - What do you want to DO
typedef struct {
  pt2Event event; // Which function to call (can be any, but should be here)
  const TCHAR *misc;    // Parameters
  int next;       // Next in event list - eg: Macros
} EventSTRUCT;

static EventSTRUCT Events[MAX_EVENTS];	 // the menu events items
static int Events_count=0;				// How many have we defined

// Labels - defined per mode
typedef struct {
  const TCHAR *label;
  int event;
} ModeLabelSTRUCT;

static ModeLabelSTRUCT ModeLabel[MAX_MODE][MAX_LABEL];
std::list<TCHAR*> LabelGarbage;

#define MAX_GCE_QUEUE 10
static int GCE_Queue[MAX_GCE_QUEUE];
#define MAX_NMEA_QUEUE 10
static int NMEA_Queue[MAX_NMEA_QUEUE];


// popup object details event queue data.
typedef struct {
    InputEvents::PopupType type;
    int index;
} PopupEvent_t;
typedef std::deque<PopupEvent_t> PopupEventQueue_t;
PopupEventQueue_t PopupEventQueue;

// -----------------------------------------------------------------------
// Initialisation and Defaults
// -----------------------------------------------------------------------

bool InitONCE = false;
#ifdef LXMINIMAP
unsigned SelectedButtonId=0;
bool IsMenuShown = false;
//bool SelectMode = false;

PeriodClock LastActiveSelectMode;
#endif

// This is set when multimaps are calling MMCONF menu.
bool IsMultimapConfigShown=false;

// Mapping text names of events to the real thing
typedef struct {
  const TCHAR *text;
  pt2Event event;
} Text2EventSTRUCT;
Text2EventSTRUCT Text2Event[MAX_TEXT2EVENTS_COUNT];
size_t Text2Event_count=0;

// Mapping text names of events to the real thing
const TCHAR *Text2GCE[GCE_COUNT+1];

// Mapping text names of events to the real thing
const TCHAR *Text2NE[NE_COUNT+1];

static Mutex  CritSec_EventQueue;

static PeriodClock myPeriodClock;


// Read the data files
void InputEvents::readFile() {
  #if TESTBENCH
  StartupStore(TEXT("... Loading input events file%s"),NEWLINE);
  #endif

  {
    ScopeLock Lock(CritSec_EventQueue);
    // clear the GCE and NMEA queues
    std::fill(std::begin(GCE_Queue), std::end(GCE_Queue), -1);
    std::fill(std::begin(NMEA_Queue), std::end(NMEA_Queue), -1);
  }
  // Get defaults
  if (!InitONCE) {
	#include "InputEvents_LK8000.cpp"
	#include "InputEvents_Text2Event.cpp"
        #ifdef TESTBENCH
        StartupStore(_T("... Loaded %d Text2Event (max is %u)%s"),(unsigned)Text2Event_count,MAX_TEXT2EVENTS_COUNT,NEWLINE);
        #endif
    InitONCE = true;
  }

#ifndef TEST_MENU_LAYOUT
  // Read in user defined configuration file

  TCHAR szFile1[MAX_PATH] = TEXT("\0");
  ZZIP_FILE *fp=NULL;

  //
  // ENGINEERING MODE: SELECTED XCI HAS PRIORITY
  //
  if (_tcslen(szInputFile)>0) {
    LocalPath(szFile1, _T(LKD_CONF), szInputFile);
    fp=openzip(szFile1, "rb");
  }

  const TCHAR * xcifile = nullptr;
  if (fp == NULL) {

    // invalide ENGINEERING MODE: SELECTED XCI, reset config.
    _tcscpy(szInputFile, _T(""));

	// No special XCI in engineering, or nonexistent file.. go ahead with language check
	// Remember: DEFAULT_MENU existance is already been checked upon startup.

	switch(AircraftCategory) {
		case umGlider:
			xcifile = _T("MENU_GLIDER.TXT");
			break;
		case umParaglider:
            xcifile = _T("MENU_PARAGLIDER.TXT");
			break;
		case umCar:
            xcifile = _T("MENU_CAR.TXT");
			break;
		case umGAaircraft:
            xcifile = _T("MENU_GA.TXT");
			break;
		default:
            xcifile = _T("MENU_OTHER.TXT");
			break;
	}

    TCHAR xcifilepath[MAX_PATH];

    SystemPath(xcifilepath,_T(LKD_SYSTEM), xcifile);
	fp=openzip(xcifilepath, "rt");
	if (fp == NULL) {
        SystemPath(xcifilepath,_T(LKD_SYSTEM), _T("DEFAULT_MENU.TXT"));
		fp=openzip(xcifilepath, "rt");
		if (fp == NULL) {
			// This cannot happen
			StartupStore(_T("..... NO DEFAULT MENU <%s>, using internal XCI!\n"),xcifilepath);
			return;
		}
	} else {
		StartupStore(_T(". Loaded menu <%s>\n"),xcifilepath);
	}
  }

  // TODO code - Safer sizes, strings etc - use C++ (can scanf restrict length?)
  TCHAR buffer[2049];	// Buffer for all
  TCHAR key[2049];	// key from scanf
  TCHAR value[2049];	// value from scanf
  TCHAR *new_label = NULL;
  int found = 0;

  // Init first entry
  bool some_data = false;		// Did we fin some in the last loop...
  TCHAR d_mode[1024] = TEXT("");			// Multiple modes (so large string)
  TCHAR d_type[256] = TEXT("");
  TCHAR d_data[256] = TEXT("");
  int event_id = 0;
  TCHAR d_label[256] = TEXT("");
  int d_location = 0;
  TCHAR d_event[256] = TEXT("");
  TCHAR d_misc[256] = TEXT("");

  int line = 0;

  /* Read from the file */
  // TODO code: Note that ^# does not allow # in key - might be required (probably not)
  //		Better way is to separate the check for # and the scanf
  // ! _stscanf works differently on WinPC and WinCE (on WinCE it returns EOF on empty string)

  while (ReadULine(fp, buffer, array_size(buffer)) && (buffer[0] == '\0' ||
	   ((found = _stscanf(buffer, TEXT("%[^#=]=%[^\r\n][\r\n]"), key, value)) != EOF))
  ) {
    line++;

    // if the first line is "#CLEAR" then the whole default config is cleared and can be overwritten by file
    if ((line == 1) && (_tcsstr(buffer, TEXT("#CLEAR")))){
#ifndef USE_GDI
      for(auto Item : Key2Event) Item.clear();
#else
      memset(&Key2Event, 0, sizeof(Key2Event));
#endif
      memset(&GC2Event, 0, sizeof(GC2Event));
      memset(&Events, 0, sizeof(Events));
      memset(&ModeLabel, 0, sizeof(ModeLabel));
      Events_count = 0;
    }

    // Check valid line? If not valid, assume next record (primative, but works ok!)
    if ((buffer[0] == '\r') || (buffer[0] == '\n') || (buffer[0] == '\0')) {
      // General checks before continue...
      if (
	  some_data
	  && (_tcscmp(d_mode, TEXT("")) != 0)		//
	  ) {

	TCHAR *token;

	// For each mode
	token = _tcstok(d_mode, TEXT(" "));

	// General errors - these should be true
	LKASSERT(d_location >= 0);
	LKASSERT(d_location <= MAX_LABEL);
	LKASSERT(event_id >= 0);
	LKASSERT(d_mode != NULL);
	LKASSERT(d_type != NULL);
	LKASSERT(d_label != NULL);

	// These could indicate bad data - thus not an ASSERT (debug only)
	// ASSERT(_tcslen(d_mode) < 1024);
	// ASSERT(_tcslen(d_type) < 1024);
	// ASSERT(_tcslen(d_label) < 1024);

	while( token != NULL ) {

	  // All modes are valid at this point
	  int mode_id = mode2int(token, true);
	  LKASSERT(mode_id >= 0);
          LKASSERT(mode_id < (int)array_size(Key2Event));

	  // Make label event
	  // TODO code: Consider Reuse existing entries...
	  if (d_location > 0) {
	    // Only copy this once per object - save string space
	    if (!new_label) {
	      new_label = StringMallocParse(d_label);
		  LabelGarbage.push_back(new_label);
	    }
	    LKASSERT(new_label!=NULL);
	    InputEvents::makeLabel(mode_id, new_label, d_location, event_id);
	  }

	  // Make key (Keyboard input)
	  if (_tcscmp(d_type, TEXT("key")) == 0)	{	// key - Hardware key or keyboard
	    const int ikey = findKey(d_data);				// Get the int key (eg: APP1 vs 'a')
	    if (ikey > 0) {
#ifdef USE_GDI
            LKASSERT(ikey < (int)array_size(Key2Event[mode_id]));
            Key2Event[mode_id][ikey] = event_id;
#else
            if(event_id > 0) {
                Key2Event[mode_id][ikey] = event_id;
            } else {
                Key2Event[mode_id].erase(ikey);
            }
#endif

        }
	    // Make gce (Glide Computer Event)
	  } else if (_tcscmp(d_type, TEXT("gce")) == 0) {		// GCE - Glide Computer Event
	    int iikey = findGCE(d_data);				// Get the int key (eg: APP1 vs 'a')
	    if (iikey >= 0 && iikey < GCE_COUNT)
	      GC2Event[mode_id][iikey] = event_id;

	    // Make ne (NMEA Event)
	  } else if (_tcscmp(d_type, TEXT("ne")) == 0) { 		// NE - NMEA Event
	    int iiikey = findNE(d_data);			// Get the int key (eg: APP1 vs 'a')
	    if (iiikey >= 0 && iiikey < NE_COUNT)
	      N2Event[mode_id][iiikey] = event_id;
	  } else if (_tcscmp(d_type, TEXT("label")) == 0)	{	// label only - no key associated (label can still be touch screen)
	    // Nothing to do here...

	  }

	  token = _tcstok( NULL, TEXT(" "));
	}

      }

      // Clear all data.
      some_data = false;
      _tcscpy(d_mode, TEXT(""));
      _tcscpy(d_type, TEXT(""));
      _tcscpy(d_data, TEXT(""));
      event_id = 0;
      _tcscpy(d_label, TEXT(""));
      d_location = 0;
      new_label = NULL;

    } else if ((found != 2) || key[0] == '\0' || value[0] == '\0') {
      // Do nothing - we probably just have a comment line
      // JG removed "void;" - causes warning (void is declaration and needs variable)
      // NOTE: Do NOT display buffer to user as it may contain an invalid stirng !

    } else {
      if (_tcscmp(key, TEXT("mode")) == 0) {
	if (_tcslen(value) < 1024) {
	  some_data = true;	// Success, we have a real entry
	  _tcscpy(d_mode, value);
	}
      } else if (_tcscmp(key, TEXT("type")) == 0) {
	if (_tcslen(value) < 256)
	  _tcscpy(d_type, value);
      } else if (_tcscmp(key, TEXT("data")) == 0) {
	if (_tcslen(value) < 256)
	  _tcscpy(d_data, value);
      } else if (_tcscmp(key, TEXT("event")) == 0) {
	if (_tcslen(value) < 256) {
	  _tcscpy(d_event, TEXT(""));
	  _tcscpy(d_misc, TEXT(""));
	  int ef;
	    ef = _stscanf(value, TEXT("%[^ ] %[A-Za-z0-9 \\/().,]"), d_event, d_misc);

	  // TODO code: Can't use token here - breaks
	  // other token - damn C - how about
	  // C++ String class ?

	  // TCHAR *eventtoken;
	  // eventtoken = _tcstok(value, TEXT(" "));
	  // d_event = token;
	  // eventtoken = _tcstok(value, TEXT(" "));

	  if ((ef == 1) || (ef == 2)) {

	    // TODO code: Consider reusing existing identical events

	    pt2Event event = findEvent(d_event);
	    if (event) {
		  TCHAR* szString = StringMallocParse(d_misc);
		  LabelGarbage.push_back(szString);
	      event_id = makeEvent(event, szString, event_id);
	    }
	  }
	}
      } else if (_tcscmp(key, TEXT("label")) == 0) {
        _tcscpy(d_label, value);
      } else if (_tcscmp(key, TEXT("location")) == 0) {
        _stscanf(value, TEXT("%d"), &d_location);

      }
    }

  } // end while
  #ifdef TESTBENCH
  StartupStore(_T("... Loaded %d Menu Events (Max is %d)%s"),Events_count,MAX_EVENTS,NEWLINE);
  #endif
  zzip_fclose(fp);
#endif
}

void InputEvents::UnloadString(){
	memset(&ModeLabel, 0, sizeof(ModeLabel));

	std::for_each(LabelGarbage.begin(),LabelGarbage.end(), safe_free());
	LabelGarbage.clear();
}

int InputEvents::findKey(const TCHAR *data) {

  if (_tcscmp(data, TEXT("APP1")) == 0)
    return KEY_APP1;
  else if (_tcscmp(data, TEXT("APP2")) == 0)
    return KEY_APP2;
  else if (_tcscmp(data, TEXT("APP3")) == 0)
    return KEY_APP3;
  else if (_tcscmp(data, TEXT("APP4")) == 0)
    return KEY_APP4;
  else if (_tcscmp(data, TEXT("APP5")) == 0)
    return KEY_APP5;
  else if (_tcscmp(data, TEXT("APP6")) == 0)
    return KEY_APP6;
  else if (_tcscmp(data, TEXT("F1")) == 0)
    return KEY_F1;
  else if (_tcscmp(data, TEXT("F2")) == 0)
    return KEY_F2;
  else if (_tcscmp(data, TEXT("F3")) == 0)
    return KEY_F3;
  else if (_tcscmp(data, TEXT("F4")) == 0)
    return KEY_F4;
  else if (_tcscmp(data, TEXT("F5")) == 0)
    return KEY_F5;
  else if (_tcscmp(data, TEXT("F6")) == 0)
    return KEY_F6;
  else if (_tcscmp(data, TEXT("F7")) == 0)
    return KEY_F7;
  else if (_tcscmp(data, TEXT("F8")) == 0)
    return KEY_F8;
  else if (_tcscmp(data, TEXT("F9")) == 0)
    return KEY_F9;
  else if (_tcscmp(data, TEXT("F10")) == 0)
    return KEY_F10;
  else if (_tcscmp(data, TEXT("LEFT")) == 0)
    return KEY_LEFT;
  else if (_tcscmp(data, TEXT("RIGHT")) == 0)
    return KEY_RIGHT;
  else if (_tcscmp(data, TEXT("UP")) == 0)
    return KEY_UP;
  else if (_tcscmp(data, TEXT("DOWN")) == 0)
    return KEY_DOWN;
  else if (_tcscmp(data, TEXT("RETURN")) == 0)
    return KEY_RETURN;
  else if (_tcscmp(data, TEXT("ESCAPE")) == 0)
    return KEY_ESCAPE;
#ifdef LXMINIMAP
  else if (_tcscmp(data, TEXT("SPACE")) == 0)
    return KEY_SPACE;
#endif

  else if (_tcslen(data) == 1)
    return _totupper(data[0]);
  else
    return 0;

}

pt2Event InputEvents::findEvent(const TCHAR *data) {
  LKASSERT(Text2Event_count<MAX_TEXT2EVENTS_COUNT);
  for (size_t i = 0; i < Text2Event_count; i++) {
    if (_tcscmp(data, Text2Event[i].text) == 0)
      return Text2Event[i].event;
  }
  return NULL;
}

int InputEvents::findGCE(const TCHAR *data) {
  int i;
  for (i = 0; i < GCE_COUNT; i++) {
    if (_tcscmp(data, Text2GCE[i]) == 0)
      return i;
  }
  return -1;
}

int InputEvents::findNE(const TCHAR *data) {
  int i;
  for (i = 0; i < NE_COUNT; i++) {
    if (_tcscmp(data, Text2NE[i]) == 0)
      return i;
  }
  return -1;
}

// Create EVENT Entry
// NOTE: String must already be copied (allows us to use literals
// without taking up more data - but when loading from file must copy string
int InputEvents::makeEvent(void (*event)(const TCHAR *), const TCHAR *misc, int next) {
  if (Events_count >= MAX_EVENTS){
    LKASSERT(0);
    return 0;
  }
  Events_count++;	// NOTE - Starts at 1 - 0 is a noop
  Events[Events_count].event = event;
  Events[Events_count].misc = misc;
  Events[Events_count].next = next;

  return Events_count;
}


// Make a new label (add to the end each time)
// NOTE: String must already be copied (allows us to use literals
// without taking up more data - but when loading from file must copy string
void InputEvents::makeLabel(int mode_id, const TCHAR* label, unsigned MenuId, int event_id) {

    static_assert(MAX_MODE == array_size(ModeLabel), "wrong array size" );
    static_assert(MAX_LABEL == array_size(ModeLabel[0]), "wrong array size" );

    unsigned LabelIdx = MenuId -1;

    if ((mode_id >= 0) && (mode_id < MAX_MODE) && (LabelIdx < MAX_LABEL)) {

        if(ModeLabel[mode_id][LabelIdx].label != nullptr && ModeLabel[mode_id][LabelIdx].label[0] != _T('\0')){
            StartupStore(_T("Menu \"%s\" override Menu \"%s\"" NEWLINE), label, ModeLabel[mode_id][LabelIdx].label);
            BUGSTOP_LKASSERT(false);
        }

        ModeLabel[mode_id][LabelIdx].label = label;
        ModeLabel[mode_id][LabelIdx].event = event_id;
    } else {
        LKASSERT(0);
    }
}

// Return 0 for anything else - should probably return -1 !
int InputEvents::mode2int(const TCHAR *mode, bool create) {
  int i = 0;

  // Better checks !
  if (mode == NULL)
    return -1;

  for (i = 0; i < mode_map_count; i++) {
    if (_tcscmp(mode, mode_map[i]) == 0)
      return i;
  }

  if (create) {
    // Keep a copy
    LK_tcsncpy(mode_map[mode_map_count], mode, MAX_MODE_STRING-1);
    mode_map_count++;
    return mode_map_count - 1;
  }
  return -1;
}


void InputEvents::setMode(const TCHAR *mode) {
  static int lastmode = -1;
  int thismode;

#ifdef LXMINIMAP
    if (GlobalModelType == MODELTYPE_PNA_MINIMAP) {


        if (_tcscmp(mode, TEXT("default")) == 0) {
            IsMenuShown = false;
            SelectedButtonId = 1;
        } else
            IsMenuShown = true;

    }
#endif

  LK_tcsncpy(mode_current, mode, MAX_MODE_STRING-1);

  // Mode must already exist to use it here...
  thismode = mode2int(mode,false);
  if (thismode < 0)	// Technically an error in config (eg
			// event=Mode DoesNotExist)
    return;	// TODO enhancement: Add debugging here

  if (thismode == lastmode) {
	//
	// Clicking again would switch menu off for these cases
	//
	if (
	(_tcscmp(mode, TEXT("MMCONF")) == 0)	||
	(_tcscmp(mode, TEXT("MTarget")) == 0) )
	{
		IsMultimapConfigShown=false;
		_tcscpy(mode_current, _T("default"));
		thismode = mode2int(_T("default"),false);
	} else {
	//
	// For all other cases, simply do nothing
	//
		return;
	}
  }

  // Special flags cleanup..
  if (_tcscmp(mode, TEXT("MMCONF")) != 0) {
	IsMultimapConfigShown=false;
  }

  drawButtons(thismode);

  lastmode = thismode;

}

void InputEvents::drawButtons(int Mode) {
    if (!(ProgramStarted == psNormalOp)) return;

    for (unsigned i = 0; i < array_size(ModeLabel[Mode]); i++) {
      ButtonLabel::SetLabelText( i+1, ModeLabel[Mode][i].label );
    }

#ifdef USE_GDI
    MapWindow::RequestFastRefresh();
#endif

}

TCHAR* InputEvents::getMode() {
  return mode_current;
}

int InputEvents::getModeID() {
  return InputEvents::mode2int(InputEvents::getMode(), false);
}

// -----------------------------------------------------------------------
// Processing functions - which one to do
// -----------------------------------------------------------------------


// Input is a via the user touching the label on a touch screen / mouse
bool InputEvents::processButton(unsigned MenuId) {
    if (!(ProgramStarted==psNormalOp)) return false;

#if TESTBENCH
    LKASSERT(MenuId > 0);
#else
    if (MenuId < 1) return false;
#endif

#ifdef LXMINIMAP
    SelectedButtonId = MenuId;
#endif
    int thismode = getModeID();
    unsigned i = MenuId - 1;
    LKASSERT(i < array_size(ModeLabel[thismode])); // Invalid MenuId

    int lastMode = thismode;

    if (!Debounce()) return true;

    // 101212 moved here so that an internal resource played will not stop LKsound running
	PlayResource(TEXT("IDR_WAV_CLICK"));

	if (ButtonLabel::IsEnabled(MenuId)) {
		processGo(ModeLabel[thismode][i].event);
	}

	// update button text, macro may change the label
	if ((lastMode == getModeID()) && (ModeLabel[thismode][i].label != NULL) && (ButtonLabel::IsVisible(MenuId))){
		drawButtons(thismode);
	}
    return true;
}


/*
  InputEvent::processKey(KeyID);
  Process keys normally brought in by hardware or keyboard presses
  Future will also allow for long and double click presses...
  Return = We had a valid key (even if nothing happens because of Bounce)
*/
bool InputEvents::processKey(int KeyID) {
  if (!(ProgramStarted==psNormalOp)) return false;

  int event_id = 0;

  // get current mode
  const int mode = InputEvents::getModeID();

#ifndef USE_GDI

  auto It = Key2Event[mode].find(KeyID);
  if(It != Key2Event[mode].end()) {
    event_id = It->second;
  }

  if (event_id == 0) {
    It = Key2Event[0].find(KeyID);
    if(It != Key2Event[0].end()) {
      event_id = It->second;
    }
  }

#else

  // Valid input ?
  if ((KeyID < 0) || (KeyID > MAX_KEY))
    return false;

  // Which key - can be defined locally or at default (fall back to default)
  event_id = Key2Event[mode][KeyID];
  if (event_id == 0) {
    // go with default key..
    event_id = Key2Event[0][KeyID];
  }

#endif

  if (event_id > 0) {

    unsigned MenuId = 0;
    int lastMode = mode;
    const TCHAR *pLabelText = NULL;

    #if 0 // REMINDER> WE WERE DEBOUNCING HARDWARE KEYS HERE ON PPC2003 AND PNA DEVICES
    if (!Debounce()) {
       return true;
    }
    #endif

    for (unsigned i = 0; i < array_size(ModeLabel[mode]); ++i) {
      if (ModeLabel[mode][i].event == event_id) {
#ifdef LXMINIMAP
        SelectedButtonId = i + 1;
#endif
        pLabelText = ModeLabel[mode][i].label;
      }
    }

    if (MenuId == 0 || ButtonLabel::IsEnabled(MenuId)) {
      InputEvents::processGo(event_id);
    }

    // experimental: update button text, macro may change the value
    if ((lastMode == getModeID()) && (MenuId > 1) && (pLabelText != NULL) && ButtonLabel::IsVisible(MenuId)) {
      drawButtons(lastMode);
    }

    return true;
  }

  return false;
}


bool InputEvents::processNmea(int ne_id) {
  // add an event to the bottom of the queue
  ScopeLock Lock(CritSec_EventQueue);
  for (int i=0; i< MAX_NMEA_QUEUE; i++) {
    if (NMEA_Queue[i]== -1) {
      NMEA_Queue[i]= ne_id;
      break;
    }
  }

  return true; // ok.
}

/*
  InputEvent::processNmea(TCHAR* data)
  Take hard coded inputs from NMEA processor.
  Return = TRUE if we have a valid key match
*/
bool InputEvents::processNmea_real(int ne_id) {
  if (!(ProgramStarted==psNormalOp)) return false;
  int event_id = 0;

  // Valid input ?
  if ((ne_id < 0) || (ne_id >= NE_COUNT))
    return false;

  // get current mode
  int mode = InputEvents::getModeID();

  // Which key - can be defined locally or at default (fall back to default)
  event_id = N2Event[mode][ne_id];
  if (event_id == 0) {
    // go with default key..
    event_id = N2Event[0][ne_id];
  }

  if (event_id > 0) {
    InputEvents::processGo(event_id);
    return true;
  }

  return false;
}


// This should be called ONLY by the GUI thread.
void InputEvents::DoQueuedEvents(void) {
  static bool blockqueue = false;
  int GCE_Queue_copy[MAX_GCE_QUEUE];
  int NMEA_Queue_copy[MAX_NMEA_QUEUE];
  int i;

  if (blockqueue) return;
  // prevent this being re-entered by gui thread while
  // still processing

  blockqueue = true;

  CAirspaceManager::Instance().ProcessAirspaceDetailQueue();

  processPopupDetails_real();

  if (RepeatWindCalc>0) { // 100203
	RepeatWindCalc=0;
	InputEvents::eventCalcWind(_T("AUTO"));
	//dlgBasicSettingsShowModal();
  }

  { // Begin Lock
    // copy and flush the queue first, blocking
    ScopeLock Lock(CritSec_EventQueue);

    for (i=0; i<MAX_GCE_QUEUE; i++) {
      GCE_Queue_copy[i]= GCE_Queue[i];
      GCE_Queue[i]= -1;
    }
    for (i=0; i<MAX_NMEA_QUEUE; i++) {
      NMEA_Queue_copy[i]= NMEA_Queue[i];
      NMEA_Queue[i]= -1;
    }
  } // End Lock

  // process each item in the queue
  for (i=0; i< MAX_GCE_QUEUE; i++) {
    if (GCE_Queue_copy[i]!= -1) {
      processGlideComputer_real(GCE_Queue_copy[i]);
    }
  }
  for (i=0; i< MAX_NMEA_QUEUE; i++) {
    if (NMEA_Queue_copy[i]!= -1) {
      processNmea_real(NMEA_Queue_copy[i]);
    }
  }

  blockqueue = false; // ok, ready to go on.
}


bool InputEvents::processGlideComputer(int gce_id) {
  // add an event to the bottom of the queue
  ScopeLock Lock(CritSec_EventQueue);
  for (int i=0; i< MAX_GCE_QUEUE; i++) {
    if (GCE_Queue[i]== -1) {
      GCE_Queue[i]= gce_id;
      break;
    }
  }
  return true; // ok.
}

void InputEvents::processPopupDetails(PopupType type, int index) {
    ScopeLock Lock(CritSec_EventQueue);
    PopupEventQueue.push_back({type, index});
}

// show details for each object queued (proccesed by MainThread inside InputsEvent::DoQueuedEvents())
void InputEvents::processPopupDetails_real() {

    while(1) {
        PopupEvent_t event;
        { // Begin Lock
            ScopeLock Lock(CritSec_EventQueue);
            if(!PopupEventQueue.empty()) {
                event = PopupEventQueue.front();
                PopupEventQueue.pop_front(); // remove object event from fifo
            } else {
                break; // no more queued event, out from loop.
            }
        } // End Lock

        switch(event.type) {
            case PopupWaypoint:
                // Do not update CommonList and Nearest Waypoint in details mode, max 60s
                LockFlightData();
			LastDoCommon = GPS_INFO.Time + NEARESTONHOLD; //@ 101003
                UnlockFlightData();

                SelectedWaypoint = event.index;
                PopupWaypointDetails();

                LastDoNearest = LastDoCommon = 0; //@ 101003
                break;
            case PopupThermal:
                // Do not update while in details mode, max 10m
                LockFlightData();
			LastDoThermalH = GPS_INFO.Time + 600;
                UnlockFlightData();

                dlgThermalDetails(event.index);

                LastDoThermalH=0;
                break;
            case PopupTraffic:
		// Do not update Traffic while in details mode, max 10m
                LockFlightData();
			LastDoTraffic = GPS_INFO.Time + 600;
                UnlockFlightData();

                dlgLKTrafficDetails(event.index);

                LastDoTraffic = 0;
                break;
            case PopupOracle:
                // Do not update Traffic while in details mode, max 10m
                extern void dlgOracleShowModal(void);
                dlgOracleShowModal();
                break;
            case PopupTeam:
                // Do not update Traffic while in details mode, max 10m
                dlgTeamCodeShowModal();
                break;
            case PopupBasic:
                // Do not update Traffic while in details mode, max 10m
                dlgBasicSettingsShowModal();
                break;
            default:
                LKASSERT(false);
                break;
        }
    }
}

/*
  InputEvents::processGlideComputer
  Take virtual inputs from a Glide Computer to do special events
*/
bool InputEvents::processGlideComputer_real(int gce_id) {
  if (!(ProgramStarted==psNormalOp)) return false;
  int event_id = 0;

  // TODO feature: Log glide computer events to IGC file

  // Valid input ?
  if ((gce_id < 0) || (gce_id >= GCE_COUNT))
    return false;

  // get current mode
  int mode = InputEvents::getModeID();

  // Which key - can be defined locally or at default (fall back to default)
  event_id = GC2Event[mode][gce_id];
  if (event_id == 0) {
    // go with default key..
    event_id = GC2Event[0][gce_id];
  }

  if (event_id > 0) {
    InputEvents::processGo(event_id);
    return true;
  }

  return false;
}

extern int MenuTimeOut;

// EXECUTE an Event - lookup event handler and call back - no return
void InputEvents::processGo(int eventid) {
  if (!(ProgramStarted==psNormalOp)) return;

  // evnentid 0 is special for "noop" - otherwise check event
  // exists (pointer to function)
  BUGSTOP_LKASSERT(eventid<=Events_count);
  if (eventid>0 && eventid<=Events_count) {  // should be ok the =
    if (Events[eventid].event) {
      Events[eventid].event(Events[eventid].misc);
      MenuTimeOut = 0;
    }
    if (Events[eventid].next > 0)
      InputEvents::processGo(Events[eventid].next);
  }
  return;
}

// -----------------------------------------------------------------------
// Execution - list of things you can do
// -----------------------------------------------------------------------

// TODO code: Keep marker text for use in log file etc.
void InputEvents::eventMarkLocation(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("reset")) == 0) {
	#if USETOPOMARKS
	reset_marks = true;
	#endif
	LockTaskData();
	for (short i=RESWP_FIRST_MARKER; i<=RESWP_LAST_MARKER; i++) {
		WayPointList[i].Latitude=RESWP_INVALIDNUMBER;
		WayPointList[i].Longitude=RESWP_INVALIDNUMBER;
		WayPointList[i].Altitude=RESWP_INVALIDNUMBER;
	        WayPointList[i].Visible=FALSE;
	        WayPointList[i].FarVisible=FALSE;
	        WayPointCalc[i].WpType = WPT_UNKNOWN;
	}
	UnlockTaskData();
	return;
  }

  LKSound(TEXT("DROPMARKER.WAV"));

  LockFlightData();

  if (_tcscmp(misc, TEXT("pan")) == 0) {
    RasterTerrain::Lock();
	short th= RasterTerrain::GetTerrainHeight(MapWindow::GetPanLatitude(), MapWindow::GetPanLongitude());
    RasterTerrain::Unlock();
	if (th==TERRAIN_INVALID) th=0;
	MarkLocation(MapWindow::GetPanLongitude(), MapWindow::GetPanLatitude(), th );
	ForceRenderMap=true;
  } else {
	#if USETOPOMARKS
	MarkLocation(GPS_INFO.Longitude, GPS_INFO.Latitude);
	#else
	MarkLocation(GPS_INFO.Longitude, GPS_INFO.Latitude, CALCULATED_INFO.NavAltitude );
	#endif
  }

  UnlockFlightData();

}

void InputEvents::eventSounds(const TCHAR *misc) {

  if (_tcscmp(misc, TEXT("toggle")) == 0)
    EnableSoundModes = !EnableSoundModes;
  else if (_tcscmp(misc, TEXT("on")) == 0)
    EnableSoundModes = true;
  else if (_tcscmp(misc, TEXT("off")) == 0)
    EnableSoundModes = false;
  else if (_tcscmp(misc, TEXT("show")) == 0) {
    if (EnableSoundModes)
	// LKTOKEN  _@M625_ = "Sounds ON"
      DoStatusMessage(MsgToken(625));
    else
	// LKTOKEN  _@M624_ = "Sounds OFF"
      DoStatusMessage(MsgToken(624));
  }
}

void InputEvents::eventBaroAltitude(const TCHAR *misc) {

  if (_tcscmp(misc, TEXT("toggle")) == 0)
    EnableNavBaroAltitude = !EnableNavBaroAltitude;
  else if (_tcscmp(misc, TEXT("on")) == 0)
    EnableNavBaroAltitude = true;
  else if (_tcscmp(misc, TEXT("off")) == 0)
    EnableNavBaroAltitude = false;
  else if (_tcscmp(misc, TEXT("show")) == 0) {
    if (EnableNavBaroAltitude)
	DoStatusMessage(MsgToken(1796)); // USING BARO ALTITUDE
    else
	DoStatusMessage(MsgToken(757));	// USING GPS ALTITUDE
  }
}

void InputEvents::eventSnailTrail(const TCHAR *misc) {

  if (_tcscmp(misc, TEXT("toggle")) == 0) {
      // We dont change in 5.0 the order OFF-LONG-SHORT-FULL but we
      // want it to become nevertheless OFF-SHORT-LONG-FULL correctly
      // So this is the trick, for the trail button
      switch(TrailActive) {
          case 0: // off
              TrailActive=2; // short
              break;
          case 1: // long
              TrailActive=3; // full
              break;
          case 2: // short
              TrailActive=1; // long
              break;
          case 3: // full
              TrailActive=0; // off
              break;
      }
  }
  else if (_tcscmp(misc, TEXT("off")) == 0)
    TrailActive = 0;
  else if (_tcscmp(misc, TEXT("long")) == 0)
    TrailActive = 1;
  else if (_tcscmp(misc, TEXT("short")) == 0)
    TrailActive = 2;
  else if (_tcscmp(misc, TEXT("full")) == 0)
    TrailActive = 3;

  else if (_tcscmp(misc, TEXT("show")) == 0) {
    if (TrailActive==0)
	// LKTOKEN  _@M620_ = "SnailTrail OFF"
      DoStatusMessage(MsgToken(620));
    if (TrailActive==1)
	// LKTOKEN  _@M622_ = "SnailTrail ON Long"
      DoStatusMessage(MsgToken(622));
    if (TrailActive==2)
	// LKTOKEN  _@M623_ = "SnailTrail ON Short"
      DoStatusMessage(MsgToken(623));
    if (TrailActive==3)
	// LKTOKEN  _@M621_ = "SnailTrail ON Full"
      DoStatusMessage(MsgToken(621));
  }
}


// The old event for VisualGlide is kept for possible future usages within M4
void InputEvents::eventVisualGlide(const TCHAR *misc) {
}

void InputEvents::eventAirSpace(const TCHAR *misc) {

  if (_tcscmp(misc, TEXT("toggle")) == 0) {
	ToggleMultimapAirspace();
  }
}

// THIS IS NOT USED ANYMORE
void InputEvents::eventActiveMap(const TCHAR *misc) {
#if 0
  if (_tcscmp(misc, TEXT("toggle")) == 0) {
	ActiveMap=!ActiveMap;
	if (ActiveMap)
		LKSound(TEXT("LK_TONEUP.WAV"));
	else
		LKSound(TEXT("LK_TONEDOWN.WAV"));
  }
  else if (_tcscmp(misc, TEXT("off")) == 0) {
    ActiveMap=false;
    LKSound(TEXT("LK_TONEDOWN.WAV"));
  } else if (_tcscmp(misc, TEXT("on")) == 0) {
    ActiveMap=true;
    LKSound(TEXT("LK_TONEUP.WAV"));
  } else if (_tcscmp(misc, TEXT("show")) == 0) {
	if (ActiveMap)
		DoStatusMessage(MsgToken(854),NULL,false); // ActiveMap ON
	else
		DoStatusMessage(MsgToken(855),NULL,false); // ActiveMap OFF
  }
#endif
}

// THIS EVENT IS NOT USED ANYMORE
void InputEvents::eventScreenModes(const TCHAR *misc) {

}



// eventAutoZoom - Turn on|off|toggle AutoZoom
// misc:
//	auto on - Turn on if not already
//	auto off - Turn off if not already
//	auto toggle - Toggle current full screen status
//	auto show - Shows autozoom status
//	+	- Zoom in
//	++	- Zoom in near
//	-	- Zoom out
//	--	- Zoom out far
//	n.n	- Zoom to a set scale
//	show - Show current zoom scale
void InputEvents::eventZoom(const TCHAR* misc) {
  // JMW pass through to handler in MapWindow
  // here:
  // -1 means toggle
  // 0 means off
  // 1 means on
  float zoom;

  if (_tcscmp(misc, TEXT("auto toggle")) == 0)
    MapWindow::zoom.EventAutoZoom(-1);
  else if (_tcscmp(misc, TEXT("auto on")) == 0)
    MapWindow::zoom.EventAutoZoom(1);
  else if (_tcscmp(misc, TEXT("auto off")) == 0)
    MapWindow::zoom.EventAutoZoom(0);
  else if (_tcscmp(misc, TEXT("auto show")) == 0) {
    if (MapWindow::zoom.AutoZoom())
	// 856 AutoZoom ON
      DoStatusMessage(MsgToken(856));
    else
	// 857 AutoZoom OFF
      DoStatusMessage(MsgToken(857));
  }
  else if (_tcscmp(misc, TEXT("slowout")) == 0)
    MapWindow::zoom.EventScaleZoom(-4);
  else if (_tcscmp(misc, TEXT("slowin")) == 0)
    MapWindow::zoom.EventScaleZoom(4);
  else if (_tcscmp(misc, TEXT("out")) == 0)
    MapWindow::zoom.EventScaleZoom(-1);
  else if (_tcscmp(misc, TEXT("in")) == 0)
    MapWindow::zoom.EventScaleZoom(1);
  else if (_tcscmp(misc, TEXT("-")) == 0)
    MapWindow::zoom.EventScaleZoom(-1);
  else if (_tcscmp(misc, TEXT("+")) == 0)
    MapWindow::zoom.EventScaleZoom(1);
  else if (_tcscmp(misc, TEXT("--")) == 0)
    MapWindow::zoom.EventScaleZoom(-2);
  else if (_tcscmp(misc, TEXT("++")) == 0)
    MapWindow::zoom.EventScaleZoom(2);
  else if (_stscanf(misc, TEXT("%f"), &zoom) == 1)
    MapWindow::zoom.EventSetZoom((double)zoom);

  else if (_tcscmp(misc, TEXT("circlezoom toggle")) == 0) {
    MapWindow::zoom.CircleZoom(!MapWindow::zoom.CircleZoom());
  } else if (_tcscmp(misc, TEXT("circlezoom on")) == 0) {
    MapWindow::zoom.CircleZoom(true);
  } else if (_tcscmp(misc, TEXT("circlezoom off")) == 0) {
    MapWindow::zoom.CircleZoom(false);
  } else if (_tcscmp(misc, TEXT("circlezoom show")) == 0) {
    if (MapWindow::zoom.CircleZoom())
	// LKTOKEN  _@M173_ = "Circling Zoom ON"
      DoStatusMessage(MsgToken(173));
    else
	// LKTOKEN  _@M172_ = "Circling Zoom OFF"
      DoStatusMessage(MsgToken(172));
  }
}

// Pan
//	on	Turn pan on
//	off	Turn pan off
//      supertoggle Toggles pan and fullscreen
//	up	zoomin		Zoom in
//	down	zoomout		Zoom out
//	left	moveleft	Pan left
//	right	moveright	Pan right
//		moveup		Pan up
//		movedown	Pan down
void InputEvents::eventPan(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("toggle")) == 0)
    MapWindow::Event_Pan(-1);
  else if (_tcscmp(misc, TEXT("supertoggle")) == 0)
    MapWindow::Event_Pan(-2);
  else if (_tcscmp(misc, TEXT("on")) == 0)
    MapWindow::Event_Pan(1);
  else if (_tcscmp(misc, TEXT("off")) == 0)
    MapWindow::Event_Pan(0);

  else if (_tcscmp(misc, TEXT("up")) == 0 || _tcscmp(misc, TEXT("zoomin")) == 0)
    MapWindow::zoom.EventScaleZoom(1);
  else if (_tcscmp(misc, TEXT("down")) == 0 || _tcscmp(misc, TEXT("zoomout")) == 0)
    MapWindow::zoom.EventScaleZoom(-1); // fixed v58
  else if (_tcscmp(misc, TEXT("left")) == 0 || _tcscmp(misc, TEXT("moveleft")) == 0)
    MapWindow::Event_PanCursor(1,0);
  else if (_tcscmp(misc, TEXT("right")) == 0 || _tcscmp(misc, TEXT("moveright")) == 0)
    MapWindow::Event_PanCursor(-1,0);
  else if (_tcscmp(misc, TEXT("moveup")) == 0)
    MapWindow::Event_PanCursor(0,1);
  else if (_tcscmp(misc, TEXT("movedown")) == 0)
    MapWindow::Event_PanCursor(0,-1);
  else if (_tcscmp(misc, TEXT("show")) == 0) {
    if (MapWindow::mode.AnyPan())
      DoStatusMessage(MsgToken(858)); // Pan mode ON
    else
      DoStatusMessage(MsgToken(859)); // Pan mode OFF
  }

}

void InputEvents::eventTerrainTopology(const TCHAR *misc) {

  if (_tcscmp(misc, TEXT("terrain toggle")) == 0) {
	ToggleMultimapTerrain();
	//MapWindow::RefreshMap();
  }
  if (_tcscmp(misc, TEXT("topology toggle")) == 0) {
	ToggleMultimapTopology();
	//MapWindow::RefreshMap();
  }
}


// ArmAdvance
// Controls waypoint advance trigger:
//     on: Arms the advance trigger
//    off: Disarms the advance trigger
//   toggle: Toggles between armed and disarmed.
//   show: Shows current armed state
void InputEvents::eventArmAdvance(const TCHAR *misc) {
  if (AutoAdvance>=2) {
    if (_tcscmp(misc, TEXT("on")) == 0) {
      AdvanceArmed = true;
    }
    if (_tcscmp(misc, TEXT("off")) == 0) {
      AdvanceArmed = false;
    }
    if (_tcscmp(misc, TEXT("toggle")) == 0) {
      AdvanceArmed = !AdvanceArmed;
    }
  }
  if (_tcscmp(misc, TEXT("show")) == 0) {
    switch (AutoAdvance) {
    case 0:
	// LKTOKEN  _@M105_ = "Auto Advance: Manual"
      DoStatusMessage(MsgToken(105));
      break;
    case 1:
	// LKTOKEN  _@M103_ = "Auto Advance: Automatic"
      DoStatusMessage(MsgToken(103));
      break;
    case 2:
      if (AdvanceArmed) {
	// LKTOKEN  _@M102_ = "Auto Advance: ARMED"
        DoStatusMessage(MsgToken(102));
      } else {
	// LKTOKEN  _@M104_ = "Auto Advance: DISARMED"
        DoStatusMessage(MsgToken(104));
      }
      break;
    case 3:
      if (ActiveTaskPoint<2) { // past start (but can re-start)
        if (AdvanceArmed) {
	// LKTOKEN  _@M102_ = "Auto Advance: ARMED"
          DoStatusMessage(MsgToken(102));
        } else {
	// LKTOKEN  _@M104_ = "Auto Advance: DISARMED"
          DoStatusMessage(MsgToken(104));
        }
      } else {
	// LKTOKEN  _@M103_ = "Auto Advance: Automatic"
        DoStatusMessage(MsgToken(103));
      }
      break;
    case 4:
      if (ActiveTaskPoint==0) { // past start (but can re-start)
	// LKTOKEN  _@M103_ = "Auto Advance: Automatic"
        DoStatusMessage(MsgToken(103));
      }
      else {
        if (AdvanceArmed) {
	// LKTOKEN  _@M102_ = "Auto Advance: ARMED"
          DoStatusMessage(MsgToken(102));
        } else {
	// LKTOKEN  _@M104_ = "Auto Advance: DISARMED"
          DoStatusMessage(MsgToken(104));
        }
      }
      break;
    default:
      break;
    }
  }
}

// Mode
// Sets the current event mode.
//  The argument is the label of the mode to activate.
//  This is used to activate menus/submenus of buttons
void InputEvents::eventMode(const TCHAR *misc) {
  LKASSERT(misc != NULL);
  InputEvents::setMode(misc);

#ifdef USE_GDI
  // trigger redraw of screen to reduce blank area under windows
  MapWindow::RequestFastRefresh();
#endif
}


// Checklist
// Displays the checklist dialog
//  See the checklist dialog section of the reference manual for more info.
void InputEvents::eventChecklist(const TCHAR *misc) {
	(void)misc;
  dlgChecklistShowModal(0); // 0 for notepad
}


// Displays the task calculator dialog
//  See the task calculator dialog section of the reference manual
// for more info.
void InputEvents::eventCalculator(const TCHAR *misc) {
	(void)misc;
  dlgTaskCalculatorShowModal();
}

// Status
// Displays one of the three status dialogs:
//    system: display the system status
//    aircraft: displays the aircraft status
//    task: displays the task status
//  See the status dialog section of the reference manual for more info
//  on these.
void InputEvents::eventStatus(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("system")) == 0) {
    dlgStatusShowModal(1);
  } else if (_tcscmp(misc, TEXT("task")) == 0) {
    dlgStatusShowModal(2);
  } else if (_tcscmp(misc, TEXT("Aircraft")) == 0) {
    dlgStatusShowModal(0);
  } else {
    dlgStatusShowModal(-1);
  }
}

// Analysis
// Displays the analysis/statistics dialog
//  See the analysis dialog section of the reference manual
// for more info.
void InputEvents::eventAnalysis(const TCHAR *misc) {
	(void)misc;

  #if TRACETHREAD
  TCHAR myevent[80];
  _stprintf(myevent,_T("eventAnalysis %s"),misc);
  SHOWTHREAD(myevent);
  #endif

  dlgAnalysisShowModal(ANALYSYS_PAGE_DEFAULT);

}

// WaypointDetails
// Displays waypoint details
//         current: the current active waypoint
//          select: brings up the waypoint selector, if the user then
//                  selects a waypoint, then the details dialog is shown.
//  See the waypoint dialog section of the reference manual
// for more info.
void InputEvents::eventWaypointDetails(const TCHAR *misc) {

  if (_tcscmp(misc, TEXT("current")) == 0) {
    if (ValidTaskPoint(ActiveTaskPoint)) { // BUGFIX 091116
      SelectedWaypoint = Task[ActiveTaskPoint].Index;
    }
    if (SelectedWaypoint<0){
	// LKTOKEN  _@M462_ = "No Active Waypoint!"
      DoStatusMessage(MsgToken(462));
      return;
    }
    PopupWaypointDetails();
  } else {
    if (_tcscmp(misc, TEXT("select")) == 0) {
      int res = dlgWayPointSelect();

      if (res != -1) {
	    SelectedWaypoint = res;
	    PopupWaypointDetails();
      }
    }
  }
}

void InputEvents::eventTimeGates(const TCHAR *misc) {
	dlgTimeGatesShowModal();
}

void InputEvents::eventMyMenu(const TCHAR *misc) {

  unsigned int i, ckeymode;
  i=_tcstoul(misc, NULL, 10);
  LKASSERT(i>0 && i<11);

  // test mode only!
  switch(i) {
	case 1:
		ckeymode=CustomMenu1;
                break;
        case 2:
                ckeymode=CustomMenu2;
                break;
        case 3:
                ckeymode=CustomMenu3;
                break;
        case 4:
                ckeymode=CustomMenu4;
                break;
        case 5:
                ckeymode=CustomMenu5;
                break;
        case 6:
                ckeymode=CustomMenu6;
                break;
        case 7:
                ckeymode=CustomMenu7;
                break;
	case 8:
		ckeymode=CustomMenu8;
		break;
	case 9:
		ckeymode=CustomMenu9;
		break;
	case 10:
		ckeymode=CustomMenu10;
		break;
	default:
		ckeymode=ckDisabled;
		break;
  }
  CustomKeyHandler(ckeymode+1000);
  return;

}


// StatusMessage
// Displays a user defined status message.
//    The argument is the text to be displayed.
//    No punctuation characters are allowed.
void InputEvents::eventStatusMessage(const TCHAR *misc) {
  // 110102 shorthack to handle lktokens and any character
  if (_tcslen(misc)>4) {
	if (misc[0]=='Z' && misc[1]=='Y' && misc[2]=='X') {	// ZYX synthetic tokens inside XCIs
		TCHAR nmisc[10];
		_tcscpy(nmisc,_T("_@M"));
		_tcscat(nmisc,&misc[3]);
		_tcscat(nmisc,_T("_"));
		DoStatusMessage(LKGetText(nmisc));
		return;
	}
  }
  DoStatusMessage(misc);
}

// Plays a sound from the filename
void InputEvents::eventPlaySound(const TCHAR *misc) {
  PlayResource(misc);
}

// MacCready
// Adjusts MacCready settings
// up, down, auto on, auto off, auto toggle, auto show
void InputEvents::eventMacCready(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("up")) == 0) {
    MacCreadyProcessing(1);
  } else if (_tcscmp(misc, TEXT("down")) == 0) {
    MacCreadyProcessing(-1);
  } else if (_tcscmp(misc, TEXT("5up")) == 0) {
    MacCreadyProcessing(+3);
  } else if (_tcscmp(misc, TEXT("5down")) == 0) {
    MacCreadyProcessing(-3);
  } else if (_tcscmp(misc, TEXT("auto toggle")) == 0) {
    MacCreadyProcessing(0);
  } else if (_tcscmp(misc, TEXT("auto on")) == 0) {
    MacCreadyProcessing(+2);
  } else if (_tcscmp(misc, TEXT("auto off")) == 0) {
    MacCreadyProcessing(-2);
  } else if (_tcscmp(misc, TEXT("auto final")) == 0) {
    MacCreadyProcessing(-5);
  } else if (_tcscmp(misc, TEXT("auto climb")) == 0) {
    MacCreadyProcessing(-6);
  } else if (_tcscmp(misc, TEXT("auto equiv")) == 0) {
    MacCreadyProcessing(-7);
  } else if (_tcscmp(misc, TEXT("auto both")) == 0) {
    MacCreadyProcessing(-8);
  } else if (_tcscmp(misc, TEXT("auto show")) == 0) {
    if (CALCULATED_INFO.AutoMacCready) {
      DoStatusMessage(MsgToken(860)); // Auto MacCready ON
    } else {
      DoStatusMessage(MsgToken(861)); // Auto MacCready OFF
    }
  } else if (_tcscmp(misc, TEXT("show")) == 0) {
    TCHAR Temp[100];
    _stprintf(Temp,TEXT("%0.1f"),MACCREADY*LIFTMODIFY);
    DoStatusMessage(TEXT("MacCready "), Temp);
  }
}

void InputEvents::eventChangeWindCalcSpeed(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("5up")) == 0) {
	ChangeWindCalcSpeed(5);
  } else if (_tcscmp(misc, TEXT("10up")) == 0) {
	ChangeWindCalcSpeed(10);
  } else if (_tcscmp(misc, TEXT("5down")) == 0) {
	ChangeWindCalcSpeed(-5);
  } else if (_tcscmp(misc, TEXT("10down")) == 0) {
	ChangeWindCalcSpeed(-10);
  }
}

void InputEvents::eventChangeMultitarget(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("TASK")) == 0) {
	OvertargetMode=OVT_TASK;
  } else if (_tcscmp(misc, TEXT("TASKCENTER")) == 0) {
	OvertargetMode=OVT_TASKCENTER;
  } else if (_tcscmp(misc, TEXT("BALT")) == 0) {
	OvertargetMode=OVT_BALT;
  } else if (_tcscmp(misc, TEXT("ALT1")) == 0) {
	OvertargetMode=OVT_ALT1;
  } else if (_tcscmp(misc, TEXT("ALT2")) == 0) {
	OvertargetMode=OVT_ALT2;
  } else if (_tcscmp(misc, TEXT("HOME")) == 0) {
	OvertargetMode=OVT_HOME;
  } else if (_tcscmp(misc, TEXT("THER")) == 0) {
	OvertargetMode=OVT_THER;
  } else if (_tcscmp(misc, TEXT("MATE")) == 0) {
	OvertargetMode=OVT_MATE;
  } else if (_tcscmp(misc, TEXT("FLARM")) == 0) {
	OvertargetMode=OVT_FLARM;
  } else if (_tcscmp(misc, TEXT("MARK")) == 0) {
	OvertargetMode=OVT_MARK;
  } else if (_tcscmp(misc, TEXT("PASS")) == 0) {
	OvertargetMode=OVT_PASS;
  } else if (_tcscmp(misc, TEXT("ROTATE")) == 0) {
	RotateOvertarget();
  } else if (_tcscmp(misc, TEXT("MENU")) == 0) {
	InputEvents::setMode(_T("MTarget"));
  }
}


// Allows forcing of flight mode (final glide)
void InputEvents::eventFlightMode(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("finalglide on")) == 0) {
    ForceFinalGlide = true;
  }
  if (_tcscmp(misc, TEXT("finalglide off")) == 0) {
    ForceFinalGlide = false;
  }
  if (_tcscmp(misc, TEXT("finalglide toggle")) == 0) {
    ForceFinalGlide = !ForceFinalGlide;
  }
  if (_tcscmp(misc, TEXT("show")) == 0) {
    if (ForceFinalGlide) {
	// LKTOKEN  _@M289_ = "Final glide forced ON"
      DoStatusMessage(MsgToken(289));
    } else {
	// LKTOKEN  _@M288_ = "Final glide automatic"
      DoStatusMessage(MsgToken(288));
    }
  }
  if (ForceFinalGlide && ActiveTaskPoint == -1){
	// LKTOKEN  _@M462_ = "No Active Waypoint!"
    DoStatusMessage(MsgToken(462));
  }
}


// Wind
// Adjusts the wind magnitude and direction
//     up: increases wind magnitude
//   down: decreases wind magnitude
//   left: rotates wind direction counterclockwise
//  right: rotates wind direction clockwise
//   save: saves wind value, so it is used at next startup
//
// TODO feature: Increase wind by larger amounts ? Set wind to specific amount ?
//	(may sound silly - but future may get SMS event that then sets wind)
void InputEvents::eventWind(const TCHAR *misc) {
#if KEYPAD_WIND	// LXMINIMAP may use this? TODO Check
  if (_tcscmp(misc, TEXT("up")) == 0) {
    WindSpeedProcessing(1);
  }
  if (_tcscmp(misc, TEXT("down")) == 0) {
    WindSpeedProcessing(-1);
  }
  if (_tcscmp(misc, TEXT("left")) == 0) {
    WindSpeedProcessing(-2);
  }
  if (_tcscmp(misc, TEXT("right")) == 0) {
    WindSpeedProcessing(2);
  }
  if (_tcscmp(misc, TEXT("save")) == 0) {
    WindSpeedProcessing(0);
  }
#endif
}

// SendNMEA
//  Sends a user-defined NMEA string to an external instrument.
//   The string sent is prefixed with the start character '$'
//   and appended with the checksum e.g. '*40'.  The user needs only
//   to provide the text in between the '$' and '*'.
//
// For the simple SendNMEA without port specified, we assume it is a FLARM
// commanded action from menu buttons.
//
void InputEvents::eventSendNMEA(const TCHAR *misc) {
  #if TESTBENCH
  StartupStore(_T("... SendNMEA: <%s>\n"),misc);
  #endif
  //
  // We might consider strings starting with PF being FLARM stuff.
  // But since we only manage FLARM buttons in v5, no reason to complicate life.
  // if ( strncmp  misc PF ..)
  short flarmfoundonport=-1;
 for(const auto& dev : DeviceList) {
    if(dev.nmeaParser.isFlarm) {
      flarmfoundonport = dev.nmeaParser.isFlarm;
      break; // we have got first available Flarm device, ingore next device.
    }
  }  
  if (flarmfoundonport==-1) {
      DoStatusMessage(_T("NO FLARM"));
      #if TESTBENCH
      StartupStore(_T("... NO FLARM\n"));
      #endif
      return;
  }
  #if TESTBENCH
  StartupStore(_T("... SendNMEA sent to port %d\n"),flarmfoundonport);
  #endif
  if (misc) {
    PDeviceDescriptor_t found_flarm = devX(flarmfoundonport);
    if (found_flarm) {
       devWriteNMEAString(found_flarm, misc);
    }
  }
}

void InputEvents::eventSendNMEAPort1(const TCHAR *misc) {
  if (misc) {
    Port1WriteNMEA(misc);
  }
}

void InputEvents::eventSendNMEAPort2(const TCHAR *misc) {
  if (misc) {
    Port2WriteNMEA(misc);
  }
}


// AdjustWaypoint
// Adjusts the active waypoint of the task
//  next: selects the next waypoint, stops at final waypoint
//  previous: selects the previous waypoint, stops at start waypoint
//  nextwrap: selects the next waypoint, wrapping back to start after final
//  previouswrap: selects the previous waypoint, wrapping to final after start
void InputEvents::eventAdjustWaypoint(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("next")) == 0) {
    NextUpDown(1); // next
  } else if (_tcscmp(misc, TEXT("nextwrap")) == 0) {
    NextUpDown(2); // next - with wrap
  } else if (_tcscmp(misc, TEXT("previous")) == 0) {
    NextUpDown(-1); // previous
  } else if (_tcscmp(misc, TEXT("previouswrap")) == 0) {
    NextUpDown(-2); // previous with wrap
  }
}


// There is no more Suspend task 091216, eventAbortTask changed
void InputEvents::eventAbortTask(const TCHAR *misc) {

  if (ValidTaskPoint(ActiveTaskPoint)) {
	if (MessageBoxX(
	// LKTOKEN  _@M179_ = "Clear the task?"
		MsgToken(179),
	// LKTOKEN  _@M178_ = "Clear task"
		MsgToken(178),
		mbYesNo) == IdYes) {
		// clear task is locking taskdata already
		ClearTask();
	}
  } else {
	MessageBoxX(
	// LKTOKEN  _@M468_ = "No Task"
		MsgToken(468),
	// LKTOKEN  _@M178_ = "Clear task"
		MsgToken(178),
		mbOk);
  }
}


extern int CalculateWindRotary(windrotary_s *wbuf, double iaspeed, double *wfrom, double *wspeed, int windcalctime, int wmode);

#define RESCHEDTIME 20
//  if AUTO mode, be quiet and say nothing until successful
void InputEvents::eventCalcWind(const TCHAR *misc) {

  #if TRACETHREAD
  TCHAR myevent[80];
  _stprintf(myevent,_T("eventCalcWind %s"),misc);
  SHOWTHREAD(myevent);
  #endif

  double wfrom=0, wspeed=0;
  int resw=0;
  static TCHAR mbuf[200];
  TCHAR ttmp[50];
  bool reschedule=false, automode=false;
  static int wmode=0;
  // number of seconds to retry after a failure to calculate
  static short retry=RESCHEDTIME;


  // TODO use digital compass if available, using D as degrees example D125
  if (misc) {
	if (_tcscmp(misc,_T("C0")) == 0) {
		wmode=0;
	}
	if (_tcscmp(misc,_T("C1")) == 0) {
		wmode=1;
	}
	if (_tcscmp(misc,_T("C2")) == 0) {
		wmode=2;
	}
  }

  // if not automode, prepare for next automode retries
  if (misc && _tcscmp(misc,_T("AUTO"))==0) {
	automode=true;
  } else {
	RepeatWindCalc=0;
	retry=RESCHEDTIME;
  }


  // IAS if available is used automatically and in this case WindCalcSpeed is ignored
  resw=CalculateWindRotary(&rotaryWind,WindCalcSpeed, &wfrom, &wspeed,WindCalcTime, wmode);

  if (resw<=0) {
	switch(resw) {
		case WCALC_INVALID_SPEED:
	// LKTOKEN  _@M369_ = "KEEP SPEED LONGER PLEASE"
			_tcscpy(mbuf,MsgToken(369));
			reschedule=true;
			break;
		case WCALC_INVALID_TRACK:
	// LKTOKEN  _@M367_ = "KEEP HEADING LONGER PLEASE"
			_tcscpy(mbuf,MsgToken(367));
			reschedule=true;
			break;
		case WCALC_INVALID_ALL:
	// LKTOKEN  _@M368_ = "KEEP SPEED AND HEADING LONGER PLEASE"
			_tcscpy(mbuf,MsgToken(368));
			reschedule=true;
			break;
		case WCALC_INVALID_HEADING:
	// LKTOKEN  _@M344_ = "INACCURATE HEADING OR TOO STRONG WIND"
			_tcscpy(mbuf,MsgToken(344));
			break;
		case WCALC_INVALID_IAS:
	// LKTOKEN  _@M366_ = "KEEP AIRSPEED CONSTANT LONGER PLEASE"
			_tcscpy(mbuf,MsgToken(366));
			reschedule=true;
			break;
		case WCALC_INVALID_NOIAS:
	// LKTOKEN  _@M345_ = "INVALID AIRSPEED"
			_tcscpy(mbuf,MsgToken(345));
			break;
		default:
			_stprintf(mbuf,_T("INVALID DATA CALCULATING WIND %d"), resw);
			break;
	}
	if (automode==true) {
		if (reschedule && --retry>0)
			RepeatWindCalc=1;
		else
			// be sure that we dont enter a loop here!
			RepeatWindCalc=0;
	} else {
		if (reschedule) {
			DoStatusMessage(mbuf);
			RepeatWindCalc=1;
		} else {
			DoStatusMessage(mbuf);
			RepeatWindCalc=0;
		}
	}
	return;
  }

  _stprintf(mbuf,_T("%.0f%s from %.0f%s\n\nAccept and save?"),
	wspeed/3.6*SPEEDMODIFY, Units::GetHorizontalSpeedName(), wfrom, MsgToken(2179));

#if 0
  if (reswp<80) _stprintf(ttmp,_T("TrueWind! Quality: low"));
  if (reswp<98) _stprintf(ttmp,_T("TrueWind! Quality: fair"));
  if (reswp>=98) _stprintf(ttmp,_T("TrueWind! Quality: good"));
#else
  _stprintf(ttmp,_T("TrueWind! %s: %d%%"),MsgToken(866),resw); // Quality
#endif

  if (MessageBoxX(mbuf, ttmp, mbYesNo) == IdYes) {

	SetWindEstimate(wspeed/3.6,wfrom);

	CALCULATED_INFO.WindSpeed=wspeed/3.6;
	CALCULATED_INFO.WindBearing=wfrom;

	// LKTOKEN  _@M746_ = "TrueWind updated!"
	DoStatusMessage(MsgToken(746));
  }
}

void InputEvents::eventInvertColor(const TCHAR *misc) { // 100114

  static short oldOutline;
  if (DoInit[MDI_EVENTINVERTCOLOR]) {
	oldOutline=OutlinedTp;
	DoInit[MDI_EVENTINVERTCOLOR]=false;
  }

  if (OutlinedTp>(OutlinedTp_t)otDisabled)
	OutlinedTp=(OutlinedTp_t)otDisabled;
  else
	OutlinedTp=oldOutline;
  Appearance.InverseInfoBox = !Appearance.InverseInfoBox;
  return;

}

void InputEvents::eventResetTask(const TCHAR *misc) { // 100117

  if (ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1)) {
	if (MessageBoxX(
	// LKTOKEN  _@M563_ = "Restart task?"
		MsgToken(563),
	// LKTOKEN  _@M562_ = "Restart task"
		MsgToken(562),
		mbYesNo) == IdYes) {
		LockTaskData();
		ResetTask(true);
		UnlockTaskData();
	}
  } else {
	MessageBoxX(
	// LKTOKEN  _@M468_ = "No Task"
		MsgToken(468),
	// LKTOKEN  _@M562_ = "Restart task"
		MsgToken(562),
		mbOk);
  }

}

void InputEvents::eventResetQFE(const TCHAR *misc) { // 100211
	if (MessageBoxX(
	// LKTOKEN  _@M557_ = "Reset QFE?"
		MsgToken(557),
	// LKTOKEN  _@M559_ = "Reset zero QFE"
		MsgToken(559),
		mbYesNo) == IdYes) {
			QFEAltitudeOffset=ALTITUDEMODIFY*CALCULATED_INFO.NavAltitude; // 100211
	}

}

void InputEvents::eventRestartCommPorts(const TCHAR *misc) { // 100211
	if (MessageBoxX(
	// LKTOKEN  _@M558_ = "Reset and restart COM Ports?"
		MsgToken(558),
	// LKTOKEN  _@M538_ = "RESET ALL COMM PORTS"
		MsgToken(538),
		mbYesNo) == IdYes) {
			LKForceComPortReset=true;
			// also reset warnings
			PortMonitorMessages=0; // 100221
	}
}

// Simple events with no arguments.
// USE SERVICE EVENTS INSTEAD OF CREATING NEW EVENTS!
void InputEvents::eventService(const TCHAR *misc) {
  #if TRACETHREAD
  TCHAR myevent[80];
  _stprintf(myevent,_T("eventService %s"),misc);
  SHOWTHREAD(myevent);
  #endif

  if (_tcscmp(misc, TEXT("TAKEOFF")) == 0) {
	// No MESSAGE on screen, only a sound
	if (ISCAR)
		DoStatusMessage(MsgToken(571),NULL,false); // START
	else {
		if (SIMMODE) DoStatusMessage(MsgToken(930),NULL,false); // Takeoff
	}
    LKSound(_T("LK_TAKEOFF.WAV"));
	return;
  }
  if (_tcscmp(misc, TEXT("LANDING")) == 0) {
	DoStatusMessage(MsgToken(931),NULL,false);
    LKSound(_T("LK_LANDING.WAV"));
	return;
  }


  if (_tcscmp(misc, TEXT("GPSRESTART")) == 0) {
	DoStatusMessage(MsgToken(928),NULL,false);
	return;
  }

  if (_tcscmp(misc, TEXT("TASKSTART")) == 0) {
	extern void TaskStartMessage(void);
	TaskStartMessage();
    LKSound(_T("LK_TASKSTART.WAV"));
	return;
  }

  if (_tcscmp(misc, TEXT("TASKFINISH")) == 0) {

	extern void TaskFinishMessage(void);
	TaskFinishMessage();
    LKSound(_T("LK_TASKFINISH.WAV"));
	return;
  }

  if (_tcscmp(misc, TEXT("TASKNEXTWAYPOINT")) == 0) {
	// LKTOKEN  _@M461_ = "Next turnpoint"
	DoStatusMessage(MsgToken(461));
	LKSound(_T("LK_TASKPOINT.WAV"));
	return;
  }

  if (_tcscmp(misc, TEXT("ORBITER")) == 0) {
	Orbiter=!Orbiter;
	if (Orbiter)
		DoStatusMessage(MsgToken(867)); // ORBITER ON
	else
		DoStatusMessage(MsgToken(868)); // ORBITER OFF
	return;
  }

  if (_tcscmp(misc, TEXT("TASKCONFIRMSTART")) == 0) {
	bool startTaskAnyway = false;
        LKSound(_T("LK_TASKSTART.WAV"));
	dlgStartTaskShowModal(&startTaskAnyway,
		CALCULATED_INFO.TaskStartTime,
		CALCULATED_INFO.TaskStartSpeed,
		CALCULATED_INFO.TaskStartAltitude);
	if (startTaskAnyway) {
		ActiveTaskPoint=0;
		StartTask(&GPS_INFO,&CALCULATED_INFO, true, true);
		// GCE_TASK_START does not work here, why?
		InputEvents::eventService(_T("TASKSTART"));
	}
	return;
  }


  if (_tcscmp(misc, TEXT("SHADING")) == 0) {
	Shading = !Shading;
	return;
  }
  if (_tcscmp(misc, TEXT("OVERLAYS")) == 0) {
	ToggleMultimapOverlays();
	return;
  }

  if (_tcscmp(misc, TEXT("LOCKMODE")) == 0) {
	TCHAR mtext[80];
	if (LockMode(1)) {
		// LKTOKEN  _@M960_ = "CONFIRM SCREEN UNLOCK?"
		_tcscpy(mtext, MsgToken(960));
	} else {
		// LKTOKEN  _@M961_ = "CONFIRM SCREEN LOCK?"
		_tcscpy(mtext, MsgToken(961));
	}
	if (MessageBoxX(
		mtext, _T(""),
		mbYesNo) == IdYes) {
			if (LockMode(2)) { // invert LockMode
				if (ISPARAGLIDER)
					DoStatusMessage(MsgToken(962)); // SCREEN IS LOCKED UNTIL TAKEOFF
				DoStatusMessage(MsgToken(1601)); // DOUBLECLICK TO UNLOCK
			} else
				DoStatusMessage(MsgToken(964)); // SCREEN IS UNLOCKED
	}
	return;
  }

  if (_tcscmp(misc, TEXT("UTMPOS")) == 0) {
	extern void LatLonToUtmWGS84 (int& utmXZone, char& utmYZone, double& easting, double& northing, double lat, double lon);
	int utmzone; char utmchar;
	double easting, northing;
	TCHAR mbuf[80];
	PlayResource(TEXT("IDR_WAV_CLICK"));
	LatLonToUtmWGS84 ( utmzone, utmchar, easting, northing, GPS_INFO.Latitude, GPS_INFO.Longitude );
	_stprintf(mbuf,_T("UTM %d%c  %.0f  %.0f"), utmzone, utmchar, easting, northing);
	Message::Lock();
	Message::AddMessage(60000, 1, mbuf);
	TCHAR sLongitude[16];
	TCHAR sLatitude[16];
	Units::LongitudeToString(GPS_INFO.Longitude, sLongitude, sizeof(sLongitude)-1);
	Units::LatitudeToString(GPS_INFO.Latitude, sLatitude, sizeof(sLatitude)-1);
	_stprintf(mbuf,_T("%s %s"), sLatitude, sLongitude);
	Message::AddMessage(60000, 1, mbuf);
	Message::Unlock();
	return;
  }

  if (_tcscmp(misc, TEXT("ORACLE")) == 0) {
	extern void dlgOracleShowModal(void);
	if (GPS_INFO.NAVWarning) {
		DoStatusMessage(MsgToken(1702)); // Oracle wants gps fix
		return;
	}

	LKSound(TEXT("LK_BELL.WAV"));
	dlgOracleShowModal();
	return;
  }

  if (_tcscmp(misc, TEXT("TOTALEN")) == 0) {
	UseTotalEnergy=!UseTotalEnergy;
	if (UseTotalEnergy)
		DoStatusMessage(MsgToken(1667)); // TOTAL ENERGY ON
	else
		DoStatusMessage(MsgToken(1668)); // TOTAL ENERGY OFF
	return;
  }

  if (_tcscmp(misc, TEXT("TERRCOL")) == 0) {
	if (TerrainRamp+1>=NUMRAMPS)
		TerrainRamp=0;  // 15 = NUMRAMPS -1
	else
		++TerrainRamp;
	MapWindow::RefreshMap();
	return;
  }
  if (_tcscmp(misc, TEXT("TERRCOLBACK")) == 0) {
	if (TerrainRamp-1<0)
		TerrainRamp=NUMRAMPS-1;
	else
		--TerrainRamp;
	MapWindow::RefreshMap();
	return;
  }
  if (_tcscmp(misc, TEXT("LOGBTXT")) == 0) {
	dlgChecklistShowModal(1); // 1 for logbook TXT
	return;
  }
  if (_tcscmp(misc, TEXT("LOGBLST")) == 0) {
	dlgChecklistShowModal(2); // 2 for logbook LST
	return;
  }
  if (_tcscmp(misc, TEXT("IGCFILE")) == 0) {
	dlgIgcFileShowModal();
	return;
  }

  if (_tcscmp(misc, TEXT("LOGBRESET")) == 0) {
	if (MessageBoxX(MsgToken(1751), _T(""), mbYesNo) == IdYes) {
		ResetLogBook();
		DoStatusMessage(MsgToken(1752)); // Reset
	}
	return;
  }

  if(!IsEmbedded()) {
    if (_tcscmp(misc, TEXT("SS320x240")) == 0) {
      RECT w=WindowResize(320,240);
      MainWindow.Resize(w.right-w.left, w.bottom-w.top);
      return;
    }
    if (_tcscmp(misc, TEXT("SS480x272")) == 0) {
      RECT w=WindowResize(480,272);
      MainWindow.Resize(w.right-w.left, w.bottom-w.top);
      return;
    }
    if (_tcscmp(misc, TEXT("SS640x480")) == 0) {
      RECT w=WindowResize(640,480);
      MainWindow.Resize(w.right-w.left, w.bottom-w.top);
      return;
    }
    if (_tcscmp(misc, TEXT("SS800x480")) == 0) {
      RECT w=WindowResize(800,480);
      MainWindow.Resize(w.right-w.left, w.bottom-w.top);
      return;
    }
    if (_tcscmp(misc, TEXT("SS896x672")) == 0) {
      RECT w=WindowResize(896,672);
      MainWindow.Resize(w.right-w.left, w.bottom-w.top);
      return;
    }
    if (_tcscmp(misc, TEXT("SS800x600")) == 0) {
      RECT w=WindowResize(800,600);
      MainWindow.Resize(w.right-w.left, w.bottom-w.top);
      return;
    }
    if (_tcscmp(misc, TEXT("SSINVERT")) == 0) {
        const PixelRect Rect(MainWindow.GetClientRect());
      if (Rect.GetSize().cx==896) return;
      RECT w=WindowResize(Rect.GetSize().cy, Rect.GetSize().cx);
      MainWindow.Resize(w.right-w.left, w.bottom-w.top);
      return;
    }
  } else {
    if (_tcscmp(misc, TEXT("SSINV90")) == 0) {
      RotateScreen(90);
      return;
    }
    if (_tcscmp(misc, TEXT("SSINV180")) == 0) {
      RotateScreen(180);
      return;
    }
  }

  if (_tcscmp(misc, TEXT("SAVESYS")) == 0) {
	dlgProfilesShowModal(0);
	return;
  }
  if (_tcscmp(misc, TEXT("SAVEPIL")) == 0) {
	dlgProfilesShowModal(1);
	return;
  }
  if (_tcscmp(misc, TEXT("SAVEAIR")) == 0) {
	dlgProfilesShowModal(2);
	return;
  }
  if (_tcscmp(misc, TEXT("SAVEDEV")) == 0) {
	dlgProfilesShowModal(3);
	return;
  }
  if (_tcscmp(misc, TEXT("CLEARALTERNATES")) == 0) {
	LockTaskData();
	Alternate1 = -1;
	Alternate2 = -1;
	RefreshTask();
	UnlockTaskData();
	DoStatusMessage(MsgToken(177)); // clear alternates
	return;
  }
  if (_tcscmp(misc, TEXT("PANREPOS")) == 0) {
	if (!SIMMODE) return;
	GPS_INFO.Latitude=MapWindow::GetPanLatitude();
	GPS_INFO.Longitude=MapWindow::GetPanLongitude();
	LastDoRangeWaypointListTime=0; // force DoRange
	LKSound(_T("LK_BEEP1.WAV"));
	extern bool ForceRenderMap;
	ForceRenderMap=true;
	MapWindow::ForceVisibilityScan=true;
	return;
  }

  if (_tcscmp(misc, TEXT("MMTERRAIN")) == 0) {
	ToggleMultimapTerrain();
	return;
  }
  if (_tcscmp(misc, TEXT("MMAIRSPACE")) == 0) {
	ToggleMultimapAirspace();
	return;
  }
  if (_tcscmp(misc, TEXT("MMTOPOLOGY")) == 0) {
	ToggleMultimapTopology();
	return;
  }
  if (_tcscmp(misc, TEXT("MMWAYPOINTS")) == 0) {
	ToggleMultimapWaypoints();
	return;
  }
  if (_tcscmp(misc, TEXT("MMOVERLAYS")) == 0) {
	ToggleMultimapOverlays();
	return;
  }
  extern void ToggleDrawTaskFAI(void);
  if (_tcscmp(misc, TEXT("TASKFAI")) == 0) {
	ToggleDrawTaskFAI();
	return;
  }

  if (_tcscmp(misc, TEXT("OPTIMODE")) == 0) {
	  CustomKeyHandler(ckOtimizerPointsToggle+1000); // passthrough mode
	return;
  }


  if (_tcscmp(misc, TEXT("SONAR")) == 0) {
	CustomKeyHandler(ckSonarToggle+1000); // passthrough mode
	return;
  }

  if (_tcscmp(misc, TEXT("CREDITS")) == 0) {
      dlgChecklistShowModal(3); // 3 for Credits
      return;
  }

  if(_tcscmp(misc, TEXT("TASKREVERSE")) == 0) {
	if (ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1)) {
		if (MessageBoxX(
			MsgToken(1852), // LKTOKEN  _@M1852_ = "Reverse task?"
			MsgToken(1851), // LKTOKEN  _@M1851_ = "Reverse task"
			mbYesNo) == IdYes) {
			LockTaskData();
			ReverseTask();
			UnlockTaskData();
		}
	  } else {
		MessageBoxX(
			MsgToken(468),  // LKTOKEN  _@M468_ = "No Task"
			MsgToken(1851), // LKTOKEN  _@M1851_ = "Reverse task"
			mbOk);
	  }
	return;
  }

  if (_tcscmp(misc, TEXT("TERMINAL")) == 0) {
        extern void dlgTerminal(int portnumber);
	dlgTerminal(0);
	return;
  }

  if (_tcscmp(misc, TEXT("DEFTASK")) == 0) {
     DefaultTask();
     return;
  }

  if (_tcscmp(misc, TEXT("LUMUP")) == 0) {
     if (TerrainWhiteness<=1.45) TerrainWhiteness+=0.05;
     return;
  }
  if (_tcscmp(misc, TEXT("LUMDOWN")) == 0) {
     if (TerrainWhiteness>=0.55) TerrainWhiteness-=0.05;
     return;
  }

  // we should not get here
  DoStatusMessage(_T("Unknown Service: "),misc);

}

void InputEvents::eventChangeBack(const TCHAR *misc) { // 100114

  if (BgMapColor>=(LKMAXBACKGROUNDS-1))
	BgMapColor=0;
  else
	BgMapColor++;

  return;

}


#include "device.h"
#include "McReady.h"

// Bugs
// Adjusts the degradation of glider performance due to bugs
// up: increases the performance by 10%
// down: decreases the performance by 10%
// max: cleans the aircraft of bugs
// min: selects the worst performance (50%)
// show: shows the current bug degradation
void InputEvents::eventBugs(const TCHAR *misc) {
  double oldBugs = BUGS;

  LockFlightData();

  if (_tcscmp(misc, TEXT("up")) == 0) {
    CheckSetBugs(iround(BUGS*100+10) / 100.0);
  }
  if (_tcscmp(misc, TEXT("down")) == 0) {
    CheckSetBugs(iround(BUGS*100-10) / 100.0);
  }
  if (_tcscmp(misc, TEXT("max")) == 0) {
    CheckSetBugs(1.0);
  }
  if (_tcscmp(misc, TEXT("min")) == 0) {
    CheckSetBugs(0.5);
  }
  if (_tcscmp(misc, TEXT("show")) == 0) {
    TCHAR Temp[100];
    _stprintf(Temp,TEXT("%d"), iround(BUGS*100));
    DoStatusMessage(TEXT("Bugs Performance"), Temp);
  }
  if (BUGS != oldBugs) {
    CheckSetBugs(BUGS);
    devPutBugs(BUGS);
    GlidePolar::SetBallast();
  }
  UnlockFlightData();
}

// Ballast
// Adjusts the ballast setting of the glider
// up: increases ballast by 10%
// down: decreases ballast by 10%
// max: selects 100% ballast
// min: selects 0% ballast
// show: displays a status message indicating the ballast percentage
void InputEvents::eventBallast(const TCHAR *misc) {
  double oldBallast= BALLAST;
  LockFlightData();
  if (_tcscmp(misc, TEXT("up")) == 0) {
    CheckSetBallast(iround(BALLAST*100.0+10) / 100.0);
  }
  if (_tcscmp(misc, TEXT("down")) == 0) {
    CheckSetBallast(iround(BALLAST*100.0-10) / 100.0);
  }
  if (_tcscmp(misc, TEXT("max")) == 0) {
    CheckSetBallast(1.0);
  }
  if (_tcscmp(misc, TEXT("min")) == 0) {
    CheckSetBallast(0.0);
  }
  if (_tcscmp(misc, TEXT("show")) == 0) {
    TCHAR Temp[100];
    _stprintf(Temp,TEXT("%d"),iround(BALLAST*100));
    DoStatusMessage(TEXT("Ballast %"), Temp);
  }
  if (BALLAST != oldBallast) {
    CheckSetBallast(BALLAST);
    devPutBallast(BALLAST);
    GlidePolar::SetBallast();
  }
  UnlockFlightData();
}

#include "Task.h"
#include "Logger.h"
#include "Kobo/System.hpp"
#include "Draw/ScreenProjection.h"


void InputEvents::eventAutoLogger(const TCHAR *misc) {
  #if TESTBENCH
  #else
  if (SIMMODE) return;
  #endif
  if (!DisableAutoLogger) {
    eventLogger(misc);
  }
}

// Logger
// Activates the internal IGC logger
//  start: starts the logger
// start ask: starts the logger after asking the user to confirm
// stop: stops the logger
// stop ask: stops the logger after asking the user to confirm
// toggle: toggles between on and off
// toggle ask: toggles between on and off, asking the user to confirm
// show: displays a status message indicating whether the logger is active
// nmea: turns on and off NMEA logging
// note: the text following the 'note' characters is added to the log file
void InputEvents::eventLogger(const TCHAR *misc) {

#if TESTBENCH
  StartupStore(_T(". eventLogger: %s") NEWLINE, misc );
#endif

  if (_tcscmp(misc, TEXT("start ask")) == 0) {
    guiStartLogger();
    return;
  } else if (_tcscmp(misc, TEXT("start")) == 0) {
    guiStartLogger(true);
    return;
  } else if (_tcscmp(misc, TEXT("stop ask")) == 0) {
    guiStopLogger();
    return;
  } else if (_tcscmp(misc, TEXT("stop")) == 0) {
    guiStopLogger(true);
    return;
  } else if (_tcscmp(misc, TEXT("toggle ask")) == 0) {
    guiToggleLogger();
    return;
  } else if (_tcscmp(misc, TEXT("toggle")) == 0) {
    guiToggleLogger(true);
    return;
  } else if (_tcscmp(misc, TEXT("nmea")) == 0) {
    EnableLogNMEA = !EnableLogNMEA;
    if (EnableLogNMEA) {
      DoStatusMessage(MsgToken(864)); // NMEA Log ON
      #if TESTBENCH
      StartupStore(_T("... NMEA LOG IS ON @%s%s"),WhatTimeIsIt(),NEWLINE);
      #endif
    } else {
      #if TESTBENCH
      StartupStore(_T("... NMEA LOG IS OFF @%s%s"),WhatTimeIsIt(),NEWLINE);
      #endif
      DoStatusMessage(MsgToken(865)); // NMEA Log OFF
    }
    return;
  } else if (_tcscmp(misc, TEXT("show")) == 0) {
    if (LoggerActive) {
      DoStatusMessage(MsgToken(862)); // Logger ON
    } else {
      DoStatusMessage(MsgToken(863)); // Logger OFF
    }
  } else if (_tcsncmp(misc, TEXT("note"), 4)==0) {
    // add note to logger file if available..
    LoggerNote(misc+4);
  }
}

// RepeatStatusMessage
// Repeats the last status message.  If pressed repeatedly, will
// repeat previous status messages
void InputEvents::eventRepeatStatusMessage(const TCHAR *misc) {
  (void)misc;
	// new interface
  // TODO enhancement: display only by type specified in misc field
  Message::Repeat(0);
}

// NearestAirspaceDetails
// Displays details of the nearest airspace to the aircraft in a
// status message.  This does nothing if there is no airspace within
// 100km of the aircraft.
// If the aircraft is within airspace, this displays the distance and bearing
// to the nearest exit to the airspace.


void InputEvents::eventNearestAirspaceDetails(const TCHAR *misc) {
  (void)misc;
  //double nearestdistance=0; REMOVE ALL
  //double nearestbearing=0;

  // StartHourglassCursor();
  //CAirspace *found = CAirspaceManager::Instance().FindNearestAirspace(GPS_INFO.Longitude, GPS_INFO.Latitude,
  //		      &nearestdistance, &nearestbearing );  REMOVE

  SetModeType(LKMODE_MAP,MP_MAPASP);

}

extern POINT startScreen;
// NearestWaypointDetails
// Displays the waypoint details dialog
//  aircraft:   the waypoint nearest the aircraft
//  pan:        the waypoint nearest to the pan cursor
//  screen :    the waypoint nearest to "long click" event on MapSpace ( #startScreen global variable )
void InputEvents::eventNearestWaypointDetails(const TCHAR *misc) {
    bool bOK = false;
    double lon = GPS_INFO.Longitude;
    double lat = GPS_INFO.Latitude;
    if (_tcscmp(misc, TEXT("aircraft")) == 0) {
        bOK = true;
    }
    if (_tcscmp(misc, TEXT("pan")) == 0) {
        bOK = true;
        if((MapWindow::mode.Is(MapWindow::Mode::MODE_PAN) || MapWindow::mode.Is(MapWindow::Mode::MODE_TARGET_PAN))) {
            lon = MapWindow::GetPanLongitude();
            lat = MapWindow::GetPanLatitude();
        }
    }
    if (_tcscmp(misc, TEXT("screen")) == 0) {
        bOK = true;
        const ScreenProjection _Proj;
        _Proj.Screen2LonLat(startScreen, lon, lat);
    }
    if(bOK) {
        MapWindow::Event_NearestWaypointDetails(lon, lat);
    }
}

// Null
// The null event does nothing.  This can be used to override
// default functionality
void InputEvents::eventNull(const TCHAR *misc) {
	(void)misc;
  // do nothing
}

// TaskLoad
// Loads the task of the specified filename
void InputEvents::eventTaskLoad(const TCHAR *misc) {
  if (misc && (_tcslen(misc)>0)) {

    TCHAR szFileName[MAX_PATH];
	LocalPath(szFileName,_T(LKD_TASKS), misc);
    LPCTSTR wextension = _tcsrchr(szFileName, '.');
    if(wextension) {
        if(_tcscmp(wextension,_T(LKS_TSK))==0) {
            CTaskFileHelper helper;
            if(!helper.Load(szFileName)) {

            }
        }
#ifdef OLDTASK_COMPAT
        else if (_tcscmp(wextension,_T(LKS_OLD_TSK))==0) {
            LockTaskData();
            LoadNewTask(szFileName);
            UnlockTaskData();
        }
#endif
    }
  }
}

// TaskSave
// Saves the task to the specified filename
void InputEvents::eventTaskSave(const TCHAR *misc) {
  TCHAR buffer[MAX_PATH];
  if (_tcslen(misc)>0) {
	LockTaskData();
	LocalPath(buffer,_T(LKD_TASKS), misc);
	SaveTask(buffer);
	UnlockTaskData();
  }
}


extern void SettingsEnter();
extern void SettingsLeave();

// ProfileLoad
// Loads the profile of the specified filename
void InputEvents::eventProfileLoad(const TCHAR *misc) {
  TCHAR buffer[MAX_PATH];
  TCHAR buffer2[MAX_PATH];
  bool factory=false;
  if (_tcslen(misc)>0) {

	if (_tcscmp(misc,_T("Factory")) == 0) {
		if (MessageBoxX(
			_T("This will reset configuration to factory default. Confirm ?"),
			_T("Reset Factory Profile"),
			mbYesNo) == IdNo) {
			return;
		}
		SystemPath(buffer,_T(LKD_SYSTEM), _T("FACTORYPRF")); // 100223
		factory=true;
	} else {
		LocalPath(buffer,_T(LKD_CONF), misc); // 100223
	}

	FILE *fp=NULL;
	if (_tcslen(buffer)>0) fp = _tfopen(buffer, TEXT("rb"));
	if(fp == NULL) {
		if (factory)
			_stprintf(buffer2,_T("Profile \"%s\" not found inside _System"),misc);
		else
			_stprintf(buffer2,_T("Profile \"%s\" not found inside _Configuration"),misc);
		MessageBoxX(buffer2, _T("Load Profile"), mbOk);
		return;
	}
	fclose(fp);

	if (!factory) {
		_stprintf(buffer2,_T("Confirm loading \"%s\" from _Configuration ?"),misc);
		if (MessageBoxX(
			buffer2,
			_T("Load Profile"),
			mbYesNo) == IdNo) {
			return;
		}
	}

	SettingsEnter();
	LKProfileLoad(buffer);
    LKProfileInitRuntime();
	SettingsLeave();
	_stprintf(buffer2,_T("Profile \"%s\" loaded"),misc);
	MessageBoxX(buffer2, _T("Load Profile"), mbOk);
  }
}

// ProfileSave
// Saves the profile to the specified filename
void InputEvents::eventProfileSave(const TCHAR *misc) {
  TCHAR buffer[MAX_PATH];
  if (_tcslen(misc)>0) {
	_stprintf(buffer,_T("Confirm saving \"%s\" to _Configuration ?"),misc);
	if (MessageBoxX(
		buffer,
		_T("Save Profile"),
		mbYesNo) == IdNo) {
		return;
	}

	LocalPath(buffer,_T(LKD_CONF)); // 100223
	_tcscat(buffer,_T(DIRSEP));
	_tcscat(buffer,misc);
	LKProfileSave(buffer);
	_stprintf(buffer,_T("%s saved to _Configuration "),misc);
	MessageBoxX(buffer, _T("Save Profile"), mbOk);
  }
}


void InputEvents::eventBeep(const TCHAR *misc) {
  PlayResource(misc); // 100221 FIX
}

void SystemConfiguration(short mode);

// Setup
// Activates configuration and setting dialogs
//  Basic: Basic settings (QNH/Bugs/Ballast/MaxTemperature)
//  Wind: Wind settings
//  Task: Task editor
//  Airspace: Airspace filter settings
//  Replay: IGC replay dialog
void InputEvents::eventSetup(const TCHAR *misc) {

  if (_tcscmp(misc,TEXT("Basic"))==0){
    dlgBasicSettingsShowModal();
  } else if (_tcscmp(misc,TEXT("Wind"))==0){
    dlgWindSettingsShowModal();
  } else if (_tcscmp(misc,TEXT("System"))==0){
    SystemConfiguration(0);
  } else if (_tcscmp(misc,TEXT("Radio"))==0){
#ifdef RADIO_ACTIVE
      if(RadioPara.Enabled)
    dlgRadioSettingsShowModal();
#endif  // RADIO_ACTIVE
  } else if (_tcscmp(misc,TEXT("Aircraft"))==0){
    SystemConfiguration(2);
  } else if (_tcscmp(misc,TEXT("Pilot"))==0){
    SystemConfiguration(1);
  } else if (_tcscmp(misc,TEXT("Device"))==0){
    SystemConfiguration(3);
  } else if (_tcscmp(misc,TEXT("Task"))==0){
    dlgTaskOverviewShowModal();
  } else if (_tcscmp(misc,TEXT("Airspace"))==0){
    dlgAirspaceShowModal(false);
  } else if (_tcscmp(misc,TEXT("Replay"))==0){
      dlgLoggerReplayShowModal();
#if USESWITCHES
  } else if (_tcscmp(misc,TEXT("Switches"))==0){
    dlgSwitchesShowModal();
#endif
  } else if (_tcscmp(misc,TEXT("OlcAnalysis"))==0){
    dlgAnalysisShowModal(ANALYSIS_PAGE_CONTEST);
  } else if (_tcscmp(misc,TEXT("Teamcode"))==0){
    dlgTeamCodeShowModal();
  } else if (_tcscmp(misc,TEXT("Target"))==0){
    dlgTarget();
  }

}


// AdjustForecastTemperature
// Adjusts the maximum ground temperature used by the convection forecast
// +: increases temperature by one degree celsius
// -: decreases temperature by one degree celsius
// show: Shows a status message with the current forecast temperature
void InputEvents::eventAdjustForecastTemperature(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("+")) == 0) {
    CuSonde::adjustForecastTemperature(1.0);
  }
  if (_tcscmp(misc, TEXT("-")) == 0) {
    CuSonde::adjustForecastTemperature(-1.0);
  }
  if (_tcscmp(misc, TEXT("show")) == 0) {
    TCHAR Temp[100];
    _stprintf(Temp,TEXT("%f"),CuSonde::maxGroundTemperature);
    DoStatusMessage(TEXT("Forecast temperature"), Temp);
  }
}

// Run
// Runs an external program of the specified filename.
// Note that LK will wait until this program exits.
void InputEvents::eventRun(const TCHAR *misc) {
#ifdef _WIN32
  bool doexec=false;
  TCHAR path[MAX_PATH];
  if (_tcscmp(misc, TEXT("ext1")) == 0) {
	LocalPath(path,_T("ext1.exe"));
	doexec=true;
  }
  if (_tcscmp(misc, TEXT("ext2")) == 0) {
	LocalPath(path,_T("ext2.exe"));
	doexec=true;
  }
  if (_tcscmp(misc, TEXT("ext3")) == 0) {
	LocalPath(path,_T("ext3.exe"));
	doexec=true;
  }
  if (_tcscmp(misc, TEXT("ext4")) == 0) {
	LocalPath(path,_T("ext4.exe"));
	doexec=true;
  }

  if (!doexec) {
	StartupStore(_T("..... Run type <%s> unknown%s"),misc,NEWLINE);
	return;
  }
  StartupStore(_T("..... Running <%s>%s"),path,NEWLINE);


  PROCESS_INFORMATION pi;
  STARTUPINFO si;
  ZeroMemory(&si,sizeof(STARTUPINFO));
  si.cb=sizeof(STARTUPINFO);
  si.wShowWindow= SW_SHOWNORMAL;
  si.dwFlags = STARTF_USESHOWWINDOW;
  // example if (!::CreateProcess(_T("C:\\WINDOWS\\notepad.exe"),_T(""), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {

  /*cf. http://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
   * The Unicode version of this function, CreateProcessW, can modify the contents of this string. Therefore, this
   * parameter cannot be a pointer to read-only memory (such as a const variable or a literal string). If this parameter
   * is a constant string, the function may cause an access violation.
   * */
  if (!::CreateProcess(path,NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
	StartupStore(_T("... RUN FAILED%s"),NEWLINE);
	return;
  }

  ::WaitForSingleObject(pi.hProcess, INFINITE);
  ::CloseHandle(pi.hProcess);
  ::CloseHandle(pi.hThread);
  StartupStore(_T("... RUN TERMINATED%s"),NEWLINE);
#endif
}

void InputEvents::eventDeclutterLabels(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("toggle")) == 0) {

	short i=GetMultimap_Labels();
	LKASSERT( (i>=0) && (i<=MAPLABELS_END));
	if (++i>MAPLABELS_END) i=MAPLABELS_START;
	SetMultimap_Labels(i);

	switch (GetMultimap_Labels()) {

		case 0:	// MAPLABELS_ALLON
			EnableMultimapWaypoints();
			break;

		case 1: // MAPLABELS_ONLYWPS
			EnableMultimapWaypoints();
			break;

		case 2: // MAPLABELS ONLYTOPO
			DisableMultimapWaypoints();
			break;

		case 3: // MAPLABELS ALLOFF
			DisableMultimapWaypoints();
			break;

		default:
			#if TESTBENCH
			LKASSERT(0);
			#endif
			break;
	}

  }
}



void InputEvents::eventExit(const TCHAR *misc) {
	(void)misc;
#ifdef KOBO
    extern bool RestartToNickel;
    RestartToNickel = false;
#endif
    MainWindow.Close();
}

void InputEvents::eventChangeTurn(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("left")) == 0){
	if (SimTurn<=-45) return;
	SimTurn-=5;
  }
  if (_tcscmp(misc, TEXT("right")) == 0){
	if (SimTurn>=45) return;
	SimTurn+=5;
  }
}

void InputEvents::eventChangeHGPS(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("up")) == 0){
	if (Units::GetUserAltitudeUnit() == unFeet)
		GPS_INFO.Altitude += 45.71999999;
	else
		GPS_INFO.Altitude += 50;
	return;
  }
  if (_tcscmp(misc, TEXT("down")) == 0){
	if (Units::GetUserAltitudeUnit() == unFeet)
		GPS_INFO.Altitude -= 45.71999999;
	else
		GPS_INFO.Altitude -= 50;
	if ((GPS_INFO.Altitude+40)<40) GPS_INFO.Altitude=0;
	return;
  }
}

double InputEvents::getIncStep(const TCHAR *misc, double step) {
static int upCount = 0;
static int downCount = 0;

if (_tcscmp(misc, TEXT("up")) == 0 || (HasKeyboard()&&(_tcscmp(misc, TEXT("kup")) == 0))){
    step = getIncStep(step, &upCount, &downCount);
} else if (_tcscmp(misc, TEXT("down")) == 0 || (HasKeyboard()&&(_tcscmp(misc, TEXT("kdown")) == 0))){
    step = getIncStep(step, &downCount, &upCount);
}
return step;
}

double InputEvents::getIncStep(double step, int *count, int *otherCount) {

    *otherCount = 0;

    if (!myPeriodClock.CheckAlwaysUpdate(300)) {
        (*count) ++;
    } else {
        *count = 1;
    }
    if (*count >= 20) {
        step = step *10;
    }

    return step;
}

// 10 Kmh
void InputEvents::eventChangeGS(const TCHAR *misc) {
double step=0;
  if (Units::GetUserHorizontalSpeedUnit() == unKnots)
	step=0.514444; //@ 1 knot, 1.8kmh  (1 knot is 1 nautical mile per hour)
  if (Units::GetUserHorizontalSpeedUnit() == unKiloMeterPerHour)
	step=0.27778; //@ 1 kmh 0.27777  has a rounding error
  if (Units::GetUserHorizontalSpeedUnit() == unStatuteMilesPerHour)
	step=0.44704; //@ 1 mph = 1.6kmh

step = getIncStep(misc, step);

if (_tcscmp(misc, TEXT("up")) == 0){
    if (AircraftCategory == (AircraftCategory_t)umParaglider)
		GPS_INFO.Speed += step;
	else
		GPS_INFO.Speed += step*5;
	return;
  }
  if (_tcscmp(misc, TEXT("down")) == 0){
	if (AircraftCategory == (AircraftCategory_t)umParaglider)
		GPS_INFO.Speed -= step;
	else
		GPS_INFO.Speed -= step*5;
	if (GPS_INFO.Speed <0) GPS_INFO.Speed=0;
	return;
  }
  if(HasKeyboard()) {
    // key action from PC is always fine-tuned
    if (_tcscmp(misc, TEXT("kup")) == 0){
      GPS_INFO.Speed += step;
      return;
    }
    if (_tcscmp(misc, TEXT("kdown")) == 0){
      GPS_INFO.Speed -= step;
      if (GPS_INFO.Speed <0) GPS_INFO.Speed=0;
      return;
    }
  }
}

void InputEvents::eventChangeNettoVario(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("up")) == 0){
	SimNettoVario+=0.1;
  }
  if (_tcscmp(misc, TEXT("down")) == 0){
	SimNettoVario-=0.1;
  }
}

void InputEvents::eventWifi(const TCHAR *misc) {
  if (_tcscmp(misc, TEXT("toggle")) == 0){
#ifdef KOBO
      if(IsKoboWifiOn()) {
          KoboWifiOff();
          DoStatusMessage(_T("Wifi OFF"));
      } else {
          KoboWifiOn();
          DoStatusMessage(_T("Wifi ON"));
      }
#endif
  }
}

void InputEvents::eventMoveGlider(const TCHAR *misc) {
  int i;
  if (_tcscmp(misc, TEXT("reset")) == 0){
	MapWindow::GliderScreenPositionX=50;
	MapWindow::GliderScreenPositionY=MapWindow::GliderScreenPosition;
  } else if (_tcscmp(misc, TEXT("down")) == 0){
	i=MapWindow::GliderScreenPositionY - 10;
	// 20 is 20%, to avoid positioning on the bottom bar
	if (i <20) i=90;
	MapWindow::GliderScreenPositionY=i;
  } else if (_tcscmp(misc, TEXT("up")) == 0){
	i=MapWindow::GliderScreenPositionY + 10;
	if (i >90) i=20;
	MapWindow::GliderScreenPositionY=i;
  } else if (_tcscmp(misc, TEXT("left")) == 0){
	i=MapWindow::GliderScreenPositionX - 10;
	if (i <10) i=90;
	MapWindow::GliderScreenPositionX=i;
  } else if (_tcscmp(misc, TEXT("right")) == 0){
	i=MapWindow::GliderScreenPositionX + 10;
	if (i >90) i=10;
	MapWindow::GliderScreenPositionX=i;
  }
  MapWindow::RefreshMap();

}

void InputEvents::eventUserDisplayModeForce(const TCHAR *misc){

  TCHAR tmode[50];
  _stprintf(tmode,_T("%s: "), MsgToken(2249));

  if (_tcscmp(misc, TEXT("unforce")) == 0){
    MapWindow::mode.UserForcedMode(MapWindow::Mode::MODE_FLY_NONE);
    _tcscat(tmode,MsgToken(2034));
  }
  else if (_tcscmp(misc, TEXT("forceclimb")) == 0){
    MapWindow::mode.UserForcedMode(MapWindow::Mode::MODE_FLY_CIRCLING);
    _tcscat(tmode,MsgToken(2031));
  }
  else if (_tcscmp(misc, TEXT("forcecruise")) == 0){
    MapWindow::mode.UserForcedMode(MapWindow::Mode::MODE_FLY_CRUISE);
    _tcscat(tmode,MsgToken(2032));
  }
  else if (_tcscmp(misc, TEXT("forcefinal")) == 0){
    MapWindow::mode.UserForcedMode(MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
    _tcscat(tmode,MsgToken(2033));
  }
  #if 0
  else if (_tcscmp(misc, TEXT("show")) == 0){ // UNUSED
    // DoStatusMessage(TEXT(""));
  }
  #endif
  DoStatusMessage(tmode);

}

void InputEvents::eventAirspaceDisplayMode(const TCHAR *misc){

  if (_tcscmp(misc, TEXT("toggle")) == 0){
    if (++AltitudeMode >ALLOFF) AltitudeMode=ALLON;
	switch(AltitudeMode) {
		case 0:
	// LKTOKEN  _@M71_ = "Airspaces: ALL ON"
			DoStatusMessage(MsgToken(71));
			break;
		case 1:
	// LKTOKEN  _@M73_ = "Airspaces: CLIPPED"
			DoStatusMessage(MsgToken(73));
			break;
		case 2:
	// LKTOKEN  _@M72_ = "Airspaces: AUTO"
			DoStatusMessage(MsgToken(72));
			break;
		case 3:
	// LKTOKEN  _@M69_ = "Airspaces: ALL BELOW"
			DoStatusMessage(MsgToken(69));
			break;
		case 4:
	// LKTOKEN  _@M74_ = "Airspaces: INSIDE"
			DoStatusMessage(MsgToken(74));
			break;
		case 5:
	// LKTOKEN  _@M70_ = "Airspaces: ALL OFF"
			DoStatusMessage(MsgToken(70));
			break;
		default:
			break;
	}
  }
  if (_tcscmp(misc, TEXT("all")) == 0){
    AltitudeMode = ALLON;
  }
  else if (_tcscmp(misc, TEXT("clip")) == 0){
    AltitudeMode = CLIP;
  }
  else if (_tcscmp(misc, TEXT("auto")) == 0){
    AltitudeMode = AUTO;
  }
  else if (_tcscmp(misc, TEXT("below")) == 0){
    AltitudeMode = ALLBELOW;
  }
  else if (_tcscmp(misc, TEXT("off")) == 0){
    AltitudeMode = ALLOFF;
  }
}

// THIS IS UNUSED, AND SHOULD NOT BE USED SINCE IT DOES NOT SUPPORT CUPs
void InputEvents::eventAddWaypoint(const TCHAR *misc) {
  WAYPOINT edit_waypoint;
  LockTaskData();
  size_t tmpWaypointNum = WayPointList.size();

  edit_waypoint.Latitude = GPS_INFO.Latitude;
  edit_waypoint.Longitude = GPS_INFO.Longitude;
  edit_waypoint.Altitude = CALCULATED_INFO.TerrainAlt;
  edit_waypoint.FileNum = 2; // don't put into file
  edit_waypoint.Flags = 0;
  if (_tcscmp(misc, TEXT("landable")) == 0) {
    edit_waypoint.Flags += LANDPOINT;
  }
  edit_waypoint.Comment = NULL;
  _stprintf(edit_waypoint.Name,TEXT("_%u"), (unsigned)tmpWaypointNum);
  edit_waypoint.Details = 0;
  edit_waypoint.Number = WayPointList.size();

  AddWaypoint(edit_waypoint);

  UnlockTaskData();
}



void InputEvents::eventOrientation(const TCHAR *misc){
int iOrientation = DisplayOrientation ;

  if (_tcscmp(misc, TEXT("northup")) == 0){
    iOrientation = NORTHUP;
  }
  else if (_tcscmp(misc, TEXT("northcircle")) == 0){
	iOrientation = NORTHCIRCLE;
  }
  else if (_tcscmp(misc, TEXT("trackcircle")) == 0){
	iOrientation = TRACKCIRCLE;
  }
  else if (_tcscmp(misc, TEXT("trackup")) == 0){
	iOrientation = TRACKUP;
  }
  else if (_tcscmp(misc, TEXT("northtrack")) == 0){
	iOrientation = NORTHTRACK;
  }
  else if (_tcscmp(misc, TEXT("northsmart")) == 0){ // 100417
	iOrientation = NORTHSMART;
	/*
	if (InfoBoxLayout::landscape)
		DisplayOrientation = NORTHSMART;
	else
		DisplayOrientation = NORTHUP;
	*/
    }

//  if(IsMultiMap())
    if  (MapSpaceMode==MSM_MAP)
    {
	  DisplayOrientation = iOrientation;
      MapWindow::SetAutoOrientation(true); // 101008 reset it
    }
    else
	  SetMMNorthUp(GetSideviewPage(),iOrientation);



}

void SwitchToMapWindow(void)
{
  MainWindow.SetFocus();

  if (MenuTimeOut< MenuTimeout_Config) {
	MenuTimeOut = MenuTimeout_Config;
  }
}


void PopupWaypointDetails()
{
  // Quick is returning:
  // 0 for cancel or error
  // 1 for details
  // 2 for goto
  // 3 and 4 for alternates
  // 5 for task

  // DialogActive is needed if not full screen!
  // DialogActive=true;
  short ret= dlgWayQuickShowModal();
  // DialogActive=false;

  switch(ret) {
	case 1:
		dlgWayPointDetailsShowModal(0);
		break;
	case 2:
		SetModeType(LKMODE_MAP,MP_MOVING);
		break;
	case 5:
		dlgWayPointDetailsShowModal(2);
		break;
	default:
		break;
  }
}


void PopupBugsBallast(int UpDown)
{
  (void)UpDown;
  FullScreen();
  SwitchToMapWindow();
}

void HideMenu() {
    MenuTimeOut = MenuTimeout_Config;
}

void ShowMenu() {
  PlayResource(TEXT("IDR_WAV_CLICK"));
  InputEvents::setMode(TEXT("Menu"));
  MenuTimeOut = 0;
}



void FullScreen() {

  if (!MenuActive) {
#ifdef USE_GDI
    MainWindow.Fullscreen();
#endif
  }
  MapWindow::RequestFastRefresh();
}

#if KEYPAD_WIND // LXMINIMAP TODO CHECK IF NEEDED
void	WindDirectionProcessing(int UpDown)
{

	if(UpDown==1)
	{
		CALCULATED_INFO.WindBearing  += 5;
		while (CALCULATED_INFO.WindBearing  >= 360)
		{
			CALCULATED_INFO.WindBearing  -= 360;
		}
	}
	else if (UpDown==-1)
	{
		CALCULATED_INFO.WindBearing  -= 5;
		while (CALCULATED_INFO.WindBearing  < 0)
		{
			CALCULATED_INFO.WindBearing  += 360;
		}
	} else if (UpDown == 0) {
          SetWindEstimate(CALCULATED_INFO.WindSpeed,
                          CALCULATED_INFO.WindBearing);
	}
	return;
}


void	WindSpeedProcessing(int UpDown)
{
	if(UpDown==1)
		CALCULATED_INFO.WindSpeed += (1/SPEEDMODIFY);
	else if (UpDown== -1)
	{
		CALCULATED_INFO.WindSpeed -= (1/SPEEDMODIFY);
		if(CALCULATED_INFO.WindSpeed < 0)
			CALCULATED_INFO.WindSpeed = 0;
	}
	// JMW added faster way of changing wind direction
	else if (UpDown== -2) {
		WindDirectionProcessing(-1);
	} else if (UpDown== 2) {
		WindDirectionProcessing(1);
	} else if (UpDown == 0) {
          SetWindEstimate(CALCULATED_INFO.WindSpeed,
                          CALCULATED_INFO.WindBearing);
	}
	return;
}
#endif

void	MacCreadyProcessing(int UpDown)
{

  if(UpDown==1) {
	CALCULATED_INFO.AutoMacCready=false;  // 091214 disable AutoMacCready when changing MC values
    MACCREADY += (double)0.1/LIFTMODIFY; // BUGFIX 100102

    if (MACCREADY>12.0) MACCREADY=12.0;
  }
  else if(UpDown==-1)
    {
	CALCULATED_INFO.AutoMacCready=false;  // 091214 disable AutoMacCready when changing MC values
      MACCREADY -= (double)0.1/LIFTMODIFY; // 100102
      if(MACCREADY < 0)
	{
	  MACCREADY = 0;
	}

  } else if (UpDown==0)
    {
      CALCULATED_INFO.AutoMacCready = !CALCULATED_INFO.AutoMacCready;
      // JMW toggle automacready
	}
  else if (UpDown==-2)
    {
      CALCULATED_INFO.AutoMacCready = false;  // SDP on auto maccready

    }
  else if (UpDown==+2)
    {
      CALCULATED_INFO.AutoMacCready = true;	// SDP off auto maccready

    }
  else if (UpDown==3)
    {
	CALCULATED_INFO.AutoMacCready=false;  // 091214 disable AutoMacCready when changing MC values
	MACCREADY += (double)0.5/LIFTMODIFY; // 100102
	if (MACCREADY>12.0) MACCREADY=12.0;

    }
  else if (UpDown==-3)
    {
	CALCULATED_INFO.AutoMacCready=false;  // 091214 disable AutoMacCready when changing MC values
	MACCREADY -= (double)0.5/LIFTMODIFY; // 100102
	if (MACCREADY<0) MACCREADY=0;

    }
  else if (UpDown==-5)
    {
      CALCULATED_INFO.AutoMacCready = true;
      AutoMcMode=amcFinalGlide;
    }
  else if (UpDown==-6)
    {
      CALCULATED_INFO.AutoMacCready = true;
      AutoMcMode=amcAverageClimb;
    }
  else if (UpDown==-7)
    {
      CALCULATED_INFO.AutoMacCready = true;
      AutoMcMode=amcEquivalent;
    }
  else if (UpDown==-8)
    {
      CALCULATED_INFO.AutoMacCready = true;
      AutoMcMode=amcFinalAndClimb;
    }
  
  devPutMacCready(MACCREADY);
  
  return;
}

/*
	1	Next waypoint
	0	Show waypoint details
	-1	Previous waypoint
	2	Next waypoint with wrap around
	-2	Previous waypoint with wrap around
*/
void NextUpDown(int UpDown)
{

  if (!ValidTaskPoint(ActiveTaskPoint)) {	// BUGFIX 091116
	StartupStore(_T(". DBG-801 activewaypoint%s"),NEWLINE);
	return;
  }

  LockTaskData();

  if(UpDown>0) {
    // this was a bug. checking if AWP was < 0 assuming AWP if inactive was -1; actually it can also be 0, a bug is around
    if(ActiveTaskPoint < MAXTASKPOINTS) {
      // Increment Waypoint
      if(Task[ActiveTaskPoint+1].Index >= 0) {
	if(ActiveTaskPoint == 0)	{
	  // manual start
	  // TODO bug: allow restart
	  // TODO bug: make this work only for manual
	  if (CALCULATED_INFO.TaskStartTime==0) {
	    CALCULATED_INFO.TaskStartTime = GPS_INFO.Time;
	  }
	}
	ActiveTaskPoint ++;
        LKASSERT(ValidTaskPoint(ActiveTaskPoint));
        if (ValidTaskPoint(ActiveTaskPoint)) {
	    AdvanceArmed = false;
	    CALCULATED_INFO.LegStartTime = GPS_INFO.Time ;
        } else {
	    ActiveTaskPoint--;
        }
      }
      // No more, try first
      else
        if((UpDown == 2) && (Task[0].Index >= 0)) {
          /* ****DISABLED****
          if(ActiveTaskPoint == 0)	{
            // TODO bug: allow restart
            // TODO bug: make this work only for manual

            // TODO bug: This should trigger reset of flight stats, but
            // should ask first...
            if (CALCULATED_INFO.TaskStartTime==0) {
              CALCULATED_INFO.TaskStartTime = GPS_INFO.Time ;
            }
          }
          */
          AdvanceArmed = false;
          ActiveTaskPoint = 0;
          CALCULATED_INFO.LegStartTime = GPS_INFO.Time ;
        }
    }
  }
  else if (UpDown<0){
    if(ActiveTaskPoint >0) {

      ActiveTaskPoint --;
      /*
	XXX How do we know what the last one is?
	} else if (UpDown == -2) {
	ActiveTaskPoint = MAXTASKPOINTS;
      */
    } else {
      if (ActiveTaskPoint==0) {

        RotateStartPoints();

	// restarted task..
	//	TODO bug: not required? CALCULATED_INFO.TaskStartTime = 0;
      }
    }
    aatdistance.ResetEnterTrigger(ActiveTaskPoint);
  }
  else if (UpDown==0) {
    #if BUGSTOP
    LKASSERT(ActiveTaskPoint>=0);
    #endif
    if (ActiveTaskPoint>=0) {
        SelectedWaypoint = Task[ActiveTaskPoint].Index;
        PopupWaypointDetails();
    }
  }
  if (ActiveTaskPoint>=0) {
    SelectedWaypoint = Task[ActiveTaskPoint].Index;
  }
  UnlockTaskData();
}


//
//       ***************** MINIMAP ONLY ************************
//

#ifdef LXMINIMAP

unsigned InputEvents::getSelectedButtonId()
{
	return SelectedButtonId;
}


void InputEvents::eventChangeSorting(const TCHAR *misc)
{
      int j = SortedMode[MapSpaceMode];

	  if (_tcscmp(misc, TEXT("PREVIOUS")) == 0)
	  {
			  if(j==0)
				  j = 4;
			  else
				  j--;

	  }
	  else
	  {
		  if(j>=4)
		    j = 0;
		  else
		    j++;
	  }

	  switch(MapSpaceMode) {
			case MSM_LANDABLE:
			case MSM_AIRPORTS:
			case MSM_NEARTPS:
						SortedMode[MapSpaceMode]=j;
						LKForceDoNearest=true;
						PlayResource(TEXT("IDR_WAV_CLICK"));
						break;
			case MSM_TRAFFIC:
						SortedMode[MapSpaceMode]=j;
						// force immediate resorting
						LastDoTraffic=0;
						PlayResource(TEXT("IDR_WAV_CLICK"));
						break;
			case MSM_AIRSPACES:
						SortedMode[MapSpaceMode]=j;
						LastDoAirspaces=0;
						PlayResource(TEXT("IDR_WAV_CLICK"));
						break;

			case MSM_THERMALS:
						SortedMode[MapSpaceMode]=j;
						// force immediate resorting
						LastDoThermalH=0;
						PlayResource(TEXT("IDR_WAV_CLICK"));
						break;
			default:
					//	DoStatusMessage(_T("ERR-022 UNKNOWN MSM in VK"));
						break;
		}
		SelectedPage[MapSpaceMode]=0;
		SelectedRaw[MapSpaceMode]=0;
		MapWindow::RefreshMap();

}

bool InputEvents::isSelectMode()
{
	if(!LastActiveSelectMode.Check(5000))
	{
		 LastActiveSelectMode.Update();
		return true;
	}
	else return false;
}


void InputEvents::eventMinimapKey(const TCHAR *misc)
{
	//MinimapKey


	 if (_tcscmp(misc, TEXT("J")) == 0)
	 {
		     PlayResource(TEXT("IDR_WAV_CLICK"));
			 eventZoom(_T("out"));
	 }
	 else if (_tcscmp(misc, TEXT("K")) == 0)
	 {
		    PlayResource(TEXT("IDR_WAV_CLICK"));
			eventZoom(_T("in"));
	 }
	 else if (_tcscmp(misc, TEXT("SELECTMODE")) == 0)
      {
		if(isSelectMode())
		{
			LastActiveSelectMode.Reset();
			eventStatusMessage(_T("Select deactive!"));
		}
		else if(ModeIndex == 0)
		{	   PlayResource(TEXT("IDR_WAV_CLICK"));
			  eventZoom(_T("out"));
		}
		else
		{
			 LastActiveSelectMode.Update();
			 eventStatusMessage(_T("Select active!"));
		}


      }
	  else if (_tcscmp(misc, TEXT("DOWN")) == 0)
	  {
		  if(isSelectMode())
		  {

			  LKevent=LKEVENT_DOWN;
						 MapWindow::RefreshMap();
		  }
		  else if(IsMenuShown)
		  {
			  PlayResource(TEXT("IDR_WAV_CLICK"));
			  short retry = 0;
			  do
			  {
			  switch (SelectedButtonId)
			                {
			                    case 1:
			                    case 2:
			                    case 3: SelectedButtonId++; break;
			                    case 4: SelectedButtonId = 9; break;
			                    case 5: SelectedButtonId = 10; break;
			                    case 6:
			                    case 7:
			                    case 8:
			                    case 9: SelectedButtonId--; break;
			                    case 10:
			                    case 11:
			                    case 12: SelectedButtonId++; break;
			                    case 13: SelectedButtonId=1; break;
			                    default:
						SelectedButtonId = 1;
			                        break;
			                }
			  retry++;
			 }while(!ButtonLabel::IsVisible(SelectedButtonId) && retry < NUMBUTTONLABELS);

			          int thismode = getModeID();
					  drawButtons(thismode);
					//  MapWindow::RefreshMap();
		  }
		  else
		  {
			  NextModeIndex();
			  MapWindow::RefreshMap();
			  SoundModeIndex();
		  }
	  }
	  else if(_tcscmp(misc, TEXT("UP")) == 0)
	  {
		   if(isSelectMode())
		   {
			              LKevent=LKEVENT_UP;
						 MapWindow::RefreshMap();
		   }
		   else if(IsMenuShown)
		   {
			   PlayResource(TEXT("IDR_WAV_CLICK"));
			 short retry = 0;

			 do
			 {
			  switch (SelectedButtonId)
			  {
			                    case 1: SelectedButtonId = 13; break;
			                    case 2:
			                    case 3:
			                    case 4: SelectedButtonId--; break;
			                    case 5:
			                    case 6:
			                    case 7:
			                    case 8: SelectedButtonId++; break;
			                    case 9: SelectedButtonId = 4; break;
			                    case 10: SelectedButtonId = 5; break;
			                    case 11:
			                    case 12:
			                    case 13: SelectedButtonId--; break;
			                    default:
						SelectedButtonId = 1;
			                        break;
			  }
			  retry++;
			 }while(ButtonLabel::IsVisible(SelectedButtonId) && retry < NUMBUTTONLABELS);


			  int thismode = getModeID();
			  drawButtons(thismode);
		//  MapWindow::RefreshMap();
		   }
		   else
		  {
					  PreviousModeIndex();
					  MapWindow::RefreshMap();
					  SoundModeIndex();
		  }

	  }
	  else if(_tcscmp(misc, TEXT("LEFT")) == 0)
	  {

		 if( isSelectMode())
		 {
			 eventChangeSorting(_T("PREVIOUS"));
		 }
		 else if(ModeIndex == 0) //tukaj preverjam kateri mode je	in ce je pravi potem menjavam poglede
			{

				BottomBarChange(false);
				MapWindow::RefreshMap();
				PlayResource(TEXT("IDR_WAV_BTONE2"));

			}
			else
			{
				ProcessVirtualKey(100, 100,0 , LKGESTURE_LEFT);
			}
	   }
	  else if(_tcscmp(misc, TEXT("RIGHT")) == 0)
	   {

		    if( isSelectMode())
		    {
					 eventChangeSorting(_T("NEXT"));
		    }
		    else if(ModeIndex == 0) //tukaj preverjam kateri mode je	in ce je pravi potem menjavam poglede
			{
			    BottomBarChange(true);
				MapWindow::RefreshMap();
				PlayResource(TEXT("IDR_WAV_BTONE2"));
			}
			else
			{
			ProcessVirtualKey(100, 100,0 , LKGESTURE_RIGHT);
			}
	   }
	  else if(_tcscmp(misc, TEXT("RETURN")) == 0)
	   {
		 if(isSelectMode())
		 {
			 LKevent=LKEVENT_ENTER;
			 MapWindow::RefreshMap();
		 }
		 else if(IsMenuShown)
	  {
			   PlayResource(TEXT("IDR_WAV_CLICK"));
		  int thismode = getModeID();

		   int i;
		   unsigned MenuId = SelectedButtonId;
		   // Note - reverse order - last one wins
		   for (i = ModeLabel_count[thismode]; i >= 0; i--) {
			if ((ModeLabel[thismode][i].location == MenuId && MenuId>0) ) {

				int lastMode = thismode;
				// JMW need a debounce method here..

				if (ButtonLabel::IsEnabled(MenuId)) {


					processGo(ModeLabel[thismode][i].event);
				}

				// update button text, macro may change the label
				if ((lastMode == getModeID()) && (ModeLabel[thismode][i].label != NULL) && ButtonLabel::IsVisible(MenuId)){
					drawButtons(thismode);
				}
				break;
			}

		   }
	  }
		  else
		  {
			  PlayResource(TEXT("IDR_WAV_CLICK"));
			  eventZoom(_T("in"));

		  }

	   }

}
#else // LXMINIMAP

void InputEvents::eventMinimapKey(const TCHAR *misc) {
};

#endif // no LXMINIMAP

void InputEvents::eventInfoStripe(const TCHAR *misc) {
    if (_tcscmp(misc, TEXT("NEXT")) == 0) {
        BottomBarChange(true);
    } else if (_tcscmp(misc, TEXT("PREVIOUS")) == 0) {
        BottomBarChange(false);
    }
    BottomSounds();
    MapWindow::RefreshMap();
}

void InputEvents::eventInfoPage(const TCHAR *misc) {
    if (_tcscmp(misc, TEXT("NEXT")) == 0) {
        NextModeIndex();
    } else if (_tcscmp(misc, TEXT("PREVIOUS")) == 0) {
        PreviousModeIndex();
    }
    MapWindow::RefreshMap();
    SoundModeIndex();
}

void InputEvents::eventModeType(const TCHAR *misc) {
    if (_tcscmp(misc, TEXT("NEXT")) == 0) {
        NextModeType();
    } else if (_tcscmp(misc, TEXT("PREVIOUS")) == 0) {
        PreviousModeType();
    }
    MapWindow::RefreshMap();
}

void InputEvents::eventShowMultiselect(const TCHAR*) {
    dlgMultiSelectListShowModal();
}
