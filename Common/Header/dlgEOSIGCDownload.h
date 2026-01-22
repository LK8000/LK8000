/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef _DLGEOSIGCDOWNLOAD_H_
#define _DLGEOSIGCDOWNLOAD_H_


#define PRPGRESS_DLG 1
  void dlgEOSIGCSelectListShowModal() ;
  void AddEOSElement(const TCHAR* Line1, const TCHAR* Line2 , uint32_t size);
  void EOSListFilled(BOOL filled) ;




             
typedef union{
  uint16_t val;
  uint8_t byte[2];
} ConvUnion;               
  
#define REC_TIMEOUT 1000 // receive timeout in ms               
  
#define STX             0x02
#define ACK             0x06
#define SYNC            0x16 // Syncronization byte (deprecated) 1 B
#define GET_LOGGER_INFO 0xC4 // Get logger info 3 B
#define SET_TASK        0xCA // Set task 352 B
#define GET_TASK        0xCB // Get task 3 B
#define SET_CLASS       0xD0 // Set Class 12 B
#define GET_FLIGHT_INFO 0xF0 // Get flight info 4 B
#define GET_FLIGTH_BLK  0xF1 // Get flight block 7 B
#define GET_NO_FLIGHTS  0xF2 // Get number of flights 3 B
#define SEND_RADIO_CMD  0xF3 // Send CMD to Radio Variable
#define SET_OBS_ZONE    0xF4 // Set Obs Zone 31 B
#define GET_OBS_ZONE    0xF5 // Get Obs Zone 4 B

#endif
