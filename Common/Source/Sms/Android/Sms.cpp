/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sms.h
 * Author: Shachar Liberman
 *
 * Created on 09.09.2017
 */
#include "../Sms.h"
#include "externs.h"
#include "Android/SmsUtil.hpp"
#include "Android/Context.hpp"
#include "Sound/Sound.h"
#include "Dialogs.h"


//char phoneNumbers[160] = "0548841311,0522929386"; //"0548841311,0522929386"; //tab delimited currently //"0546738539";  //period-delimited phone numbers
//char takeOffMessage[160] =  "Hey, I Just Took off! livetrack24.com/user/lshachar";
char SafeLandingMessage[160] =  "I safely landed :)";
char EmergencyLandingMessage[160] =  "Emergency landing. :( please help. livetrack24.com/user/lshachar";
//int LandingDlgTimeoutfixed = 600;


void LKTakeoffSms() {
    LKSms(PhoneNumbers_Config,TakeoffMsg_Config);
    DoStatusMessage("Takeoff message sent",NULL,false); // Takeoff
}

void LKSafeLandingSms() {
    LKSms(PhoneNumbers_Config,SafeLandingMessage);
    DoStatusMessage("Safe landing message sent", NULL, false);  // todo:sms: should only display message if sms was actually sent.
}

void LKEmergencyLandingSms() {
    LKSms(PhoneNumbers_Config,EmergencyLandingMessage);
    DoStatusMessage("EMERGENCY MESSAGE SENT", NULL, false);     // todo:sms: should only display message if sms was actually sent.
    LKSound(_T("TARGVISIBLE.WAV"));
}


void LKSLandingSms() {
  bool emergencyMsgSent = false;
  if (1) { // if emergency landing msg is on: lk should ask pilot how landing went
    if (MessageBoxX("Safe Landing?", "Landing detected", mbYesNo, false,
                          LandingDlgTimeout / 2, IdNo) == IdNo) { //if not safe landing
      LKSound(_T("LK_BOOSTER.WAV"));
      if (MessageBoxX("SEND EMERGENCY MESSAGE TO CONTACTS?", "unsafe landing", mbYesNo, true,
                          LandingDlgTimeout / 2, IdYes) == IdYes) {
        LKEmergencyLandingSms();
        emergencyMsgSent = true;
      }
    }
  }

  if (!emergencyMsgSent) {
    if (1) {         // if lk should send safe landing messages
      LKSafeLandingSms();
    } else {         // do not send landing messages at all
      DoStatusMessage(MsgToken(931), NULL, false); // Landing
    }
  }

}



// Sends SMS through Android phone.
// Todo:sms: get proper sent/not sent sms status from java code
// Todo:sms: keep trying to send message if failed.
void LKSms(const char *phoneNumber, const char *textMessage) {
  if (strcmp(phoneNumber, "") == 0)
    return;           // todo:sms: should return unsent status.

  SmsUtil::SendSms(Java::GetEnv(),phoneNumber, textMessage);
}
