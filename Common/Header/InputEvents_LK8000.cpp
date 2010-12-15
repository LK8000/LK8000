/* AUTOMATICALLY GENERATED FILE - DO NOT EDIT BY HAND - see Common/Data/Input/xci2cpp.pl */
int event_id;
int mode_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventService, TEXT("TASKSTART"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_TASK_START] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventService, TEXT("TASKFINISH"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_TASK_FINISH] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventService, TEXT("TASKNEXTWAYPOINT"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_TASK_NEXTWAYPOINT] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventService, TEXT("TASKCONFIRMSTART"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_TASK_CONFIRMSTART] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventBeep, TEXT("1"), event_id);
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("In sector, arm advance when ready"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_ARM_READY] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("Waiting for GPS Connection"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_GPS_CONNECTION_WAIT] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("Restarting Comm Ports"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_COMMPORT_RESTART] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("Waiting for GPS Fix"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_GPS_FIX_WAIT] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventTaskLoad, TEXT("Default.tsk"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_STARTUP_SIMULATOR] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventTaskLoad, TEXT("Default.tsk"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_STARTUP_REAL] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventAutoLogger, TEXT("start"), event_id);
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("Takeoff"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_TAKEOFF] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("Landing"), event_id);
event_id = InputEvents::makeEvent(&eventAutoLogger, TEXT("stop"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_LANDING] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("Above Final Glide"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_FLIGHTMODE_FINALGLIDE_ABOVE] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("Below Final Glide"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_FLIGHTMODE_FINALGLIDE_BELOW] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("Final Glide Through Terrain"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
GC2Event[mode_id][GCE_FLIGHTMODE_FINALGLIDE_TERRAIN] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Bugs"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Ballast"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Wind"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("pan"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Bugs"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Ballast"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Wind"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("pan"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

mode_id = InputEvents::mode2int(TEXT("pan"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Bugs"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Ballast"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Wind"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("pan"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNull, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNull, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNull, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNull, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNull, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventPan, TEXT("down"), event_id);
mode_id = InputEvents::mode2int(TEXT("pan"), true);
Key2Event[mode_id][VK_DOWN] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventPan, TEXT("up"), event_id);
mode_id = InputEvents::mode2int(TEXT("pan"), true);
Key2Event[mode_id][VK_UP] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventPan, TEXT("left"), event_id);
mode_id = InputEvents::mode2int(TEXT("pan"), true);
Key2Event[mode_id][VK_LEFT] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventPan, TEXT("right"), event_id);
mode_id = InputEvents::mode2int(TEXT("pan"), true);
Key2Event[mode_id][VK_RIGHT] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventZoom, TEXT("in"), event_id);
mode_id = InputEvents::mode2int(TEXT("pan"), true);
makeLabel(mode_id,TEXT("Zoom\nin"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventZoom, TEXT("out"), event_id);
mode_id = InputEvents::mode2int(TEXT("pan"), true);
makeLabel(mode_id,TEXT("Zoom\nout"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNearestWaypointDetails, TEXT("pan"), event_id);
mode_id = InputEvents::mode2int(TEXT("pan"), true);
makeLabel(mode_id,TEXT("Nearest\nWaypoint"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventPan, TEXT("supertoggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("pan"), true);
makeLabel(mode_id,TEXT("Pan\n$(PanModeStatus)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("pan"), true);
makeLabel(mode_id,TEXT("Full\nScreen"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Nav1"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Info1"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Config1"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Display1"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventZoom, TEXT("out"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
Key2Event[mode_id][VK_DOWN] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventZoom, TEXT("in"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
Key2Event[mode_id][VK_UP] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventScreenModes, TEXT("toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
Key2Event[mode_id][VK_RETURN] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMarkLocation, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
Key2Event[mode_id][VK_LEFT] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventScreenModes, TEXT("toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
Key2Event[mode_id][VK_RIGHT] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventVisualGlide, TEXT("show"), event_id);
event_id = InputEvents::makeEvent(&eventVisualGlide, TEXT("toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("default"), true);
Key2Event[mode_id][VK_ESCAPE] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNull, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNull, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNull, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventNull, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventDoInfoKey, TEXT("up"), event_id);
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
Key2Event[mode_id][VK_UP] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventDoInfoKey, TEXT("down"), event_id);
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
Key2Event[mode_id][VK_DOWN] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventDoInfoKey, TEXT("left"), event_id);
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
Key2Event[mode_id][VK_LEFT] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventDoInfoKey, TEXT("right"), event_id);
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
Key2Event[mode_id][VK_RIGHT] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventDoInfoKey, TEXT("return"), event_id);
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
Key2Event[mode_id][VK_RETURN] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
makeLabel(mode_id,TEXT(""),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
makeLabel(mode_id,TEXT(""),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
makeLabel(mode_id,TEXT(""),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
makeLabel(mode_id,TEXT(""),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("infobox"), true);
makeLabel(mode_id,TEXT(""),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Nav2"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT("Nav\n1/3"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT("Cancel"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SIMMENU"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT("SIM\nMENU$(SIMONLY)"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventCalculator, TEXT(""), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT("Task\nCalc$(CheckTask)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventArmAdvance, TEXT("show"), event_id);
event_id = InputEvents::makeEvent(&eventArmAdvance, TEXT("toggle"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT("$(CheckTask)Advance\n&(AdvanceArmed)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventAdjustWaypoint, TEXT("previous"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT("$(WaypointPrevious)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventAdjustWaypoint, TEXT("next"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT("$(WaypointNext)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventWaypointDetails, TEXT("select"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav1"), true);
makeLabel(mode_id,TEXT("Waypoint\nLookup$(CheckWaypointFile)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Nav3"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT("Nav\n2/3"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT("Cancel"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSetup, TEXT("Task"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT("Task\nEdit$(CheckWaypointFile)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventAbortTask, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT("Task\nClear$(CheckTask)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventResetTask, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT("Task\nRestart$(RealTask)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSetup, TEXT("Target"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT("$(CheckTask)Target"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSetup, TEXT("Teamcode"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav2"), true);
makeLabel(mode_id,TEXT("Team\nCode"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Nav1"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav3"), true);
makeLabel(mode_id,TEXT("Nav\n3/3"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav3"), true);
makeLabel(mode_id,TEXT("Cancel"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav3"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Nav3"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventMarkLocation, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav3"), true);
makeLabel(mode_id,TEXT("Drop\nMarker"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventTimeGates, TEXT(""), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav3"), true);
makeLabel(mode_id,TEXT("Time\nGates"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("MTarget"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav3"), true);
makeLabel(mode_id,TEXT("Multi\nTarget"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventRun, TEXT("ext2"), event_id);
mode_id = InputEvents::mode2int(TEXT("Nav3"), true);
makeLabel(mode_id,TEXT("Reserv$(DISABLED)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Nav3"), true);
makeLabel(mode_id,TEXT("Reserv$(DISABLED)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT("Back"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT("Cancel"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Display2"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT("Display\n1/3"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventDeclutterLabels, TEXT("toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT("Labels\n$(MapLabelsToggleActionName)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventTerrainTopology, TEXT("topology toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT("Topology\n$(TopologyToggleActionName)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventTerrainTopology, TEXT("terrain toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT("Terrain\n$(TerrainToggleActionName)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventAirSpace, TEXT("toggle"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT("AirSpace\n$(AirSpaceToggleName)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventVisualGlide, TEXT("toggle"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display1"), true);
makeLabel(mode_id,TEXT("VisualGld\n$(VisualGlideToggleName)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT("Back"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT("Cancel"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Display3"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT("Display\n2/3"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventUserDisplayModeForce, TEXT("forceclimb"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT("DspMode\n$(DispModeClimbShortIndicator)Thermal&(DispModeClimbShortIndicator)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventUserDisplayModeForce, TEXT("forcecruise"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT("DspMode\n$(DispModeCruiseShortIndicator)Cruise&(DispModeCruiseShortIndicator)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventUserDisplayModeForce, TEXT("forcefinal"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT("DspMode\n$(DispModeFinalShortIndicator)Final&(DispModeFinalShortIndicator)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventUserDisplayModeForce, TEXT("unforce"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT("DspMode\n$(DispModeAutoShortIndicator)Auto&(DispModeAutoShortIndicator)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventFlightMode, TEXT("show"), event_id);
event_id = InputEvents::makeEvent(&eventFlightMode, TEXT("finalglide toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display2"), true);
makeLabel(mode_id,TEXT("$(CheckTask)Final\n&(FinalForceToggleActionName)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT("Back"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT("Cancel"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Display1"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT("Display\n3/3"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventZoom, TEXT("auto show"), event_id);
event_id = InputEvents::makeEvent(&eventZoom, TEXT("auto toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT("Zoom\n$(ZoomAutoToggleActionName)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSnailTrail, TEXT("show"), event_id);
event_id = InputEvents::makeEvent(&eventSnailTrail, TEXT("toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT("Trail\n$(SnailTrailToggleName)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventInvertColor, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT("Invert\nText"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeBack, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT("Topo\nBack$(TerrainVisible)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Orientation"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display3"), true);
makeLabel(mode_id,TEXT("Map\nOrient"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display4"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display4"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display4"), true);
makeLabel(mode_id,TEXT("Cancel"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Display4"), true);
makeLabel(mode_id,TEXT("Display\n4/4"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display4"), true);
makeLabel(mode_id,TEXT(""),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display4"), true);
makeLabel(mode_id,TEXT(""),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display4"), true);
makeLabel(mode_id,TEXT(""),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display4"), true);
makeLabel(mode_id,TEXT(""),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Display4"), true);
makeLabel(mode_id,TEXT(""),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT("Back"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Config2"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT("Config\n1/3"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSetup, TEXT("Basic"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT("Setup\nBasic"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSetup, TEXT("Wind"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT("Setup\nWind"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventActiveMap, TEXT("show"), event_id);
event_id = InputEvents::makeEvent(&eventActiveMap, TEXT("toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT("ActivMap\n$(ActiveMap)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventAirspaceDisplayMode, TEXT("toggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT("Airspace\n$(AirspaceMode)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventBaroAltitude, TEXT("show"), event_id);
event_id = InputEvents::makeEvent(&eventBaroAltitude, TEXT("toggle"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config1"), true);
makeLabel(mode_id,TEXT("Nav by\n$(TOGGLEHBAR)&(HBARAVAILABLE)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT("Back"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Config3"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT("Config\n2/3"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSetup, TEXT("System"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT("Setup\nSystem$(CheckSettingsLockout)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventStatusMessage, TEXT("Marks cleared"), event_id);
event_id = InputEvents::makeEvent(&eventMarkLocation, TEXT("reset"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT("Clear\nMarks"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventLogger, TEXT("show"), event_id);
event_id = InputEvents::makeEvent(&eventLogger, TEXT("toggle ask"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT("$(OnlyInFly)Logger\n&(LoggerActive)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSetup, TEXT("Replay"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT("$(OnlyInSim)Logger\nReplay&(CheckReplay)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventLogger, TEXT("nmea"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config2"), true);
makeLabel(mode_id,TEXT("$(OnlyInFly)NMEA\nLogger"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT("Back"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Config1"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT("Config\n3/3"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SetupFlarm"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT("$(CheckFLARM)FLARM\nSetup&(OnlyInFly)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventResetQFE, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT("Zero\nQFE"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventRestartCommPorts, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT("Reset\nComms$(OnlyInFly)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventService, TEXT("PROFILES"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT("Profiles"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSounds, TEXT("show"), event_id);
event_id = InputEvents::makeEvent(&eventSounds, TEXT("toggle"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Config3"), true);
makeLabel(mode_id,TEXT("Sounds\n$(EnableSoundModes)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT("Back"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Info2"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT("Info\n1/2"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT("Cancel"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventWaypointDetails, TEXT("current"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT("$(CheckTask)Waypoint\nDetails"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventNearestWaypointDetails, TEXT("aircraft"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT("$(CheckWaypointFile)Nearest\nWaypoint"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventNearestAirspaceDetails, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT("Nearest\nAirspace$(CheckAirspace)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventStatus, TEXT("all"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT("Status"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventAnalysis, TEXT(""), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info1"), true);
makeLabel(mode_id,TEXT("Analysis"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT("Back"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Info1"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT("Info\n2/2"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT("Cancel"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT(""),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChecklist, TEXT(""), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT("Notepad"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSetup, TEXT("Weather"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT("Weather"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventRepeatStatusMessage, TEXT(""), event_id);
mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT("Message\nRepeat"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventScreenModes, TEXT("toggleauxiliary"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT("$(BoxMode)Box Aux\n&(AuxInfoToggleActionName)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventService, TEXT("ORBITER"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Info2"), true);
makeLabel(mode_id,TEXT("Orbiter\n$(Orbiter)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT("Menu"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT(""),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT(""),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT(""),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT(""),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Exit"), true);
makeLabel(mode_id,TEXT(""),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Nav1"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Nav"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Info1"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Info"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Config1"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Config"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Display1"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Display"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventExit, TEXT("system"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Exit"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("TrueWind"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("TrueWind\nCalc"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("MC"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Mc ($(MacCreadyMode))\n&(MacCreadyValue)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("ScreenMode"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Screen\nViews"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Cancel"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("MC"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("MC"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("MC"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMacCready, TEXT("auto show"), event_id);
event_id = InputEvents::makeEvent(&eventMacCready, TEXT("auto toggle"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MC"), true);
makeLabel(mode_id,TEXT("$(CheckAutoMc)Mc\n&(MacCreadyToggleActionName)"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMacCready, TEXT("5down"), event_id);
mode_id = InputEvents::mode2int(TEXT("MC"), true);
makeLabel(mode_id,TEXT("Mc\n-0.5"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMacCready, TEXT("down"), event_id);
mode_id = InputEvents::mode2int(TEXT("MC"), true);
makeLabel(mode_id,TEXT("Mc\n-0.1"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMacCready, TEXT("up"), event_id);
mode_id = InputEvents::mode2int(TEXT("MC"), true);
makeLabel(mode_id,TEXT("Mc\n+0.1"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMacCready, TEXT("5up"), event_id);
mode_id = InputEvents::mode2int(TEXT("MC"), true);
makeLabel(mode_id,TEXT("Mc\n+0.5"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MC"), true);
makeLabel(mode_id,TEXT("OK\n$(MacCreadyValue)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventService, TEXT("OVERLAYS"), event_id);
mode_id = InputEvents::mode2int(TEXT("ScreenMode"), true);
makeLabel(mode_id,TEXT("Overlays\n$(OVERLAY)"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("ScreenMode"), true);
makeLabel(mode_id,TEXT("Back"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("ScreenMode"), true);
makeLabel(mode_id,TEXT("Cancel"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventService, TEXT("SHADING"), event_id);
mode_id = InputEvents::mode2int(TEXT("ScreenMode"), true);
makeLabel(mode_id,TEXT("Shading\n$(SHADING)"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventZoom, TEXT("in"), event_id);
mode_id = InputEvents::mode2int(TEXT("ScreenMode"), true);
makeLabel(mode_id,TEXT("Zoom\nIn"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventZoom, TEXT("out"), event_id);
mode_id = InputEvents::mode2int(TEXT("ScreenMode"), true);
makeLabel(mode_id,TEXT("Zoom\nout"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Center"), event_id);
mode_id = InputEvents::mode2int(TEXT("ScreenMode"), true);
makeLabel(mode_id,TEXT("Set\nMap"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventPan, TEXT("supertoggle"), event_id);
mode_id = InputEvents::mode2int(TEXT("ScreenMode"), true);
makeLabel(mode_id,TEXT("PAN\n$(PanModeStatus)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
event_id = InputEvents::makeEvent(&eventScreenModes, TEXT("togglefull"), event_id);
mode_id = InputEvents::mode2int(TEXT("ScreenMode"), true);
makeLabel(mode_id,TEXT("IBOX\n$(FullScreenToggleActionName)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Orientation"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("Orientation"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Display3"), event_id);
mode_id = InputEvents::mode2int(TEXT("Orientation"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Orientation"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventOrientation, TEXT("northup"), event_id);
mode_id = InputEvents::mode2int(TEXT("Orientation"), true);
makeLabel(mode_id,TEXT("North\nup"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventOrientation, TEXT("trackup"), event_id);
mode_id = InputEvents::mode2int(TEXT("Orientation"), true);
makeLabel(mode_id,TEXT("Track\nup"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventOrientation, TEXT("northcircle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Orientation"), true);
makeLabel(mode_id,TEXT("North\ncircle"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventOrientation, TEXT("trackcircle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Orientation"), true);
makeLabel(mode_id,TEXT("Target\ncircle"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventOrientation, TEXT("northsmart"), event_id);
mode_id = InputEvents::mode2int(TEXT("Orientation"), true);
makeLabel(mode_id,TEXT("North\nsmart"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAR,0"), event_id);
mode_id = InputEvents::mode2int(TEXT("SetupFlarm"), true);
makeLabel(mode_id,TEXT("REBOOT\nFLARM"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,NMEAOUT,1"), event_id);
mode_id = InputEvents::mode2int(TEXT("SetupFlarm"), true);
makeLabel(mode_id,TEXT("Normal\nNmea"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Config3"), event_id);
mode_id = InputEvents::mode2int(TEXT("SetupFlarm"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("SetupFlarm"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,CFLAGS,0"), event_id);
mode_id = InputEvents::mode2int(TEXT("SetupFlarm"), true);
makeLabel(mode_id,TEXT("Normal\nFlags"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("FlarmSpeed"), event_id);
mode_id = InputEvents::mode2int(TEXT("SetupFlarm"), true);
makeLabel(mode_id,TEXT("Baud\nRate"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("FlarmSig"), event_id);
mode_id = InputEvents::mode2int(TEXT("SetupFlarm"), true);
makeLabel(mode_id,TEXT("Leds and\nSounds"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("FlarmStealth"), event_id);
mode_id = InputEvents::mode2int(TEXT("SetupFlarm"), true);
makeLabel(mode_id,TEXT("Stealth\nModes"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("FlarmRange"), event_id);
mode_id = InputEvents::mode2int(TEXT("SetupFlarm"), true);
makeLabel(mode_id,TEXT("Radio\nRange"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmRange"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmRange"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SetupFlarm"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmRange"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmRange"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,RANGE,2000"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmRange"), true);
makeLabel(mode_id,TEXT("Lowest\n2km"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,RANGE,3000"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmRange"), true);
makeLabel(mode_id,TEXT("Default\n3km"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,RANGE,10000"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmRange"), true);
makeLabel(mode_id,TEXT("Averag\n10km"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,RANGE,15000"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmRange"), true);
makeLabel(mode_id,TEXT("Averag\n15km"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,RANGE,25500"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmRange"), true);
makeLabel(mode_id,TEXT("Highest\n25km"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmStealth"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmStealth"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SetupFlarm"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmStealth"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmStealth"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmStealth"), true);
makeLabel(mode_id,TEXT("Reserv$(DISABLED)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,PRIV,0"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmStealth"), true);
makeLabel(mode_id,TEXT("Stealth\nOFF"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,PRIV,1"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmStealth"), true);
makeLabel(mode_id,TEXT("Stealth\nON"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmStealth"), true);
makeLabel(mode_id,TEXT("Reserv$(DISABLED)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmStealth"), true);
makeLabel(mode_id,TEXT("Reserv$(DISABLED)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmSpeed"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmSpeed"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SetupFlarm"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSpeed"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSpeed"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,BAUD,0"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSpeed"), true);
makeLabel(mode_id,TEXT("Baud\n4800"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,BAUD,1"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSpeed"), true);
makeLabel(mode_id,TEXT("Baud\n9600"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,BAUD,2"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSpeed"), true);
makeLabel(mode_id,TEXT("Baud\n19200"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,BAUD,4"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSpeed"), true);
makeLabel(mode_id,TEXT("Baud\n38400"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,BAUD,5"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSpeed"), true);
makeLabel(mode_id,TEXT("Baud\n57600"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmSig"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmSig"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SetupFlarm"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSig"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSig"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("FlarmSig"), true);
makeLabel(mode_id,TEXT("Radar\nMode$(DISABLED)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,UI,0"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSig"), true);
makeLabel(mode_id,TEXT("Normal\nALL ON"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,UI,1"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSig"), true);
makeLabel(mode_id,TEXT("Led+Buz\nALL OFF"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,UI,2"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSig"), true);
makeLabel(mode_id,TEXT("Led OFF\nBuz ON"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSendNMEAPort1, TEXT("PFLAC,S,UI,3"), event_id);
mode_id = InputEvents::mode2int(TEXT("FlarmSig"), true);
makeLabel(mode_id,TEXT("Led ON\nBuz OFF"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("TrueWind"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("TrueWind"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("TrueWind"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Menu"), event_id);
mode_id = InputEvents::mode2int(TEXT("TrueWind"), true);
makeLabel(mode_id,TEXT("Back"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("TWSpeed"), event_id);
mode_id = InputEvents::mode2int(TEXT("TrueWind"), true);
makeLabel(mode_id,TEXT("IAS\n$(WCSpeed)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventCalcWind, TEXT("C0"), event_id);
mode_id = InputEvents::mode2int(TEXT("TrueWind"), true);
makeLabel(mode_id,TEXT("N    E\nW   S$(CheckFlying)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventCalcWind, TEXT("C1"), event_id);
mode_id = InputEvents::mode2int(TEXT("TrueWind"), true);
makeLabel(mode_id,TEXT("3    12\n30   21$(CheckFlying)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventCalcWind, TEXT("C2"), event_id);
mode_id = InputEvents::mode2int(TEXT("TrueWind"), true);
makeLabel(mode_id,TEXT("6    15\n33   24$(CheckFlying)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("TrueWind"), true);
makeLabel(mode_id,TEXT("Cancel"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("TWSpeed"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("TWSpeed"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("TWSpeed"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("TrueWind"), event_id);
mode_id = InputEvents::mode2int(TEXT("TWSpeed"), true);
makeLabel(mode_id,TEXT("Back"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeWindCalcSpeed, TEXT("10down"), event_id);
mode_id = InputEvents::mode2int(TEXT("TWSpeed"), true);
makeLabel(mode_id,TEXT("IAS\n-10"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeWindCalcSpeed, TEXT("5down"), event_id);
mode_id = InputEvents::mode2int(TEXT("TWSpeed"), true);
makeLabel(mode_id,TEXT("IAS\n-5"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeWindCalcSpeed, TEXT("5up"), event_id);
mode_id = InputEvents::mode2int(TEXT("TWSpeed"), true);
makeLabel(mode_id,TEXT("IAS\n+5"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeWindCalcSpeed, TEXT("10up"), event_id);
mode_id = InputEvents::mode2int(TEXT("TWSpeed"), true);
makeLabel(mode_id,TEXT("IAS\n+10"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("TrueWind"), event_id);
mode_id = InputEvents::mode2int(TEXT("TWSpeed"), true);
makeLabel(mode_id,TEXT("OK\n$(WCSpeed)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventOrientation, TEXT("northup"), event_id);
mode_id = InputEvents::mode2int(TEXT("Center"), true);
makeLabel(mode_id,TEXT("North\nUp"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventOrientation, TEXT("northcircle"), event_id);
mode_id = InputEvents::mode2int(TEXT("Center"), true);
makeLabel(mode_id,TEXT("Track\nUp"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventOrientation, TEXT("northsmart"), event_id);
mode_id = InputEvents::mode2int(TEXT("Center"), true);
makeLabel(mode_id,TEXT("North\nSmart"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Center"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMoveGlider, TEXT("left"), event_id);
mode_id = InputEvents::mode2int(TEXT("Center"), true);
makeLabel(mode_id,TEXT("<< Left$(NoSmart)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMoveGlider, TEXT("up"), event_id);
mode_id = InputEvents::mode2int(TEXT("Center"), true);
makeLabel(mode_id,TEXT("UP$(NoSmart)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMoveGlider, TEXT("reset"), event_id);
mode_id = InputEvents::mode2int(TEXT("Center"), true);
makeLabel(mode_id,TEXT("Center\nDefault$(NoSmart)"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMoveGlider, TEXT("down"), event_id);
mode_id = InputEvents::mode2int(TEXT("Center"), true);
makeLabel(mode_id,TEXT("\nDOWN$(NoSmart)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMoveGlider, TEXT("right"), event_id);
mode_id = InputEvents::mode2int(TEXT("Center"), true);
makeLabel(mode_id,TEXT("Right >>$(NoSmart)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventProfileLoad, TEXT("PROFILE3.prf"), event_id);
mode_id = InputEvents::mode2int(TEXT("Profile"), true);
makeLabel(mode_id,TEXT("Load\nProfile3"),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventProfileSave, TEXT("PROFILE3.prf"), event_id);
mode_id = InputEvents::mode2int(TEXT("Profile"), true);
makeLabel(mode_id,TEXT("Save\nProfile3"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("Config3"), event_id);
mode_id = InputEvents::mode2int(TEXT("Profile"), true);
makeLabel(mode_id,TEXT("Back"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Profile"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventProfileLoad, TEXT("PROFILE1.prf"), event_id);
mode_id = InputEvents::mode2int(TEXT("Profile"), true);
makeLabel(mode_id,TEXT("Load\nProfile1"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventProfileSave, TEXT("PROFILE1.prf"), event_id);
mode_id = InputEvents::mode2int(TEXT("Profile"), true);
makeLabel(mode_id,TEXT("Save\nProfile1"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventProfileLoad, TEXT("PROFILE2.prf"), event_id);
mode_id = InputEvents::mode2int(TEXT("Profile"), true);
makeLabel(mode_id,TEXT("Load\nProfile2"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventProfileSave, TEXT("PROFILE2.prf"), event_id);
mode_id = InputEvents::mode2int(TEXT("Profile"), true);
makeLabel(mode_id,TEXT("Save\nProfile2"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventProfileLoad, TEXT("Factory"), event_id);
mode_id = InputEvents::mode2int(TEXT("Profile"), true);
makeLabel(mode_id,TEXT("Reset\nFactory$(DISABLED)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("MTarget"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeMultitarget, TEXT("FLARM"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MTarget"), true);
makeLabel(mode_id,TEXT("$(CheckFLARM)F>\nFlarm"),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeMultitarget, TEXT("MATE"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MTarget"), true);
makeLabel(mode_id,TEXT("M>\nTeam"),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeMultitarget, TEXT("THER"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MTarget"), true);
makeLabel(mode_id,TEXT("L>\nThermal"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeMultitarget, TEXT("TASK"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MTarget"), true);
makeLabel(mode_id,TEXT("T>\nTask"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeMultitarget, TEXT("BALT"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MTarget"), true);
makeLabel(mode_id,TEXT("B>\nBestAlt"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeMultitarget, TEXT("ALT1"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MTarget"), true);
makeLabel(mode_id,TEXT("1>\nAltern1"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeMultitarget, TEXT("ALT2"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MTarget"), true);
makeLabel(mode_id,TEXT("2>\nAltern2"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeMultitarget, TEXT("HOME"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("MTarget"), true);
makeLabel(mode_id,TEXT("H>\nHome"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMMENU"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMMENU"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMMENU"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMMENU"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMMENU"), true);
makeLabel(mode_id,TEXT(">Speed<\n$(GS)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SIMALT"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMMENU"), true);
makeLabel(mode_id,TEXT("Alt\n$(HGPS)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SIMDIR"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMMENU"), true);
makeLabel(mode_id,TEXT("Turn\n$(TURN)°/s"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeGS, TEXT("down"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMMENU"), true);
makeLabel(mode_id,TEXT("\n-$(NotInReplay)&(OnlyInSim)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeGS, TEXT("up"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMMENU"), true);
makeLabel(mode_id,TEXT("\n+$(NotInReplay)&(OnlyInSim)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMALT"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMALT"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMALT"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMALT"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SIMMENU"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMALT"), true);
makeLabel(mode_id,TEXT("Speed\n$(GS)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMALT"), true);
makeLabel(mode_id,TEXT(">Alt<\n$(HGPS)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SIMDIR"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMALT"), true);
makeLabel(mode_id,TEXT("Turn\n$(TURN)°/s"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeHGPS, TEXT("down"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMALT"), true);
makeLabel(mode_id,TEXT("\n-$(NotInReplay)&(OnlyInSim)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeHGPS, TEXT("up"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMALT"), true);
makeLabel(mode_id,TEXT("\n+$(NotInReplay)&(OnlyInSim)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMDIR"), true);
makeLabel(mode_id,TEXT(""),1,event_id);
Key2Event[mode_id][VK_APP1] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMDIR"), true);
makeLabel(mode_id,TEXT(""),2,event_id);
Key2Event[mode_id][VK_APP2] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMDIR"), true);
makeLabel(mode_id,TEXT(""),3,event_id);
Key2Event[mode_id][VK_APP3] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMDIR"), true);
makeLabel(mode_id,TEXT("Cancel"),4,event_id);
Key2Event[mode_id][VK_APP4] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SIMMENU"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMDIR"), true);
makeLabel(mode_id,TEXT("Speed\n$(GS)"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("SIMALT"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMDIR"), true);
makeLabel(mode_id,TEXT("Alt\n$(HGPS)"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
mode_id = InputEvents::mode2int(TEXT("SIMDIR"), true);
makeLabel(mode_id,TEXT(">Turn<\n$(TURN)°/s"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeTurn, TEXT("left"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMDIR"), true);
makeLabel(mode_id,TEXT("\n<<$(NotInReplay)&(OnlyInSim)"),8,event_id);
Key2Event[mode_id]['9'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventChangeTurn, TEXT("right"), event_id);
mode_id = InputEvents::mode2int(TEXT("SIMDIR"), true);
makeLabel(mode_id,TEXT("\n>>$(NotInReplay)&(OnlyInSim)"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

