/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#include "externs.h"
#include "WindowControls.h"

namespace {

	struct DATAOPTIONS {
		MsgToken_t Description;
		MsgToken_t Title;
	};

	constexpr size_t NUMDATAOPTIONS_MAX = 159;
	DATAOPTIONS Data_Options[NUMDATAOPTIONS_MAX];

	template <int index, unsigned DescriptionId, unsigned TitleId>
	void SetDataOption() {

		static_assert(index < NUMDATAOPTIONS_MAX, "invalid index or NUMDATAOPTIONS_MAX too small");

		Data_Options[index] = {
			&MsgToken<DescriptionId>,
			&MsgToken<TitleId>
		};

		if (NumDataOptions <= index) {
			NumDataOptions = index + 1; //No. of items = max index+1
		}
	}
}

void FillDataOptions() {

	// cleanup array, mandatory for avoid to have pointer to freed string.
	std::fill(std::begin(Data_Options), std::end(Data_Options), DATAOPTIONS{});

	// LKTOKEN  _@M1001_ = "Altitude QNH", _@M1002_ = "Alt"
	SetDataOption<LK_HNAV, 1001, 1002>();
	// LKTOKEN  _@M1003_ = "Altitude AGL", _@M1004_ = "HAGL"
	SetDataOption<LK_HAGL, 1003, 1004>();
	// LKTOKEN  _@M1005_ = "Thermal last 30 sec", _@M1006_ = "TC.30\""
	SetDataOption<LK_TC_30S, 1005, 1006>();
	// LKTOKEN  _@M1007_ = "Bearing", _@M1008_ = "Brg"
	SetDataOption<LK_BRG, 1007, 1008>();
	// LKTOKEN  _@M1009_ = "Eff.last 20 sec", _@M1010_ = "E.20\""
	SetDataOption<LK_LD_INST, 1009, 1010>();
	// LKTOKEN  _@M1011_ = "Eff.cruise last therm", _@M1012_ = "E.Cru"
	SetDataOption<LK_LD_CRUISE, 1011, 1012>();
	// LKTOKEN  _@M1013_ = "Speed ground", _@M1014_ = "GS"
	SetDataOption<LK_GNDSPEED, 1013, 1014>();
	// LKTOKEN  _@M1015_ = "Thermal Average Last", _@M1016_ = "TL.Avg"
	SetDataOption<LK_TL_AVG, 1015, 1016>();
	// LKTOKEN  _@M1017_ = "Thermal Gain Last", _@M1018_ = "TL.Gain"
	SetDataOption<LK_TL_GAIN, 1017, 1018>();
	// LKTOKEN  _@M1019_ = "Thermal Time Last", _@M1020_ = "TL.Time"
	SetDataOption<LK_TL_TIME, 1019, 1020>();
	// LKTOKEN  _@M1021_ = "MacCready Setting", _@M1022_ = "MCready"
	SetDataOption<LK_MC, 1021, 1022>();
	// LKTOKEN  _@M1023_ = "Next Distance", _@M1024_ = "Dist"
	SetDataOption<LK_NEXT_DIST, 1023, 1024>();
	// LKTOKEN  _@M1025_ = "Next Alt.Arrival", _@M1026_ = "NxtArr"
	SetDataOption<LK_NEXT_ALTDIFF, 1025, 1026>();
	// LKTOKEN  _@M1027_ = "Next Alt.Required", _@M1028_ = "NxtAltR"
	SetDataOption<LK_NEXT_ALTREQ, 1027, 1028>();
	// LKTOKEN  _@M1029_ = "Next Waypoint", _@M1030_ = "Next"  -> Reserved 2
	SetDataOption<LK_NEXT_WP, 1807, 1030>();
	// LKTOKEN  _@M1031_ = "Task Alt.Arrival", _@M1032_ = "TskArr"
	SetDataOption<LK_FIN_ALTDIFF, 1031, 1032>();
	// LKTOKEN  _@M1033_ = "Task Alt.Required", _@M1034_ = "TskAltR"
	SetDataOption<LK_FIN_ALTREQ, 1033, 1034>();
	// LKTOKEN  _@M1035_ = "Task Speed Average", _@M1036_ = "TskSpAv"
	SetDataOption<LK_SPEEDTASK_AVG, 1035, 1036>();
	// LKTOKEN  _@M1037_ = "Task Distance", _@M1038_ = "TskDis"
	SetDataOption<LK_FIN_DIST, 1037, 1038>();
	// LKTOKEN  _@M1039_ = "_Reserved 1", _@M1040_ = "OLD fLD"
	SetDataOption<LK_RESERVED1, 1039, 1040>();
	// LKTOKEN  _@M1041_ = "Terrain Elevation", _@M1042_ = "Gnd"
	SetDataOption<LK_HGND, 1041, 1042>();
	// LKTOKEN  _@M1043_ = "Thermal Average", _@M1044_ = "TC.Avg"
	SetDataOption<LK_TC_AVG, 1043, 1044>();
	// LKTOKEN  _@M1045_ = "Thermal Gain", _@M1046_ = "TC.Gain"
	SetDataOption<LK_TC_GAIN, 1045, 1046>();
	// LKTOKEN  _@M1047_ = "Track", _@M1048_ = "Track"
	SetDataOption<LK_TRACK, 1047, 1048>();
	// LKTOKEN  _@M1049_ = "Vario", _@M1050_ = "Vario"
	SetDataOption<LK_VARIO, 1049, 1050>();
	// LKTOKEN  _@M1051_ = "Wind Speed", _@M1052_ = "WindV"
	SetDataOption<LK_WIND_SPEED, 1051, 1052>();
	// LKTOKEN  _@M1053_ = "Wind Bearing", _@M1054_ = "WindB"
	SetDataOption<LK_WIND_BRG, 1053, 1054>();
	// LKTOKEN  _@M1055_ = "AA Time", _@M1056_ = "AATime"
	SetDataOption<LK_AA_TIME, 1055, 1056>();
	// LKTOKEN  _@M1057_ = "AA Distance Max", _@M1058_ = "AADmax"
	SetDataOption<LK_AA_DISTMAX, 1057, 1058>();
	// LKTOKEN  _@M1059_ = "AA Distance Min", _@M1060_ = "AADmin"
	SetDataOption<LK_AA_DISTMIN, 1059, 1060>();
	// LKTOKEN  _@M1061_ = "AA Speed Max", _@M1062_ = "AAVmax"
	SetDataOption<LK_AA_SPEEDMAX, 1061, 1062>();
	// LKTOKEN  _@M1063_ = "AA Speed Min", _@M1064_ = "AAVmin"
	SetDataOption<LK_AA_SPEEDMIN, 1063, 1064>();
	// LKTOKEN  _@M1065_ = "Airspeed IAS", _@M1066_ = "IAS"
	SetDataOption<LK_IAS, 1065, 1066>();
	// LKTOKEN  _@M1067_ = "Altitude BARO", _@M1068_ = "HBAR"
	SetDataOption<LK_HBARO, 1067, 1068>();
	// LKTOKEN  _@M1069_ = "Speed MacReady", _@M1070_ = "SpMc"
	SetDataOption<LK_SPEED_MC, 1069, 1070>();
	// LKTOKEN  _@M1071_ = "Percentage clim", _@M1072_ = "%Climb"
	SetDataOption<LK_PRCCLIMB, 1071, 1072>();
	// LKTOKEN  _@M1073_ = "Time of flight", _@M1074_ = "FlyTime"
	SetDataOption<LK_TIMEFLIGHT, 1073, 1074>();
	// LKTOKEN  _@M1075_ = "G load", _@M1076_ = "G"
	SetDataOption<LK_GLOAD, 1075, 1076>();
	// LKTOKEN  _@M1699_ =  "Multitarget Bearing"  "_@M001699_": "BrgMtg",
	SetDataOption<LK_MTG_BRG, 1698, 1699>();
	// LKTOKEN  _@M2476_ = "Multitarget Bearing Difference   "_@M002477_": "DiffMtg",
	SetDataOption<LK_MTG_BRG_DIFF, 2476, 2477>();      
	// LKTOKEN  _@M1079_ = "Time local", _@M1080_ = "Time"
	SetDataOption<LK_TIME_LOCAL, 1079, 1080>();
	// LKTOKEN  _@M1081_ = "Time UTC", _@M1082_ = "UTC"
	SetDataOption<LK_TIME_UTC, 1081, 1082>();
	// LKTOKEN  _@M1083_ = "Task Time To Go", _@M1084_ = "TskETE"
	SetDataOption<LK_FIN_ETE, 1083, 1084>();
	// LKTOKEN  _@M1085_ = "Next Time To Go", _@M1086_ = "NextETE"
	SetDataOption<LK_NEXT_ETE, 1085, 1086>();
	// LKTOKEN  _@M1087_ = "Speed To Fly", _@M1088_ = "STF"
	SetDataOption<LK_SPEED_DOLPHIN, 1087, 1088>();
	// LKTOKEN  _@M1089_ = "Netto Vario", _@M1090_ = "Netto"
	SetDataOption<LK_NETTO, 1089, 1090>();
	// LKTOKEN  _@M1091_ = "Task Arrival Time", _@M1092_ = "TskETA"
	SetDataOption<LK_FIN_ETA, 1091, 1092>();
	// LKTOKEN  _@M1093_ = "Next Arrival Time", _@M1094_ = "NextETA"
	SetDataOption<LK_NEXT_ETA, 1093, 1094>();
	// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
	SetDataOption<LK_BRGDIFF, 1095, 1096>();
	// LKTOKEN  _@M1097_ = "Outside Air Temperature", _@M1098_ = "OAT"
	SetDataOption<LK_OAT, 1097, 1098>();
	// LKTOKEN  _@M1099_ = "Relative Humidity", _@M1100_ = "RelHum"
	SetDataOption<LK_RELHUM, 1099, 1100>();
	// LKTOKEN  _@M1101_ = "Forecast Temperature", _@M1102_ = "MaxTemp"
	SetDataOption<LK_MAXTEMP, 1101, 1102>();
	// LKTOKEN  _@M1103_ = "AA Distance Tg", _@M1104_ = "AADtgt"
	SetDataOption<LK_AA_TARG_DIST, 1103, 1104>();
	// LKTOKEN  _@M1105_ = "AA Speed Tg", _@M1106_ = "AAVtgt"
	SetDataOption<LK_AA_TARG_SPEED, 1105, 1106>();
	// LKTOKEN  _@M1107_ = "L/D vario", _@M1108_ = "L/D vario"
	SetDataOption<LK_LD_VARIO, 1107, 1108>();
	// LKTOKEN  _@M1109_ = "Airspeed TAS", _@M1110_ = "TAS"
	SetDataOption<LK_TAS, 1109, 1110>();
	// LKTOKEN  _@M1111_ = "Team Code", _@M1112_ = "TeamCode"
	SetDataOption<LK_TEAM_CODE, 1111, 1112>();
	// LKTOKEN  _@M1113_ = "Team Bearing", _@M1114_ = "TmBrng"
	SetDataOption<LK_TEAM_BRG, 1113, 1114>();
	// LKTOKEN  _@M1115_ = "Team Bearing Diff", _@M1116_ = "TeamBd"
	SetDataOption<LK_TEAM_BRGDIFF, 1115, 1116>();
	// LKTOKEN  _@M1117_ = "Team Range", _@M1118_ = "TeamDis"
	SetDataOption<LK_TEAM_DIST, 1117, 1118>();
	// LKTOKEN  _@M1119_ = "Task Speed Instantaneous", _@M1120_ = "TskSpI"
	SetDataOption<LK_SPEEDTASK_INST, 1119, 1120>();
	// LKTOKEN  _@M1121_ = "Home Distance", _@M1122_ = "HomeDis"
	SetDataOption<LK_HOME_DIST, 1121, 1122>();
	// LKTOKEN  _@M1123_ = "Task Speed", _@M1124_ = "TskSp"
	SetDataOption<LK_SPEEDTASK_ACH, 1123, 1124>();
	// LKTOKEN  _@M1125_ = "AA Delta Time", _@M1126_ = "AAdT"
	SetDataOption<LK_AA_DELTATIME, 1125, 1126>();
	// LKTOKEN  _@M1127_ = "Thermal All", _@M1128_ = "Th.All"
	SetDataOption<LK_TC_ALL, 1127, 1128>();

	SetDataOption<LK_LOGGER, 1695, 1695>();

	// LKTOKEN  _@M1131_ = "Battery Percent", _@M1132_ = "Battery"
	SetDataOption<LK_BATTERY, 1131, 1132>();
	// LKTOKEN  _@M1133_ = "Task Req.Efficiency", _@M1134_ = "TskReqE"
	SetDataOption<LK_FIN_GR, 1133, 1134>();
	// LKTOKEN  _@M1135_ = "Alternate1 Req.Efficiency", _@M1136_ = "Atn1.E"
	SetDataOption<LK_ALTERN1_GR, 1135, 1136>();
	// LKTOKEN  _@M1137_ = "Alternate2 Req.Efficiency", _@M1138_ = "Atn2.E"
	SetDataOption<LK_ALTERN2_GR, 1137, 1138>();
	// LKTOKEN  _@M1139_ = "BestAltern Req.Efficiency", _@M1140_ = "BAtn.E"
	SetDataOption<LK_BESTALTERN_GR, 1139, 1140>();
	// LKTOKEN  _@M2484_ = "Home Req.Efficiency", _@M2485_ = "Home.E"
	SetDataOption<LK_HOME_GR, 2484, 2485>();
	// LKTOKEN  _@M1141_ = "Altitude QFE", _@M1142_ = "QFE"
	SetDataOption<LK_QFE, 1141, 1142>();
	// LKTOKEN  _@M1143_ = "Average Efficiency", _@M1144_ = "E.Avg"
	SetDataOption<LK_LD_AVR, 1143, 1144>();
	// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
	SetDataOption<LK_NEXT_GR, 1145, 1146>();
	// LKTOKEN  _@M1147_ = "Flight Level", _@M1148_ = "FL"
	SetDataOption<LK_FL, 1147, 1148>();
	// LKTOKEN  _@M1149_ = "Task Covered distance", _@M1150_ = "TskCov"
	SetDataOption<LK_TASK_DISTCOV, 1149, 1150>();
	// LKTOKEN  _@M1151_ = "Alternate1 Arrival", _@M1152_ = "Atn1Arr"
	SetDataOption<LK_ALTERN1_ARRIV, 1151, 1152>();
	// LKTOKEN  _@M1153_ = "Alternate2 Arrival", _@M1154_ = "Atn2Arr"
	SetDataOption<LK_ALTERN2_ARRIV, 1153, 1154>();
	// LKTOKEN  _@M1155_ = "BestAlternate Arrival", _@M1156_ = "BAtnArr"
	SetDataOption<LK_BESTALTERN_ARRIV, 1155, 1156>();
	// LKTOKEN  _@M1157_ = "Home Radial", _@M1158_ = "Radial"
	SetDataOption<LK_HOMERADIAL, 1157, 1158>();
	// LKTOKEN  _@M1159_ "Airspace Horizontal Dist", _@M1160_ "ArSpcH"
	SetDataOption<LK_AIRSPACEHDIST, 1159, 1160>();
	// LKTOKEN  _@M1161_ = "Ext.Batt.Bank", _@M1162_ = "xBnk#"
	SetDataOption<LK_EXTBATTBANK, 1161, 1162>();
	// LKTOKEN  _@M1163_ = "Ext.Batt.1 Voltage", _@M1164_ = "xBat1"
	SetDataOption<LK_EXTBATT1VOLT, 1163, 1164>();
	// LKTOKEN  _@M1165_ = "Ext.Batt.2 Voltage", _@M1166_ = "xBat2"
	SetDataOption<LK_EXTBATT2VOLT, 1165, 1166>();
	// LKTOKEN  _@M1167_ = "Odometer", _@M1168_ = "Odo"
	SetDataOption<LK_ODOMETER, 1167, 1168>();
	// LKTOKEN  _@M1169_ = "Altern QNH", _@M1170_ = "aAlt"
	SetDataOption<LK_AQNH, 1169, 1170>();
	// LKTOKEN  _@M1171_ = "Altern AGL", _@M1172_ = "aHAGL"
	SetDataOption<LK_AALTAGL, 1171, 1172>();
	// LKTOKEN  _@M1173_ = "Altitude GPS", _@M1174_ = "HGPS"
	SetDataOption<LK_HGPS, 1173, 1174>();
	// LKTOKEN  _@M1175_ = "MacCready Equivalent", _@M1176_ = "eqMC"
	SetDataOption<LK_EQMC, 1175, 1176>();
	// LKTOKEN  _@M1177_ = "_Experimental1", _@M1178_ = "Exp1"
	SetDataOption<LK_EXP1, 1177, 1178>();
	// LKTOKEN  _@M1179_ = "_Experimental2", _@M1180_ = "Exp2"
	SetDataOption<LK_EXP2, 1179, 1180>();
	// LKTOKEN  _@M2472_ = "MultiTarget QNH Arrival", _@M2473_ = "MTgtArr"
	SetDataOption<LK_MTG_QNH_ARRIV, 2472, 2473>();
	// LKTOKEN  _@M2478_ = "Alternate 1 QNH Arrival", _@M2479_ = "Alt1QNH"
	SetDataOption<LK_ALTERN1_QNH_ARRIV, 2474, 2475>();

	// Distance OLC
	SetDataOption<LK_OLC_CLASSIC_DIST, 1455, 1456>();
	// Distance FAI triangle
	SetDataOption<LK_OLC_FAI_DIST, 1457, 1458>();
	// Distance League
	SetDataOption<LK_OLC_LEAGUE_DIST, 1459, 1460>();
	// Distance FAI 3 TPs
	SetDataOption<LK_OLC_3TPS_DIST, 1461, 1462>();

	////////  PREDICTED ////////
	// Distance OLC
	SetDataOption<LK_OLC_CLASSIC_PREDICTED_DIST, 1463, 1464>();
	// Distance FAI triangle
	SetDataOption<LK_OLC_FAI_PREDICTED_DIST, 1465, 1466>();
	// Distance FAI 3 TPs
	SetDataOption<LK_OLC_3TPS_PREDICTED_DIST, 1469, 1470>();

	// Speed OLC
	SetDataOption<LK_OLC_CLASSIC_SPEED, 1471, 1472>();
	// Speed FAI triangle
	SetDataOption<LK_OLC_FAI_SPEED, 1473, 1474>();
	// Speed League
	SetDataOption<LK_OLC_LEAGUE_SPEED, 1475, 1476>();
	// Speed FAI 3 TPs
	SetDataOption<LK_OLC_3TPS_SPEED, 1477, 1478>();

	////////  PREDICTED ////////
	// Speed OLC
	SetDataOption<LK_OLC_CLASSIC_PREDICTED_SPEED, 1479, 1480>();
	// Speed FAI triangle
	SetDataOption<LK_OLC_FAI_PREDICTED_SPEED, 1481, 1482>();
	// Speed FAI 3 TPs
	SetDataOption<LK_OLC_3TPS_PREDICTED_SPEED, 1485, 1486>();

	// Score OLC
	SetDataOption<LK_OLC_CLASSIC_SCORE, 1487, 1488>();
	// Score FAI triangle
	SetDataOption<LK_OLC_FAI_SCORE, 1489, 1490>();
	// Score League
	SetDataOption<LK_OLC_LEAGUE_SCORE, 1491, 1492>();
	// FAI 3 TPs currently has no score
	SetDataOption<LK_OLC_3TPS_SCORE, 1493, 1494>();

	////////  PREDICTED ////////
	// Score OLC
	SetDataOption<LK_OLC_CLASSIC_PREDICTED_SCORE, 1495, 1496>();
	// Score FAI triangle
	SetDataOption<LK_OLC_FAI_PREDICTED_SCORE, 1497, 1498>();
	// FAI 3 TPs currently has no score
	SetDataOption<LK_OLC_3TPS_PREDICTED_SCORE, 1501, 1502>();

	// Olc Plus Score
	SetDataOption<LK_OLC_PLUS_SCORE, 1503, 1504>();
	// Olc Plus Score Predicted
	SetDataOption<LK_OLC_PLUS_PREDICTED_SCORE, 1505, 1506>();

	// Flaps
	SetDataOption<LK_FLAPS, 1640, 1641>();

	// Vertical distance to airspace
	SetDataOption<LK_AIRSPACEVDIST, 1285, 1286>();

	// LKTOKEN  _@M1644_ = "Home Alt.Arrival", _@M1645_ = "HomeArr"
	SetDataOption<LK_HOME_ARRIVAL, 1644, 1645>();

	// No reason to have abbreviated name, since normally it is the wp name itself
	SetDataOption<LK_ALTERN1_BRG, 1761, 1512>(); // Alternate1 bearing
	SetDataOption<LK_ALTERN2_BRG, 1762, 1513>(); // Alternate2 bearing
	SetDataOption<LK_HOME_BRG, 2488, 2489>(); // Home bearing
	SetDataOption<LK_BESTALTERN_BRG, 1763, 1514>(); // BestAlternate bearing
	SetDataOption<LK_ALTERN1_DIST, 1764, 1515>(); // Alternate1 distance
	SetDataOption<LK_ALTERN2_DIST, 1765, 1516>(); // Alternate2 distance
	SetDataOption<LK_BESTALTERN_DIST, 1766, 1517>(); // BestAlternate distance

	SetDataOption<LK_MAXALT, 1767, 1768>(); // Max Altitude reached
	SetDataOption<LK_MAXHGAINED, 1769, 1770>(); // Max Height gained
	SetDataOption<LK_HEADWINDSPEED, 1771, 1772>(); // Head wind speed
	SetDataOption<LK_OLC_FAI_CLOSE, 1507, 1508>(); //OLC FAI triangle Distance to close
	SetDataOption<LK_OLC_FAI_CLOSE_PERCENT, 1509, 1510>(); // OLC FAI triangle Distance close %
	SetDataOption<LK_BANK_ANGLE, 1518, 1197>(); // Bank angle
	SetDataOption<LK_ALTERN1_RAD, 1841, 1519>(); // Alternate1 radial
	SetDataOption<LK_ALTERN2_RAD, 1842, 1520>(); // Alternate2 radial
	SetDataOption<LK_HEADING, 1287, 1521>(); // Heading, text is changed in lkprocess
	SetDataOption<LK_ALTERN1_DISTNM, 1843, 1522>(); // Alternate1 distance NMiles
	SetDataOption<LK_ALTERN2_DISTNM, 1844, 1523>(); // Alternate2 distance NMiles
	SetDataOption<LK_HOME_DISTNM, 2486, 2487>(); //  home distance NMiles
	SetDataOption<LK_SPEED_ME, 2312, 2313>(); // Speed of maximum efficiency
	SetDataOption<LK_TARGET_RE, 2314, 2315>(); // Target Req. Efficicency
	SetDataOption<LK_QNE, 2323, 2324>(); // Altitude QNE
	SetDataOption<LK_NEXT_DIST_RADIUS, 2190, 2191>(); // Dist to next Cylinder turnpoint
	SetDataOption<LK_XC_FF_DIST, 2255, 2266>();		// AdditionalContest FREE Fligth distance
	SetDataOption<LK_XC_FF_SCORE, 2256, 2267>();		// AdditionalContest FREE Fligth score
	SetDataOption<LK_XC_FT_DIST, 2257, 2268>();		// AdditionalContest FREE Triangle distance
	SetDataOption<LK_XC_FT_SCORE, 2258, 2269>();		// AdditionalContest FREE Triangle score
	SetDataOption<LK_XC_FAI_DIST, 2259, 2270>();		// AdditionalContest FAI Triangle distance
	SetDataOption<LK_XC_FAI_SCORE, 2260, 2271>();		// AdditionalContest FAI Triangle score
	SetDataOption<LK_XC_DIST, 2261, 2272>();		// AdditionalContest combined distance
	SetDataOption<LK_XC_SCORE, 2262, 2273>();		// AdditionalContest combined score
	SetDataOption<LK_XC_CLOSURE_DIST, 2263, 2274>();		   // AdditionalContest combined closure distance
	SetDataOption<LK_XC_CLOSURE_PERC, 2264, 2275>();		// AdditionalContest combined closure %
	SetDataOption<LK_XC_PREDICTED_DIST, 2265, 2276>();		   // Additional Contest combined distance
	SetDataOption<LK_XC_MEAN_SPEED, 2277, 2278>();		   // Additional Contest combined distance
	SetDataOption<LK_XC_TYPE, 2495, 2495>();		   // Type of result in XC
	SetDataOption<LK_TIMETASK, 2427, 2488>();		   // Elapsed task time (time since start
	SetDataOption<LK_HEART_RATE, 2500, 2501>();		   // Pilot Heart rate in bpm
	SetDataOption<LK_AVERAGE_GS, 2502, 2503>();		   // Average speed for Eff Time

	// Fill all null string pointer with empty string, avoid to check all time is used.
	for(auto& tag : Data_Options) {
		if(!tag.Description) {
			tag.Description = []() { return _T(""); };
		}
		if(!tag.Title) {
			tag.Title = []() { return _T(""); };
		}
	}
}

void FillDataOptionDescription(DataField* dfe) {
	for (int i=0; i<NumDataOptions; i++) {
		dfe->addEnumText(Data_Options[i].Description());
	}
}

const TCHAR* DataOptionsTitle(unsigned index) {
	if(index < std::size(Data_Options)) {
		return Data_Options[index].Title();
	}
	return _T("");
}
