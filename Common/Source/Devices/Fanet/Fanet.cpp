#include "Fanet.h"
#include "Parser.h"
#include "MessageLog.h"
#include "Comm/device.h"
#include "FlarmCalculations.h"
#include "utils/lookup_table.h"
#include "Units.h"
#include "MathFunctions.h"
#include "utils/charset_helper.h"
#include "Sound/Sound.h"
#include "Message.h"

static_assert(IsLittleEndian(), "Big-Endian Arch is not supported");

extern FlarmCalculations flarmCalculations;

namespace {

/*
 * Convert 24bit signed integer to int32_t
 */
int32_t to_int32_t(const uint8_t *buf) {
  int32_t value;
  ((uint8_t*)&value)[0] = buf[0];
  ((uint8_t*)&value)[1] = buf[1];
  ((uint8_t*)&value)[2] = buf[2];
  ((uint8_t*)&value)[3] = ((buf[2]&0x80) ? 0xFF : 0x00); // Sign extension
  return value;
}

/*
 * Convert 2 Byte to uint16_t
 */
uint16_t to_uint16_t(const uint8_t *buf) {
  uint16_t value;
  ((uint8_t*)&value)[0] = buf[0];
  ((uint8_t*)&value)[1] = buf[1];
  return value;
}

/*
 * Convert 6Byte buffer to Latitude/longitude
 */
void payload_absolut2coord(double &lat, double &lon, const uint8_t *buf) {
  if(buf == nullptr)
    return;
  lat = to_int32_t(&buf[0]) / 93206.0;
  lon = to_int32_t(&buf[3]) / 46603.0;
}

void UpdateName(FLARM_TRAFFIC& traffic) {
  traffic.UpdateNameFlag = false; // clear flag first

  const TCHAR *fname = LookupFLARMDetails(traffic.RadioId);
  if (fname) {
    _tcsncpy(traffic.Name, fname, MAXFLARMNAME);
    const TCHAR *cname = LookupFLARMCn(traffic.RadioId);
    if (cname) {
      int cnamelen = _tcslen(cname);
      if (cnamelen <= MAXFLARMCN) {
        _tcscpy(traffic.Cn, cname);
      } else {
        // else probably it is the Name again, and we create a fake Cn
        traffic.Cn[0] = cname[0];
        traffic.Cn[1] = cname[cnamelen-2];
        traffic.Cn[2] = cname[cnamelen-1];
        traffic.Cn[3] = _T('\0');
      }
    }
    else {
      _tcscpy(traffic.Cn, _T("Err"));
    }
  }
  else {
    // Else we NEED to set a name, otherwise it will constantly search for it over and over..
    _tcscpy(traffic.Name, _T("?"));
    _tcscpy(traffic.Cn, _T("?"));
  }
}

template<typename _Tp, size_t size>
int FanetGetIndex(uint32_t ID, _Tp (&array)[size], bool bEmptyIndex){
  int iEmpyIndex = -1;
  for (size_t i = 0;i < size;i++){
    if (array[i].Time_Fix == 0){
      if ((iEmpyIndex < 0) && (bEmptyIndex)){
        iEmpyIndex = i;
      }
    } else if (array[i].ID == ID){
      //we found the right entry (same cn)
      return i;
    }
  }
  return iEmpyIndex;
}

template<typename _Tp, size_t size>
bool FanetInsert(const _Tp& item, _Tp (&array)[size], double Time){
  int index = FanetGetIndex(item.ID, array, true);
  if (index < 0){
    return false;
  }
  array[index] = item; //copy data to structure
  array[index].Time_Fix = Time; //set time for clearing after nothing is received
  return true;
}

enum class aircraft_t : uint8_t {
  other = 0,
  paraglider = 1,
  hangglider = 2,
  balloon = 3,
  glider = 4,
  powered_aircraft = 5,
  helicopter = 6,
  uav = 7,
};

enum class flarm_aircraft_t : uint16_t {
  unknown = 0,
  glider = 1,
  tow_plane = 2,
  helicopter = 3,
  parachute = 4,
  drop_plane = 5,
  fixed_hang_glider = 6,
  soft_para_glider = 7,
  powered_aircraft = 8,
  jet_aircraft = 9,
  UFO = 0x0A,
  balloon = 0x0B,
  blimp = 0x0C,
  zeppelin = 0x0D,
  UAV =  0x0E,
  static_object = 0x0F
};

constexpr auto aircraft_type_table = lookup_table<aircraft_t, flarm_aircraft_t>({
  { aircraft_t::paraglider, flarm_aircraft_t::soft_para_glider },
  { aircraft_t::hangglider, flarm_aircraft_t::fixed_hang_glider },
  { aircraft_t::balloon, flarm_aircraft_t::balloon },
  { aircraft_t::glider, flarm_aircraft_t::glider },
  { aircraft_t::powered_aircraft, flarm_aircraft_t::powered_aircraft },
  { aircraft_t::helicopter, flarm_aircraft_t::helicopter },
  { aircraft_t::uav, flarm_aircraft_t::UAV },
});

} // namespace

bool GetFanetName(uint32_t ID, const NMEA_INFO &info, TCHAR* szName, size_t size) {
  int index = FanetGetIndex(ID, info.FanetName, false);
  if (index >= 0) {
    _tcsncpy(szName, info.FanetName[index].Name, size);
    return true;
  }
  szName[0] = _T('\0'); // empty out string if name not found.
  return false;
}

bool FanetParseType1Msg(DeviceDescriptor_t* d, NMEA_INFO* pGPS, uint32_t id, const std::vector<uint8_t>& data) {
  /*
    Tracking (Type = 1)
    [recommended interval: floor((#neighbors/10 + 1) * 5s) ]
    Note: Done by app layer of the fanet module

    [Byte 0-2]	Position	(Little Endian, 2-Complement)
    bit 0-23	Latitude 	(Absolute, see below)
    [Byte 3-5]	Position	(Little Endian, 2-Complement)
    bit 0-23	Longitude 	(Absolute, see below)

    [Byte 6-7]	Type		(Little Endian)
    bit 15 		Online Tracking
    bit 12-14	Aircraft Type
          0: Other
          1: Paraglider
          2: Hangglider
          3: Balloon
          4: Glider
          5: Powered Aircraft
          6: Helicopter
          7: UAV
    bit 11		Altitude Scaling 1->4x, 0->1x
    bit 0-10	Altitude in m

    [Byte 8]	Speed		(max 317.5km/h)
    bit 7		Scaling 	1->5x, 0->1x
    bit 0-6		Value		in 0.5km/h		

    [Byte 9]	Climb		(max +/- 31.5m/s, 2-Complement)
    bit 7		Scaling 	1->5x, 0->1x
    bit 0-6		Value		in 0.1m/s

    [Byte 10]	Heading
    bit 0-7		Value		in 360/256 deg

    [optional]
    [Byte 11]	Turn rate 	(max +/- 64deg/s, positive is clock wise, 2-Complement)
    bit 7		Scaling 	1->4x, 0->1x
    bit 0-6		Value 		in 0.25deg/s	

    [optional, if used byte 11 is mandatory as well]
    [Byte 12]	QNE offset 	(=QNE-GPS altitude, max +/- 254m, 2-Complement)
    bit 7		Scaling 	1->4x, 0->1x
    bit 0-6		Value 		in m	  
   */
  uint32_t RadioId = id;

  int flarm_slot = FLARM_FindSlot(pGPS, RadioId);
  if (flarm_slot < 0) {
    // no more slots available,
    DebugLog(_T("... NO SLOTS for Flarm traffic, too many ids!"));
    return FALSE;
  }

  d->nmeaParser.setFlarmAvailable(pGPS);

  FLARM_TRAFFIC &traffic = pGPS->FLARM_Traffic[flarm_slot];

  if (traffic.RadioId != RadioId) {
    traffic = {};
  } else {
    // before changing timefix, see if it was an old target back locked in!
    CheckBackTarget(*pGPS, flarm_slot);
  }

  traffic.RadioId = RadioId;
  traffic.Time_Fix = pGPS->Time;
  traffic.Status = LKT_REAL;

  uint8_t payload_length = data.size();
  if ((payload_length < 11) || (payload_length > 13)) {
    return FALSE;
  }

  payload_absolut2coord(traffic.Latitude, traffic.Longitude, &data[0]);
  uint16_t Type = to_uint16_t(&data[6]);
  uint16_t altitude = Type & 0x7FF;
  // OnlineTracking = (Type & 0x8000);
  if (Type & 0x0800) {
    altitude *= 4;
  }
  traffic.Altitude = altitude;
  traffic.Average30s = flarmCalculations.Average30s(traffic.RadioId, pGPS->Time, traffic.Altitude);

  traffic.Type = static_cast<uint16_t>(aircraft_type_table.get((aircraft_t) ((Type >> 12) & 0x07),
                                                                 flarm_aircraft_t::unknown));

  if (data[8] & 0x80) {
    traffic.Speed = Units::From(unKiloMeterPerHour, ((data[8] & 0x007F) * 5) * 0.5);
  }
  else {
    traffic.Speed = Units::From(unKiloMeterPerHour, data[8] * 0.5);
  }

  uint8_t climb = data[9];
  uint8_t climb2 = (climb & 0x7F) | (climb & (1 << 6)) << 1; //create 2-complement
  if (climb & 0x80) {
    traffic.ClimbRate = climb2 * 5.0 / 10.0;
  } else {
    traffic.ClimbRate = climb2 / 10.0;
  }

  traffic.TrackBearing = AngleLimit360(data[10] * 360. / 255.);

  if (!traffic.Name[0] || traffic.UpdateNameFlag) {
    UpdateName(traffic);
  }
  return TRUE;
}

bool FanetParseType2Msg(DeviceDescriptor_t* d, NMEA_INFO* pGPS, uint32_t id, const std::vector<uint8_t>& data) {
  /*
    Name (Type = 2)
    [recommended interval: every 4min]

    8bit String (of arbitrary length, \0 termination not required)

   */

  FANET_NAME fanetDevice;
  
  fanetDevice.ID = id;

  std::string name = { data.begin(), data.end() };
  from_unknown_charset(name.c_str(), fanetDevice.Name); 

  uint32_t flarmId = fanetDevice.ID;
  if (AddFlarmLookupItem(flarmId, fanetDevice.Name, true)) { //check, if device is already in flarm-database
    int flarm_slot = FLARM_FindSlot(pGPS, flarmId); //check if Flarm is already in List
    if (flarm_slot>=0) {
      pGPS->FLARM_Traffic[flarm_slot].UpdateNameFlag = true;
    }
  }

  FanetInsert(fanetDevice, pGPS->FanetName, pGPS->Time);
  return TRUE;

}

bool FanetParseType3Msg(DeviceDescriptor_t* d, NMEA_INFO* pGPS, uint32_t id, const std::vector<uint8_t>& data) {
  /*
    Message (Type = 3)

    [Byte 0]	Header
    bit 0-7 	Subheader, Subtype (TBD)
          0: Normal Message
          
    8bit String (of arbitrary length)  
   */

  // station ID (3 Bytes)
  uint32_t ID = id;

  std::string msg = { data.begin(), data.end() };

  TCHAR text[150]; // at least (31 + 2 + 80)
  if(!GetFanetName(ID, *pGPS, text)) {
    _stprintf(text, _T("%07X"), ID); // no name, use ID
  }
  _tcscat(text, _T("\r\n"));

  TCHAR* Out = text + _tcslen(text);
  from_unknown_charset(msg.c_str(), Out, Out - text);

  PlayResource(TEXT("IDR_WAV_DRIP")); //play sound
  Message::AddMessage(10000, MSG_COMMS, text); // message time 10s
  return TRUE;
}

bool FanetParseType4Msg(DeviceDescriptor_t* d, NMEA_INFO* pGPS, uint32_t id, const std::vector<uint8_t>& data) {
  /*
    Service (Type = 4)
    [recommended interval: 40sec]

    [Byte 0]	Header	(additional payload will be added in order 6 to 1, followed by Extended Header payload 7 to 0 once defined)
        bit 7		Internet Gateway (no additional payload required, other than a position)
        bit 6		Temperature (+1byte in 0.5 degree, 2-Complement)
        bit 5		Wind (+3byte: 1byte Heading in 360/256 degree, 1byte speed and 1byte gusts in 0.2km/h (each: bit 7 scale 5x or 1x, bit 0-6))
        bit 4		Humidity (+1byte: in 0.4% (%rh*10/4))
        bit 3		Barometric pressure normailized (+2byte: in 10Pa, offset by 430hPa, unsigned little endian (hPa-430)*10)
        bit 2		Support for Remote Configuration (Advertisement)
        bit 1		State of Charge  (+1byte lower 4 bits: 0x00 = 0%, 0x01 = 6.666%, .. 0x0F = 100%)
        bit 0		Extended Header (+1byte directly after byte 0)
    The following is only mandatory if no additional data will be added. Broadcasting only the gateway/remote-cfg flag doesn't require pos information. 
    [Byte 1-3 or Byte 2-4]	Position	(Little Endian, 2-Complement)		
        bit 0-23	Latitude	(Absolute, see below)
    [Byte 4-6 or Byte 5-7]	Position	(Little Endian, 2-Complement)
        bit 0-23	Longitude   (Absolute, see below)
    + additional data according to the sub header order (bit 6 down to 1)

   */

  FANET_WEATHER weather;
  weather.ID = id;

  int index = 1;
  if (data[0] & 0x01){ //check extended header
    ++index;
  }
  payload_absolut2coord(weather.Latitude,weather.Longitude,&data[index]);
  index+=6;
  if ((data[0] >> 6) & 0x01){
    //Temperature (+1byte in 0.5 degree, 2-Complement)
    weather.temp = ((int8_t)data[index++])/2.f;
  }else{
    weather.temp = 0;
  }
  if ((data[0] >> 5) & 0x01){
    //Wind (+3byte: 1byte Heading in 360/256 degree, 1byte speed and 1byte gusts in 0.2km/h (each: bit 7 scale 5x or 1x, bit 0-6))
    weather.windDir =  data[index++] * 360.0f / 256.0f;
    weather.windSpeed = (float)(data[index] & 0x7F);
    if (!((data[index++] >> 7) & 0x01)){
      weather.windSpeed /= 5;
    }
    weather.windSpeed /= 3.6; //convert to m/s
    weather.windGust = (float)(data[index] & 0x7F);
    if (!((data[index++] >> 7) & 0x01)) {
      weather.windGust /= 5;
    }
    weather.windGust /= 3.6; //convert to m/s
  }else{
    weather.windDir = 0;
    weather.windSpeed = 0;
    weather.windGust = 0;
  }
  if ((data[0] >> 4) & 0x01){
    //Humidity (+1byte: in 0.4% (%rh*10/4))
    weather.hum = data[index++] * 4.f / 10.f;
  }else{
    weather.hum = 0;
  }
  if ((data[0] >> 3) & 0x01){
    //Barometric pressure normalized (+2byte: in 10Pa, offset by 430hPa, unsigned little endian (hPa-430)*10)
    uint16_t pPress = data[index++];
    pPress += data[index++] * 256U;
    weather.pressure = (pPress / 10.0f) + 430.0f;
  }else{
    weather.pressure = 0;
  }

  if ((data[0] >> 1) & 0x01){
    //State of Charge  (+1byte lower 4 bits: 0x00 = 0%, 0x01 = 6.666%, .. 0x0F = 100%)
    weather.Battery = (data[index++] & 0x0F) * 100.0f / 15.0f;
  }else{
    weather.Battery = 0;
  }

  FanetInsert(weather,pGPS->FANET_Weather, pGPS->Time); //insert data into weather-structure
  return TRUE;
}

bool FanetParseType7Msg(DeviceDescriptor_t* d, NMEA_INFO* pGPS, uint32_t id, const std::vector<uint8_t>& data) {
  /*
    Ground Tracking (Type = 7)
    [recommended interval: floor((#neighbors/10 + 1) * 5s)]

    [Byte 0-2]	Position	(Little Endian, 2-Complement)
    bit 0-23	Latitude 	(Absolute, see below)
    [Byte 3-5]	Position	(Little Endian, 2-Complement)
    bit 0-23	Longitude 	(Absolute, see below)
    [Byte 6]
    bit 7-4		Type
                          0:    Other
                          1:    Walking
                          2:    Vehicle
                          3:    Bike
                          4:    Boot
                          8:    Need a ride
                          9:    Landed well
                          12:   Need technical support
                          13:   Need medical help
                          14:   Distress call
                          15:   Distress call automatically
                          Rest: TBD
    bit 3-1		TBD
    bit 0		Online Tracking
  */

  d->nmeaParser.setFlarmAvailable(pGPS);

  /*
  TODO: display static object ?
  GeoPoint pos;
  payload_absolut2coord(pos.latitude,pos.longitude,&data[0]);
  uint8_t type = data[6] > 4;
  bool online_tracking = data[6] & 0x01;
  */
  return TRUE;
}

bool FanetParseUnknown(DeviceDescriptor_t* d, NMEA_INFO* pGPS, uint32_t id, const std::vector<uint8_t>& data) {
  DebugLog(_T("Unknown Fanet Message : id = %x, payloadsize %zu"), id, data.size());
  return TRUE;
}
