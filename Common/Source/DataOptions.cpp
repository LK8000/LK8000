/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


//
// LK Infobox list
// Included by lk temporarily, only with CUTIBOX
//
bool SetDataOption( int index, UnitGroup_t UnitGroup, const TCHAR *Description, const TCHAR *Title)
{
	LKASSERT(index<NUMDATAOPTIONS_MAX);
	if (index>=NUMDATAOPTIONS_MAX) return false;

    Data_Options[index] = {
        UnitGroup,
        LKGetText(Description),
        LKGetText(Title)
    };
	if (NumDataOptions<=index) NumDataOptions=index+1; //No. of items = max index+1

	return true;
}



void FillDataOptions()
{

   // cleanup array, madatory for avoid to have pointer to freed string.
   std::fill(std::begin(Data_Options), std::end(Data_Options), DATAOPTIONS{});


	// LKTOKEN  _@M1001_ = "Altitude QNH", _@M1002_ = "Alt"
	SetDataOption(0, ugAltitude,		TEXT("_@M1001_"), TEXT("_@M1002_"));
	// LKTOKEN  _@M1003_ = "Altitude AGL", _@M1004_ = "HAGL"
	SetDataOption(1, ugAltitude,		TEXT("_@M1003_"), TEXT("_@M1004_"));
	// LKTOKEN  _@M1005_ = "Thermal last 30 sec", _@M1006_ = "TC.30\""
	SetDataOption(2, ugVerticalSpeed,	TEXT("_@M1005_"), TEXT("_@M1006_"));
	// LKTOKEN  _@M1007_ = "Bearing", _@M1008_ = "Brg"
	SetDataOption(3, ugNone,			TEXT("_@M1007_"), TEXT("_@M1008_"));
	// LKTOKEN  _@M1009_ = "Eff.last 20 sec", _@M1010_ = "E.20\""
	SetDataOption(4, ugNone,			TEXT("_@M1009_"), TEXT("_@M1010_"));
	// LKTOKEN  _@M1011_ = "Eff.cruise last therm", _@M1012_ = "E.Cru"
	SetDataOption(5, ugNone,            TEXT("_@M1011_"), TEXT("_@M1012_"));
	// LKTOKEN  _@M1013_ = "Speed ground", _@M1014_ = "GS"
	SetDataOption(6, ugHorizontalSpeed, TEXT("_@M1013_"), TEXT("_@M1014_"));
	// LKTOKEN  _@M1015_ = "Thermal Average Last", _@M1016_ = "TL.Avg"
	SetDataOption(7, ugVerticalSpeed,   TEXT("_@M1015_"), TEXT("_@M1016_"));
	// LKTOKEN  _@M1017_ = "Thermal Gain Last", _@M1018_ = "TL.Gain"
	SetDataOption(8, ugAltitude,        TEXT("_@M1017_"), TEXT("_@M1018_"));
	// LKTOKEN  _@M1019_ = "Thermal Time Last", _@M1020_ = "TL.Time"
	SetDataOption(9, ugNone,            TEXT("_@M1019_"), TEXT("_@M1020_"));
	// LKTOKEN  _@M1021_ = "MacCready Setting", _@M1022_ = "MCready"
	SetDataOption(10, ugVerticalSpeed,  TEXT("_@M1021_"), TEXT("_@M1022_"));
	// LKTOKEN  _@M1023_ = "Next Distance", _@M1024_ = "Dist"
	SetDataOption(11, ugDistance,       TEXT("_@M1023_"), TEXT("_@M1024_"));
	// LKTOKEN  _@M1025_ = "Next Alt.Arrival", _@M1026_ = "NxtArr"
	SetDataOption(12, ugAltitude,       TEXT("_@M1025_"), TEXT("_@M1026_"));
	// LKTOKEN  _@M1027_ = "Next Alt.Required", _@M1028_ = "NxtAltR"
	SetDataOption(13, ugAltitude,       TEXT("_@M1027_"), TEXT("_@M1028_"));
	// LKTOKEN  _@M1029_ = "Next Waypoint", _@M1030_ = "Next"  -> Reserved 2
	SetDataOption(14, ugNone,           TEXT("_@M1807_"), TEXT("_@M1030_"));
	// LKTOKEN  _@M1031_ = "Task Alt.Arrival", _@M1032_ = "TskArr"
	SetDataOption(15, ugAltitude,       TEXT("_@M1031_"), TEXT("_@M1032_"));
	// LKTOKEN  _@M1033_ = "Task Alt.Required", _@M1034_ = "TskAltR"
	SetDataOption(16, ugAltitude,       TEXT("_@M1033_"), TEXT("_@M1034_"));
	// LKTOKEN  _@M1035_ = "Task Speed Average", _@M1036_ = "TskSpAv"
	SetDataOption(17, ugTaskSpeed,		TEXT("_@M1035_"), TEXT("_@M1036_"));
	// LKTOKEN  _@M1037_ = "Task Distance", _@M1038_ = "TskDis"
	SetDataOption(18, ugDistance,       TEXT("_@M1037_"), TEXT("_@M1038_"));
	// LKTOKEN  _@M1039_ = "_Reserved 1", _@M1040_ = "OLD fLD"
	SetDataOption(19, ugNone,           TEXT("_@M1039_"), TEXT("_@M1040_"));
	// LKTOKEN  _@M1041_ = "Terrain Elevation", _@M1042_ = "Gnd"
	SetDataOption(20, ugAltitude,       TEXT("_@M1041_"), TEXT("_@M1042_"));
	// LKTOKEN  _@M1043_ = "Thermal Average", _@M1044_ = "TC.Avg"
	SetDataOption(21, ugVerticalSpeed,  TEXT("_@M1043_"), TEXT("_@M1044_"));
	// LKTOKEN  _@M1045_ = "Thermal Gain", _@M1046_ = "TC.Gain"
	SetDataOption(22, ugAltitude,       TEXT("_@M1045_"), TEXT("_@M1046_"));
	// LKTOKEN  _@M1047_ = "Track", _@M1048_ = "Track"
	SetDataOption(23, ugNone,           TEXT("_@M1047_"), TEXT("_@M1048_"));
	// LKTOKEN  _@M1049_ = "Vario", _@M1050_ = "Vario"
	SetDataOption(24, ugVerticalSpeed,  TEXT("_@M1049_"), TEXT("_@M1050_"));
	// LKTOKEN  _@M1051_ = "Wind Speed", _@M1052_ = "WindV"
	SetDataOption(25, ugWindSpeed,      TEXT("_@M1051_"), TEXT("_@M1052_"));
	// LKTOKEN  _@M1053_ = "Wind Bearing", _@M1054_ = "WindB"
	SetDataOption(26, ugNone,           TEXT("_@M1053_"), TEXT("_@M1054_"));
	// LKTOKEN  _@M1055_ = "AA Time", _@M1056_ = "AATime"
	SetDataOption(27, ugNone,           TEXT("_@M1055_"), TEXT("_@M1056_"));
	// LKTOKEN  _@M1057_ = "AA Distance Max", _@M1058_ = "AADmax"
	SetDataOption(28, ugDistance,       TEXT("_@M1057_"), TEXT("_@M1058_"));
	// LKTOKEN  _@M1059_ = "AA Distance Min", _@M1060_ = "AADmin"
	SetDataOption(29, ugDistance,       TEXT("_@M1059_"), TEXT("_@M1060_"));
	// LKTOKEN  _@M1061_ = "AA Speed Max", _@M1062_ = "AAVmax"
	SetDataOption(30, ugTaskSpeed,      TEXT("_@M1061_"), TEXT("_@M1062_"));
	// LKTOKEN  _@M1063_ = "AA Speed Min", _@M1064_ = "AAVmin"
	SetDataOption(31, ugTaskSpeed,      TEXT("_@M1063_"), TEXT("_@M1064_"));
	// LKTOKEN  _@M1065_ = "Airspeed IAS", _@M1066_ = "IAS"
	SetDataOption(32, ugHorizontalSpeed,TEXT("_@M1065_"), TEXT("_@M1066_"));
	// LKTOKEN  _@M1067_ = "Altitude BARO", _@M1068_ = "HBAR"
	SetDataOption(33, ugAltitude,       TEXT("_@M1067_"), TEXT("_@M1068_"));
	// LKTOKEN  _@M1069_ = "Speed MacReady", _@M1070_ = "SpMc"
	SetDataOption(34, ugHorizontalSpeed,TEXT("_@M1069_"), TEXT("_@M1070_"));
	// LKTOKEN  _@M1071_ = "Percentage clim", _@M1072_ = "%Climb"
	SetDataOption(35, ugNone,           TEXT("_@M1071_"), TEXT("_@M1072_"));
	// LKTOKEN  _@M1073_ = "Time of flight", _@M1074_ = "FlyTime"
	SetDataOption(36, ugNone,           TEXT("_@M1073_"), TEXT("_@M1074_"));
	// LKTOKEN  _@M1075_ = "G load", _@M1076_ = "G"
	SetDataOption(37, ugNone,           TEXT("_@M1075_"), TEXT("_@M1076_"));

	SetDataOption(38, ugNone,           TEXT("_@M1698_"), TEXT("_@M1699_")); // MTG BRG

	// LKTOKEN  _@M1079_ = "Time local", _@M1080_ = "Time"
	SetDataOption(39, ugNone,           TEXT("_@M1079_"), TEXT("_@M1080_"));
	// LKTOKEN  _@M1081_ = "Time UTC", _@M1082_ = "UTC"
	SetDataOption(40, ugNone,           TEXT("_@M1081_"), TEXT("_@M1082_"));
	// LKTOKEN  _@M1083_ = "Task Time To Go", _@M1084_ = "TskETE"
	SetDataOption(41, ugNone,           TEXT("_@M1083_"), TEXT("_@M1084_"));
	// LKTOKEN  _@M1085_ = "Next Time To Go", _@M1086_ = "NextETE"
	SetDataOption(42, ugNone,           TEXT("_@M1085_"), TEXT("_@M1086_"));
	// LKTOKEN  _@M1087_ = "Speed To Fly", _@M1088_ = "STF"
	SetDataOption(43, ugHorizontalSpeed,TEXT("_@M1087_"), TEXT("_@M1088_"));
	// LKTOKEN  _@M1089_ = "Netto Vario", _@M1090_ = "Netto"
	SetDataOption(44, ugVerticalSpeed,  TEXT("_@M1089_"), TEXT("_@M1090_"));
	// LKTOKEN  _@M1091_ = "Task Arrival Time", _@M1092_ = "TskETA"
	SetDataOption(45, ugNone,           TEXT("_@M1091_"), TEXT("_@M1092_"));
	// LKTOKEN  _@M1093_ = "Next Arrival Time", _@M1094_ = "NextETA"
	SetDataOption(46, ugNone,           TEXT("_@M1093_"), TEXT("_@M1094_"));
	// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
	SetDataOption(47, ugNone,           TEXT("_@M1095_"), TEXT("_@M1096_"));
	// LKTOKEN  _@M1097_ = "Outside Air Temperature", _@M1098_ = "OAT"
	SetDataOption(48, ugNone,           TEXT("_@M1097_"), TEXT("_@M1098_"));
	// LKTOKEN  _@M1099_ = "Relative Humidity", _@M1100_ = "RelHum"
	SetDataOption(49, ugNone,           TEXT("_@M1099_"), TEXT("_@M1100_"));
	// LKTOKEN  _@M1101_ = "Forecast Temperature", _@M1102_ = "MaxTemp"
	SetDataOption(50, ugNone,           TEXT("_@M1101_"), TEXT("_@M1102_"));
	// LKTOKEN  _@M1103_ = "AA Distance Tg", _@M1104_ = "AADtgt"
	SetDataOption(51, ugDistance,       TEXT("_@M1103_"), TEXT("_@M1104_"));
	// LKTOKEN  _@M1105_ = "AA Speed Tg", _@M1106_ = "AAVtgt"
	SetDataOption(52, ugTaskSpeed, TEXT("_@M1105_"), TEXT("_@M1106_"));
	// LKTOKEN  _@M1107_ = "L/D vario", _@M1108_ = "L/D vario"
	SetDataOption(53, ugNone,           TEXT("_@M1107_"), TEXT("_@M1108_"));
	// LKTOKEN  _@M1109_ = "Airspeed TAS", _@M1110_ = "TAS"
	SetDataOption(54, ugHorizontalSpeed,TEXT("_@M1109_"), TEXT("_@M1110_"));
	// LKTOKEN  _@M1111_ = "Team Code", _@M1112_ = "TeamCode"
	SetDataOption(55, ugNone,           TEXT("_@M1111_"), TEXT("_@M1112_"));
	// LKTOKEN  _@M1113_ = "Team Bearing", _@M1114_ = "TmBrng"
	SetDataOption(56, ugNone,           TEXT("_@M1113_"), TEXT("_@M1114_"));
	// LKTOKEN  _@M1115_ = "Team Bearing Diff", _@M1116_ = "TeamBd"
	SetDataOption(57, ugNone,           TEXT("_@M1115_"), TEXT("_@M1116_"));
	// LKTOKEN  _@M1117_ = "Team Range", _@M1118_ = "TeamDis"
	SetDataOption(58, ugNone,           TEXT("_@M1117_"), TEXT("_@M1118_"));
	// LKTOKEN  _@M1119_ = "Task Speed Instantaneous", _@M1120_ = "TskSpI"
	SetDataOption(59, ugTaskSpeed,      TEXT("_@M1119_"), TEXT("_@M1120_"));
	// LKTOKEN  _@M1121_ = "Home Distance", _@M1122_ = "HomeDis"
	SetDataOption(60, ugDistance,       TEXT("_@M1121_"), TEXT("_@M1122_"));
	// LKTOKEN  _@M1123_ = "Task Speed", _@M1124_ = "TskSp"
	SetDataOption(61, ugTaskSpeed,      TEXT("_@M1123_"), TEXT("_@M1124_"));
	// LKTOKEN  _@M1125_ = "AA Delta Time", _@M1126_ = "AAdT"
	SetDataOption(62, ugNone,           TEXT("_@M1125_"), TEXT("_@M1126_"));
	// LKTOKEN  _@M1127_ = "Thermal All", _@M1128_ = "Th.All"
	SetDataOption(63, ugVerticalSpeed,  TEXT("_@M1127_"), TEXT("_@M1128_"));

	SetDataOption(64, ugNone,  TEXT("_@M1695_"), TEXT("_@M1695_"));

	// LKTOKEN  _@M1131_ = "Battery Percent", _@M1132_ = "Battery"
	SetDataOption(65, ugNone,           TEXT("_@M1131_"), TEXT("_@M1132_"));
	// LKTOKEN  _@M1133_ = "Task Req.Efficiency", _@M1134_ = "TskReqE"
	SetDataOption(66, ugNone,           TEXT("_@M1133_"), TEXT("_@M1134_"));
	// LKTOKEN  _@M1135_ = "Alternate1 Req.Efficiency", _@M1136_ = "Atn1.E"
	SetDataOption(67, ugNone,           TEXT("_@M1135_"), TEXT("_@M1136_"));
	// LKTOKEN  _@M1137_ = "Alternate2 Req.Efficiency", _@M1138_ = "Atn2.E"
	SetDataOption(68, ugNone,           TEXT("_@M1137_"), TEXT("_@M1138_"));
	// LKTOKEN  _@M1139_ = "BestAltern Req.Efficiency", _@M1140_ = "BAtn.E"
	SetDataOption(69, ugNone,           TEXT("_@M1139_"), TEXT("_@M1140_"));
	// LKTOKEN  _@M1141_ = "Altitude QFE", _@M1142_ = "QFE"
	SetDataOption(70, ugAltitude,       TEXT("_@M1141_"), TEXT("_@M1142_"));
	// LKTOKEN  _@M1143_ = "Average Efficiency", _@M1144_ = "E.Avg"
	SetDataOption(71, ugNone,           TEXT("_@M1143_"), TEXT("_@M1144_"));
	// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
	SetDataOption(72, ugNone,           TEXT("_@M1145_"), TEXT("_@M1146_"));
	// LKTOKEN  _@M1147_ = "Flight Level", _@M1148_ = "FL"
	SetDataOption(73, ugNone,           TEXT("_@M1147_"), TEXT("_@M1148_"));
	// LKTOKEN  _@M1149_ = "Task Covered distance", _@M1150_ = "TskCov"
	SetDataOption(74, ugDistance,       TEXT("_@M1149_"), TEXT("_@M1150_"));
	// LKTOKEN  _@M1151_ = "Alternate1 Arrival", _@M1152_ = "Atn1Arr"
	SetDataOption(75, ugAltitude,       TEXT("_@M1151_"), TEXT("_@M1152_"));
	// LKTOKEN  _@M1153_ = "Alternate2 Arrival", _@M1154_ = "Atn2Arr"
	SetDataOption(76, ugAltitude,       TEXT("_@M1153_"), TEXT("_@M1154_"));
	// LKTOKEN  _@M1155_ = "BestAlternate Arrival", _@M1156_ = "BAtnArr"
	SetDataOption(77, ugAltitude,       TEXT("_@M1155_"), TEXT("_@M1156_"));
	// LKTOKEN  _@M1157_ = "Home Radial", _@M1158_ = "Radial"
	SetDataOption(78, ugNone,           TEXT("_@M1157_"), TEXT("_@M1158_"));
	// LKTOKEN  _@M1159_ "Airspace Horizontal Dist", _@M1160_ "ArSpcH"
	SetDataOption(79, ugDistance,       TEXT("_@M1159_"), TEXT("_@M1160_"));
	// LKTOKEN  _@M1161_ = "Ext.Batt.Bank", _@M1162_ = "xBnk#"
	SetDataOption(80, ugNone,           TEXT("_@M1161_"), TEXT("_@M1162_"));
	// LKTOKEN  _@M1163_ = "Ext.Batt.1 Voltage", _@M1164_ = "xBat1"
	SetDataOption(81, ugNone,           TEXT("_@M1163_"), TEXT("_@M1164_"));
	// LKTOKEN  _@M1165_ = "Ext.Batt.2 Voltage", _@M1166_ = "xBat2"
	SetDataOption(82, ugNone,           TEXT("_@M1165_"), TEXT("_@M1166_"));
	// LKTOKEN  _@M1167_ = "Odometer", _@M1168_ = "Odo"
	SetDataOption(83, ugDistance,       TEXT("_@M1167_"), TEXT("_@M1168_"));
	// LKTOKEN  _@M1169_ = "Altern QNH", _@M1170_ = "aAlt"
	SetDataOption(84, ugInvAltitude,    TEXT("_@M1169_"), TEXT("_@M1170_"));
	// LKTOKEN  _@M1171_ = "Altern AGL", _@M1172_ = "aHAGL"
	SetDataOption(85, ugInvAltitude,    TEXT("_@M1171_"), TEXT("_@M1172_"));
	// LKTOKEN  _@M1173_ = "Altitude GPS", _@M1174_ = "HGPS"
	SetDataOption(86, ugAltitude,       TEXT("_@M1173_"), TEXT("_@M1174_"));
	// LKTOKEN  _@M1175_ = "MacCready Equivalent", _@M1176_ = "eqMC"
	SetDataOption(87, ugVerticalSpeed,  TEXT("_@M1175_"), TEXT("_@M1176_"));
	// LKTOKEN  _@M1177_ = "_Experimental1", _@M1178_ = "Exp1"
	SetDataOption(88, ugNone,           TEXT("_@M1177_"), TEXT("_@M1178_"));
	// LKTOKEN  _@M1179_ = "_Experimental2", _@M1180_ = "Exp2"
	SetDataOption(89, ugNone,           TEXT("_@M1179_"), TEXT("_@M1180_"));

	// Distance OLC
	SetDataOption(90, ugNone,           TEXT("_@M1455_"), TEXT("_@M1456_"));
	// Distance FAI triangle
	SetDataOption(91, ugNone,           TEXT("_@M1457_"), TEXT("_@M1458_"));
	// Distance League
	SetDataOption(92, ugNone,           TEXT("_@M1459_"), TEXT("_@M1460_"));
	// Distance FAI 3 TPs
	SetDataOption(93, ugNone,           TEXT("_@M1461_"), TEXT("_@M1462_"));

	////////  PREDICTED ////////
	// Distance OLC
	SetDataOption(94, ugNone,           TEXT("_@M1463_"), TEXT("_@M1464_"));
	// Distance FAI triangle
	SetDataOption(95, ugNone,           TEXT("_@M1465_"), TEXT("_@M1466_"));
	// Distance FAI 3 TPs
	SetDataOption(96, ugNone,           TEXT("_@M1469_"), TEXT("_@M1470_"));

	// Speed OLC
	SetDataOption(97, ugNone,           TEXT("_@M1471_"), TEXT("_@M1472_"));
	// Speed FAI triangle
	SetDataOption(98, ugNone,           TEXT("_@M1473_"), TEXT("_@M1474_"));
	// Speed League
	SetDataOption(99, ugNone,           TEXT("_@M1475_"), TEXT("_@M1476_"));
	// Speed FAI 3 TPs
	SetDataOption(100, ugNone,           TEXT("_@M1477_"), TEXT("_@M1478_"));

	////////  PREDICTED ////////
	// Speed OLC
	SetDataOption(101, ugNone,           TEXT("_@M1479_"), TEXT("_@M1480_"));
	// Speed FAI triangle
	SetDataOption(102, ugNone,           TEXT("_@M1481_"), TEXT("_@M1482_"));
	// Speed FAI 3 TPs
	SetDataOption(103, ugNone,           TEXT("_@M1485_"), TEXT("_@M1486_"));

	// Score OLC
	SetDataOption(104, ugNone,           TEXT("_@M1487_"), TEXT("_@M1488_"));
	// Score FAI triangle
	SetDataOption(105, ugNone,           TEXT("_@M1489_"), TEXT("_@M1490_"));
	// Score League
	SetDataOption(106, ugNone,           TEXT("_@M1491_"), TEXT("_@M1492_"));
	// FAI 3 TPs currently has no score
	SetDataOption(107, ugNone,           TEXT("_Reserved 3"), TEXT("_@M1494_"));

	////////  PREDICTED ////////
	// Score OLC
	SetDataOption(108, ugNone,           TEXT("_@M1495_"), TEXT("_@M1496_"));
	// Score FAI triangle
	SetDataOption(109, ugNone,           TEXT("_@M1497_"), TEXT("_@M1498_"));
	// FAI 3 TPs currently has no score
	SetDataOption(110, ugNone,           TEXT("_Reserved 4"), TEXT("_@M1502_"));

	// Olc Plus Score
	SetDataOption(111, ugNone,           TEXT("_@M1503_"), TEXT("_@M1504_"));
	// Olc Plus Score Predicted
	SetDataOption(112, ugNone,           TEXT("_@M1505_"), TEXT("_@M1506_"));

	// Flaps
	SetDataOption(113, ugNone,           TEXT("_@M1640_"), TEXT("_@M1641_"));

	// Vertical distance to airspace
	SetDataOption(114, ugNone,           TEXT("_@M1285_"), TEXT("_@M1286_"));

	// LKTOKEN  _@M1644_ = "Home Alt.Arrival", _@M1645_ = "HomeArr"
	SetDataOption(115, ugAltitude,       TEXT("_@M1644_"), TEXT("_@M1645_"));

	// No reason to have abbreviated name, since normally it is the wp name itself
	SetDataOption(116, ugNone, TEXT("_@M1761_"), TEXT("Atn1Brg")); // Alternate1 bearing
	SetDataOption(117, ugNone, TEXT("_@M1762_"), TEXT("Atn2Brg")); // Alternate2 bearing
	SetDataOption(118, ugNone, TEXT("_@M1763_"), TEXT("BstBrg"));  // BestAlternate bearing
	SetDataOption(119, ugDistance, TEXT("_@M1764_"), TEXT("Atn1Dst")); // Alternate1 distance
	SetDataOption(120, ugDistance, TEXT("_@M1765_"), TEXT("Atn2Dst")); // Alternate2 distance
	SetDataOption(121, ugDistance, TEXT("_@M1766_"), TEXT("BstDst"));  // BestAlternate distance

	SetDataOption(122, ugAltitude, TEXT("_@M1767_"), TEXT("_@M1768_"));  // Max Altitude reached
	SetDataOption(123, ugAltitude, TEXT("_@M1769_"), TEXT("_@M1770_"));  // Max Height gained
	SetDataOption(124, ugWindSpeed, TEXT("_@M1771_"), TEXT("_@M1772_")); // Head wind speed
	SetDataOption(125, ugDistance, TEXT("_@M1507_"), TEXT("_@M1508_")); //OLC FAI triangle Distance to close
	SetDataOption(126, ugNone, TEXT("_@M1509_"), TEXT("_@M1510_")); // OLC FAI triangle Distance close %
	SetDataOption(127, ugNone, TEXT("Bank angle"), TEXT("_@M1197_")); // Bank
	SetDataOption(128, ugNone, TEXT("_@M1841_"), TEXT("Atn1Rad")); // Alternate1 radial
	SetDataOption(129, ugNone, TEXT("_@M1842_"), TEXT("Atn2Rad")); // Alternate2 radial
	SetDataOption(130, ugNone, TEXT("_@M1287_"), TEXT("HDG")); // Heading, text is changed in lkprocess
	SetDataOption(131, ugDistance, TEXT("_@M1843_"), TEXT("Atn1 nm")); // Alternate1 distance NMiles
	SetDataOption(132, ugDistance, TEXT("_@M1844_"), TEXT("Atn2 nm")); // Alternate2 distance NMiles
	SetDataOption(133, ugHorizontalSpeed,TEXT("_@M2312_"), TEXT("_@M2313_")); // Speed of maximum efficiency
	SetDataOption(134, ugNone,TEXT("_@M2314_"), TEXT("_@M2315_")); // Target Req. Efficicency
	SetDataOption(135, ugAltitude,TEXT("_@M2323_"), TEXT("_@M2324_"));  // Altitude QNE
	SetDataOption(135, ugAltitude,TEXT("_@M2323_"), TEXT("_@M2324_"));  // Altitude QNE
	SetDataOption(LK_NEXT_DIST_RADIUS, ugDistance, TEXT("_@M2190_"), TEXT("_@M2191_")); // Dist to next Cylinder turnpoint

	//Before adding new items, consider changing NUMDATAOPTIONS_MAX
  static_assert(LK_NEXT_DIST_RADIUS < NUMDATAOPTIONS_MAX, "NUMDATAOPTIONS_MAX are too small");


    // Fill all null string pointer with empty string, avoid to check all time is used.
    for(auto& tag : Data_Options) {
        if(!tag.Description) {
            tag.Description = _T("");
        }
        if(!tag.Title) {
            tag.Title = _T("");
        }
    }
}
