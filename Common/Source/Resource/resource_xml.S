/*
 * LK8000 Tactical Flight Computer - WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File: resource_xml.S
 * Author: Bruno de Lacheisserie
 * 
 * Created on 18 septembre 2014, 13:34
 */

.altmacro
.macro binfile p q
    .globl \p&_begin
    .globl \p&_end
    .globl \p&_size
\p&_begin:
    .incbin \q
\p&_end:
    /*
    Put a ".byte 0" here if you know  your data is text
    and you wish to use \p&_begin as a C string. It 
    doesn't hurt to leave it here even for binary data
    since it is not counted in \p_&size
    */
    .byte 0
\p&_size:
    .int (\p&_end - \p&_begin)
    /* add alias for "_" decorated symname for avoid link error. */
    .globl _\p&_begin
    .set _\p&_begin, \p&_begin
    .globl _\p&_end
    .set _\p&_end, \p&_end
    .globl _\p&_size
    .set _\p&_size, \p&_size

.endm	

    .section .rodata

    /* XMLDIALOG */
    binfile IDR_XML_MULTISELECTLIST_L "../../Data/Dialogs/dlgMultiSelectList_L.xml"
    binfile IDR_XML_MULTISELECTLIST_P "../../Data/Dialogs/dlgMultiSelectList_P.xml"

    binfile IDR_XML_FLYSIM_L "../../Data/Dialogs/dlgFlySim_L.xml"
    binfile IDR_XML_FLYSIM_P "../../Data/Dialogs/dlgFlySim_P.xml"

    binfile IDR_XML_AIRSPACE_L "../../Data/Dialogs/dlgAirspace_L.xml"
    binfile IDR_XML_AIRSPACE_P "../../Data/Dialogs/dlgAirspace_P.xml"

    binfile IDR_XML_AIRSPACECOLOURS_L "../../Data/Dialogs/dlgAirspaceColours_L.xml"
    binfile IDR_XML_AIRSPACECOLOURS_P "../../Data/Dialogs/dlgAirspaceColours_P.xml"

    binfile IDR_XML_AIRSPACEPATTERNS_L "../../Data/Dialogs/dlgAirspacePatterns_L.xml"
    binfile IDR_XML_AIRSPACEPATTERNS_P "../../Data/Dialogs/dlgAirspacePatterns_P.xml"

    binfile IDR_XML_ANALYSIS_L "../../Data/Dialogs/dlgAnalysis_L.xml"
    binfile IDR_XML_ANALYSIS_P "../../Data/Dialogs/dlgAnalysis_P.xml"

    binfile IDR_XML_BASICSETTINGS_L "../../Data/Dialogs/dlgBasicSettings_L.xml"
    binfile IDR_XML_BASICSETTINGS_P "../../Data/Dialogs/dlgBasicSettings_P.xml"

    binfile IDR_XML_CHECKLIST_L "../../Data/Dialogs/dlgChecklist_L.xml"
    binfile IDR_XML_CHECKLIST_P "../../Data/Dialogs/dlgChecklist_P.xml"

    binfile IDR_XML_STARTUP_L "../../Data/Dialogs/dlgStartup_L.xml"
    binfile IDR_XML_STARTUP_P "../../Data/Dialogs/dlgStartup_P.xml"

    binfile IDR_XML_DUALPROFILE_L "../../Data/Dialogs/dlgDualProfile_L.xml"
    binfile IDR_XML_DUALPROFILE_P "../../Data/Dialogs/dlgDualProfile_P.xml"

    binfile IDR_XML_ORACLE_L "../../Data/Dialogs/dlgOracle_L.xml"
    binfile IDR_XML_ORACLE_P "../../Data/Dialogs/dlgOracle_P.xml"

    binfile IDR_XML_TASKOVERVIEW_L "../../Data/Dialogs/dlgTaskOverview_L.xml"
    binfile IDR_XML_TASKOVERVIEW_P "../../Data/Dialogs/dlgTaskOverview_P.xml"

    binfile IDR_XML_TASKWAYPOINT_L "../../Data/Dialogs/dlgTaskWaypoint_L.xml"
    binfile IDR_XML_TASKWAYPOINT_P "../../Data/Dialogs/dlgTaskWaypoint_P.xml"

    binfile IDR_XML_WAYPOINTDETAILS_L "../../Data/Dialogs/dlgWayPointDetails_L.xml"
    binfile IDR_XML_WAYPOINTDETAILS_P "../../Data/Dialogs/dlgWayPointDetails_P.xml"

    binfile IDR_XML_WAYPOINTSELECT_L "../../Data/Dialogs/dlgWayPointSelect_L.xml"
    binfile IDR_XML_WAYPOINTSELECT_P "../../Data/Dialogs/dlgWayPointSelect_P.xml"

    binfile IDR_XML_WAYPOINTQUICK_L "../../Data/Dialogs/dlgWayQuick_L.xml"
    binfile IDR_XML_WAYPOINTQUICK_P "../../Data/Dialogs/dlgWayQuick_P.xml"

    binfile IDR_XML_TEXTENTRY_KEYBOARD_L "../../Data/Dialogs/dlgTextEntry_Keyboard_L.xml"
    binfile IDR_XML_TEXTENTRY_KEYBOARD_P "../../Data/Dialogs/dlgTextEntry_Keyboard_P.xml"

    binfile IDR_XML_NUMENTRY_KEYBOARD_L "../../Data/Dialogs/dlgNumEntry_Keyboard_L.xml"
    binfile IDR_XML_NUMENTRY_KEYBOARD_P "../../Data/Dialogs/dlgNumEntry_Keyboard_P.xml"

    binfile IDR_XML_STARTPOINT_L "../../Data/Dialogs/dlgStartPoint_L.xml"
    binfile IDR_XML_STARTPOINT_P "../../Data/Dialogs/dlgStartPoint_P.xml"

    binfile IDR_XML_WAYPOINTEDIT_L "../../Data/Dialogs/dlgWaypointEdit_L.xml"
    binfile IDR_XML_WAYPOINTEDIT_P "../../Data/Dialogs/dlgWaypointEdit_P.xml"

    binfile IDR_XML_TARGET_L "../../Data/Dialogs/dlgTarget_L.xml"
    binfile IDR_XML_TARGET_P "../../Data/Dialogs/dlgTarget_P.xml"

    binfile IDR_XML_LKAIRSPACEWARNING_L "../../Data/Dialogs/dlgLKAirspaceWarning_L.xml"
    binfile IDR_XML_LKAIRSPACEWARNING_P "../../Data/Dialogs/dlgLKAirspaceWarning_P.xml"

    binfile IDR_XML_HELP_L "../../Data/Dialogs/dlgHelp_L.xml"
    binfile IDR_XML_HELP_P "../../Data/Dialogs/dlgHelp_P.xml"

    binfile IDR_XML_AIRSPACESELECT_P "../../Data/Dialogs/dlgAirspaceSelect_P.xml"
    binfile IDR_XML_AIRSPACESELECT_L "../../Data/Dialogs/dlgAirspaceSelect_L.xml"

    binfile IDR_XML_COMBOPICKER_L "../../Data/Dialogs/dlgComboPicker_L.xml"
    binfile IDR_XML_COMBOPICKER_P "../../Data/Dialogs/dlgComboPicker_P.xml"

    binfile IDR_XML_CONFIGURATION_L "../../Data/Dialogs/dlgConfiguration_L.xml"
    binfile IDR_XML_CONFIGURATION_P "../../Data/Dialogs/dlgConfiguration_P.xml"

    binfile IDR_XML_CONFIGAIRCRAFT_L "../../Data/Dialogs/dlgConfigAircraft_L.xml"
    binfile IDR_XML_CONFIGAIRCRAFT_P "../../Data/Dialogs/dlgConfigAircraft_P.xml"

    binfile IDR_XML_CONFIGPILOT_L "../../Data/Dialogs/dlgConfigPilot_L.xml"
    binfile IDR_XML_CONFIGPILOT_P "../../Data/Dialogs/dlgConfigPilot_P.xml"

    binfile IDR_XML_CONFIGDEVICE_L "../../Data/Dialogs/dlgConfigDevice_L.xml"
    binfile IDR_XML_CONFIGDEVICE_P "../../Data/Dialogs/dlgConfigDevice_P.xml"

    binfile IDR_XML_WAYPOINTEDITUTM_L "../../Data/Dialogs/dlgWaypointEditUTM_L.xml"
    binfile IDR_XML_WAYPOINTEDITUTM_P "../../Data/Dialogs/dlgWaypointEditUTM_P.xml"

    binfile IDR_XML_OVERLAYS "../../Data/Dialogs/dlgOverlays.xml"
    binfile IDR_XML_MULTIMAPS "../../Data/Dialogs/dlgMultimaps.xml"
    binfile IDR_XML_DEVCPROBE "../../Data/Dialogs/dlgDevCProbe.xml"

    binfile IDR_XML_BLUETOOTH_L "../../Data/Dialogs/dlgBluetooth_L.xml"
    binfile IDR_XML_BLUETOOTH_P "../../Data/Dialogs/dlgBluetooth_P.xml"

    binfile IDR_XML_IGCFILE_L "../../Data/Dialogs/dlgIgcFile_L.xml"
    binfile IDR_XML_IGCFILE_P "../../Data/Dialogs/dlgIgcFile_P.xml"

    binfile IDR_XML_BLUEFLYCONFIG_L "../../Data/Dialogs/dlgBlueFlyConfig_L.xml"
    binfile IDR_XML_BLUEFLYCONFIG_P "../../Data/Dialogs/dlgBlueFlyConfig_P.xml"

    binfile IDR_XML_TERMINAL_L  "../../Data/Dialogs/dlgTerminal_L.xml"
    binfile IDR_XML_TERMINAL_P  "../../Data/Dialogs/dlgTerminal_P.xml"

    binfile IDR_XML_PROGRESS_L "../../Data/Dialogs/dlgProgress_L.xml"
    binfile IDR_XML_PROGRESS_P "../../Data/Dialogs/dlgProgress_P.xml"
   binfile IDR_XML_RADIOSETTINGS  "../../Data/Dialogs/dlgRadioSettings.xml"
    binfile IDR_XML_LOGGERREPLAY "../../Data/Dialogs/dlgLoggerReplay.xml"
    binfile IDR_XML_STATUS "../../Data/Dialogs/dlgStatus.xml"
    binfile IDR_XML_TASKCALCULATOR "../../Data/Dialogs/dlgTaskCalculator.xml"
    binfile IDR_XML_WINDSETTINGS "../../Data/Dialogs/dlgWindSettings.xml"
    binfile IDR_XML_STARTTASK "../../Data/Dialogs/dlgStartTask.xml"
    binfile IDR_XML_TIMEGATES "../../Data/Dialogs/dlgTimeGates.xml"
    binfile IDR_XML_TOPOLOGY "../../Data/Dialogs/dlgTopology.xml"
    binfile IDR_XML_CUSTOMMENU "../../Data/Dialogs/dlgCustomMenu.xml"
    binfile IDR_XML_CUSTOMKEYS "../../Data/Dialogs/dlgCustomKeys.xml"
    binfile IDR_XML_BOTTOMBAR "../../Data/Dialogs/dlgBottomBar.xml"
    binfile IDR_XML_INFOPAGES "../../Data/Dialogs/dlgInfoPages.xml"
    binfile IDR_XML_WAYPOINTTERRAIN "../../Data/Dialogs/dlgWaypointOutOfTerrain.xml"
    binfile IDR_XML_TEAMCODE "../../Data/Dialogs/dlgTeamCode.xml"
    binfile IDR_XML_TASKRULES "../../Data/Dialogs/dlgTaskRules.xml"
    binfile IDR_XML_AIRSPACEDETAILS "../../Data/Dialogs/dlgAirspaceDetails.xml"
    binfile IDR_XML_LKTRAFFICDETAILS "../../Data/Dialogs/dlgLKTraffic.xml"
    binfile IDR_XML_THERMALDETAILS "../../Data/Dialogs/dlgThermalDetails.xml"
    binfile IDR_XML_PROFILES "../../Data/Dialogs/dlgProfiles.xml"
    binfile IDR_XML_AIRSPACEWARNINGPARAMS "../../Data/Dialogs/dlgAirspaceWarningParams.xml"
