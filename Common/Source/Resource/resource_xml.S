/*
 * LK8000 Tactical Flight Computer - WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File: resource_xml.S
 * Author: Bruno de Lacheisserie
 * 
 * Created on 18 septembre 2014, 13:34
 */

.macro binfile p q
    .globl \p\()_begin
    .globl \p\()_end
    .globl \p\()_size
\p\()_begin:
    .incbin "\q"
\p\()_end:
    /*
    Put a ".byte 0" here if you know  your data is text
    and you wish to use \p\()_begin as a C string. It 
    doesn't hurt to leave it here even for binary data
    since it is not counted in \p_&size
    */
    .byte 0
    .byte 0
\p\()_size:
    .int (\p\()_end - \p\()_begin)
    /* add alias for "_" decorated symname for avoid link error. */
    .globl _\p\()_begin
    .set _\p\()_begin, \p\()_begin
    .globl _\p\()_end
    .set _\p\()_end, \p\()_end
    .globl _\p\()_size
    .set _\p\()_size, \p\()_size

.endm	

    .section .rodata

    /* XMLDIALOG */
    binfile IDR_XML_MULTISELECTLIST_L, Common/Data/Dialogs/dlgMultiSelectList_L.xml
    binfile IDR_XML_MULTISELECTLIST_P, Common/Data/Dialogs/dlgMultiSelectList_P.xml

    binfile IDR_XML_FLYSIM_L, Common/Data/Dialogs/dlgFlySim_L.xml
    binfile IDR_XML_FLYSIM_P, Common/Data/Dialogs/dlgFlySim_P.xml

    binfile IDR_XML_AIRSPACE_L, Common/Data/Dialogs/dlgAirspace_L.xml
    binfile IDR_XML_AIRSPACE_P, Common/Data/Dialogs/dlgAirspace_P.xml

    binfile IDR_XML_AIRSPACECOLOURS_L, Common/Data/Dialogs/dlgAirspaceColours_L.xml
    binfile IDR_XML_AIRSPACECOLOURS_P, Common/Data/Dialogs/dlgAirspaceColours_P.xml

    binfile IDR_XML_AIRSPACEPATTERNS_L, Common/Data/Dialogs/dlgAirspacePatterns_L.xml
    binfile IDR_XML_AIRSPACEPATTERNS_P, Common/Data/Dialogs/dlgAirspacePatterns_P.xml

    binfile IDR_XML_ANALYSIS_L, Common/Data/Dialogs/dlgAnalysis_L.xml
    binfile IDR_XML_ANALYSIS_P, Common/Data/Dialogs/dlgAnalysis_P.xml

    binfile IDR_XML_BASICSETTINGS_L, Common/Data/Dialogs/dlgBasicSettings_L.xml
    binfile IDR_XML_BASICSETTINGS_P, Common/Data/Dialogs/dlgBasicSettings_P.xml

    binfile IDR_XML_CHECKLIST_L, Common/Data/Dialogs/dlgChecklist_L.xml
    binfile IDR_XML_CHECKLIST_P, Common/Data/Dialogs/dlgChecklist_P.xml

    binfile IDR_XML_STARTUP_L, Common/Data/Dialogs/dlgStartup_L.xml
    binfile IDR_XML_STARTUP_P, Common/Data/Dialogs/dlgStartup_P.xml

    binfile IDR_XML_DUALPROFILE_L, Common/Data/Dialogs/dlgDualProfile_L.xml
    binfile IDR_XML_DUALPROFILE_P, Common/Data/Dialogs/dlgDualProfile_P.xml

    binfile IDR_XML_ORACLE_L, Common/Data/Dialogs/dlgOracle_L.xml
    binfile IDR_XML_ORACLE_P, Common/Data/Dialogs/dlgOracle_P.xml

    binfile IDR_XML_TASKOVERVIEW_L, Common/Data/Dialogs/dlgTaskOverview_L.xml
    binfile IDR_XML_TASKOVERVIEW_P, Common/Data/Dialogs/dlgTaskOverview_P.xml

    binfile IDR_XML_TASKWAYPOINT_L, Common/Data/Dialogs/dlgTaskWaypoint_L.xml
    binfile IDR_XML_TASKWAYPOINT_P, Common/Data/Dialogs/dlgTaskWaypoint_P.xml

    binfile IDR_XML_WAYPOINTDETAILS_L, Common/Data/Dialogs/dlgWayPointDetails_L.xml
    binfile IDR_XML_WAYPOINTDETAILS_P, Common/Data/Dialogs/dlgWayPointDetails_P.xml

    binfile IDR_XML_WAYPOINTQUICK_L, Common/Data/Dialogs/dlgWayQuick_L.xml
    binfile IDR_XML_WAYPOINTQUICK_P, Common/Data/Dialogs/dlgWayQuick_P.xml

    binfile IDR_XML_TEXTENTRY_KEYBOARD_L, Common/Data/Dialogs/dlgTextEntry_Keyboard_L.xml
    binfile IDR_XML_TEXTENTRY_KEYBOARD_P, Common/Data/Dialogs/dlgTextEntry_Keyboard_P.xml

    binfile IDR_XML_NUMENTRY_KEYBOARD_L, Common/Data/Dialogs/dlgNumEntry_Keyboard_L.xml
    binfile IDR_XML_NUMENTRY_KEYBOARD_P, Common/Data/Dialogs/dlgNumEntry_Keyboard_P.xml

    binfile IDR_XML_STARTPOINT_L, Common/Data/Dialogs/dlgStartPoint_L.xml
    binfile IDR_XML_STARTPOINT_P, Common/Data/Dialogs/dlgStartPoint_P.xml

    binfile IDR_XML_WAYPOINTEDIT_L, Common/Data/Dialogs/dlgWaypointEdit_L.xml
    binfile IDR_XML_WAYPOINTEDIT_P, Common/Data/Dialogs/dlgWaypointEdit_P.xml

    binfile IDR_XML_TARGET_L, Common/Data/Dialogs/dlgTarget_L.xml
    binfile IDR_XML_TARGET_P, Common/Data/Dialogs/dlgTarget_P.xml

    binfile IDR_XML_LKAIRSPACEWARNING_L, Common/Data/Dialogs/dlgLKAirspaceWarning_L.xml
    binfile IDR_XML_LKAIRSPACEWARNING_P, Common/Data/Dialogs/dlgLKAirspaceWarning_P.xml

    binfile IDR_XML_HELP_L, Common/Data/Dialogs/dlgHelp_L.xml
    binfile IDR_XML_HELP_P, Common/Data/Dialogs/dlgHelp_P.xml

    binfile IDR_XML_SELECTOBJECT_P, Common/Data/Dialogs/dlgSelectObject_P.xml
    binfile IDR_XML_SELECTOBJECT_L, Common/Data/Dialogs/dlgSelectObject_L.xml

    binfile IDR_XML_COMBOPICKER_L, Common/Data/Dialogs/dlgComboPicker_L.xml
    binfile IDR_XML_COMBOPICKER_P, Common/Data/Dialogs/dlgComboPicker_P.xml

    binfile IDR_XML_CONFIGURATION_L, Common/Data/Dialogs/dlgConfiguration_L.xml
    binfile IDR_XML_CONFIGURATION_P, Common/Data/Dialogs/dlgConfiguration_P.xml

    binfile IDR_XML_CONFIGAIRCRAFT_L, Common/Data/Dialogs/dlgConfigAircraft_L.xml
    binfile IDR_XML_CONFIGAIRCRAFT_P, Common/Data/Dialogs/dlgConfigAircraft_P.xml

    binfile IDR_XML_CONFIGPILOT_L, Common/Data/Dialogs/dlgConfigPilot_L.xml
    binfile IDR_XML_CONFIGPILOT_P, Common/Data/Dialogs/dlgConfigPilot_P.xml

    binfile IDR_XML_CONFIGDEVICE_L, Common/Data/Dialogs/dlgConfigDevice_L.xml
    binfile IDR_XML_CONFIGDEVICE_P, Common/Data/Dialogs/dlgConfigDevice_P.xml

    binfile IDR_XML_WAYPOINTEDITUTM_L, Common/Data/Dialogs/dlgWaypointEditUTM_L.xml
    binfile IDR_XML_WAYPOINTEDITUTM_P, Common/Data/Dialogs/dlgWaypointEditUTM_P.xml

    binfile IDR_XML_OVERLAYS, Common/Data/Dialogs/dlgOverlays.xml
    binfile IDR_XML_MULTIMAPS, Common/Data/Dialogs/dlgMultimaps.xml
    binfile IDR_XML_DEVCPROBE, Common/Data/Dialogs/dlgDevCProbe.xml

    binfile IDR_XML_BLUETOOTH_L, Common/Data/Dialogs/dlgBluetooth_L.xml
    binfile IDR_XML_BLUETOOTH_P, Common/Data/Dialogs/dlgBluetooth_P.xml

    binfile IDR_XML_IGCFILE_L, Common/Data/Dialogs/dlgIgcFile_L.xml
    binfile IDR_XML_IGCFILE_P, Common/Data/Dialogs/dlgIgcFile_P.xml

    binfile IDR_XML_BLUEFLYCONFIG_L, Common/Data/Dialogs/dlgBlueFlyConfig_L.xml
    binfile IDR_XML_BLUEFLYCONFIG_P, Common/Data/Dialogs/dlgBlueFlyConfig_P.xml

    binfile IDR_XML_TERMINAL_L , Common/Data/Dialogs/dlgTerminal_L.xml
    binfile IDR_XML_TERMINAL_P , Common/Data/Dialogs/dlgTerminal_P.xml

    binfile IDR_XML_PROGRESS_L, Common/Data/Dialogs/dlgProgress_L.xml
    binfile IDR_XML_PROGRESS_P, Common/Data/Dialogs/dlgProgress_P.xml
    binfile IDR_XML_RADIOSETTINGS , Common/Data/Dialogs/dlgRadioSettings.xml
    binfile IDR_XML_DEVFLARM, Common/Data/Dialogs/dlgDevFlarm.xml
    binfile IDR_XML_LOGGERREPLAY, Common/Data/Dialogs/dlgLoggerReplay.xml
    binfile IDR_XML_STATUS, Common/Data/Dialogs/dlgStatus.xml
    binfile IDR_XML_TASKCALCULATOR, Common/Data/Dialogs/dlgTaskCalculator.xml
    binfile IDR_XML_WINDSETTINGS, Common/Data/Dialogs/dlgWindSettings.xml
    binfile IDR_XML_STARTTASK, Common/Data/Dialogs/dlgStartTask.xml
    binfile IDR_XML_TIMEGATES, Common/Data/Dialogs/dlgTimeGates.xml
    binfile IDR_XML_TOPOLOGY, Common/Data/Dialogs/dlgTopology.xml
    binfile IDR_XML_CUSTOMMENU, Common/Data/Dialogs/dlgCustomMenu.xml
    binfile IDR_XML_CUSTOMKEYS, Common/Data/Dialogs/dlgCustomKeys.xml
    binfile IDR_XML_BOTTOMBAR, Common/Data/Dialogs/dlgBottomBar.xml
    binfile IDR_XML_INFOPAGES, Common/Data/Dialogs/dlgInfoPages.xml
    binfile IDR_XML_WAYPOINTTERRAIN, Common/Data/Dialogs/dlgWaypointOutOfTerrain.xml
    binfile IDR_XML_TEAMCODE, Common/Data/Dialogs/dlgTeamCode.xml
    binfile IDR_XML_TASKRULES, Common/Data/Dialogs/dlgTaskRules.xml
    binfile IDR_XML_AIRSPACEDETAILS, Common/Data/Dialogs/dlgAirspaceDetails.xml
    binfile IDR_XML_LKTRAFFICDETAILS, Common/Data/Dialogs/dlgLKTraffic.xml
    binfile IDR_XML_THERMALDETAILS, Common/Data/Dialogs/dlgThermalDetails.xml
    binfile IDR_XML_PROFILES, Common/Data/Dialogs/dlgProfiles.xml
    binfile IDR_XML_AIRSPACEWARNINGPARAMS, Common/Data/Dialogs/dlgAirspaceWarningParams.xml
    binfile IDR_XML_FILES_L, Common/Data/Dialogs/dlgFiles_L.xml
    binfile IDR_XML_FILES_P, Common/Data/Dialogs/dlgFiles_P.xml
    binfile IDR_XML_IGC_PROGRESS_P, Common/Data/Dialogs/dlgIGCProgress_L.xml
    binfile IDR_XML_IGC_PROGRESS_L, Common/Data/Dialogs/dlgIGCProgress_P.xml
    binfile IDR_XML_DEV_LXNAV_L, Common/Data/Dialogs/dlgDevLXNav_L.xml
    binfile IDR_XML_DEV_LXNAV_P, Common/Data/Dialogs/dlgDevLXNav_P.xml
    binfile IDR_XML_WEATHERSTDETAILS, Common/Data/Dialogs/dlgWeatherSt.xml
    binfile IDR_XML_NMEAREPLAY, Common/Data/Dialogs/dlgNMEAReplay.xml
    binfile IDR_XML_RADIOPRISECSEL, Common/Data/Dialogs/dlgRadioPriSecSel.xml
