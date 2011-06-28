/* AUTOMATICALLY GENERATED FILE - DO NOT EDIT BY HAND - see Common/Data/Input/xci2cpp.pl */
int event_id;
int mode_id;

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
event_id = InputEvents::makeEvent(&eventExit, TEXT("system"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Exit"),5,event_id);
Key2Event[mode_id]['6'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventSetup, TEXT("System"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Setup\nSystem"),6,event_id);
Key2Event[mode_id]['7'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventService, TEXT("PROFILES"), event_id);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Profiles"),7,event_id);
Key2Event[mode_id]['8'] = event_id;

event_id = 0;
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Cancel"),9,event_id);
Key2Event[mode_id]['0'] = event_id;

