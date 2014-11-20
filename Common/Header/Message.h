#ifndef MESSAGE_H
#define MESSAGE_H

#include "Poco/Timespan.h"



#define MAXMESSAGES 20

enum {
  MSG_UNKNOWN=0,
  MSG_AIRSPACE,
  MSG_USERINTERFACE,
  MSG_GLIDECOMPUTER,
  MSG_COMMS
};


struct singleMessage {
  TCHAR text[1000];
  int type;
  Poco::Timespan tstart; // time message was created
  Poco::Timespan texpiry; // time message will expire
  Poco::Timespan tshow; // time message is visible for
};

class WndMessage;

class Message {
 public:
  static void Initialize(RECT rc);
  static void Destroy();
  static bool Render(); // returns true if messages have changed

  static void AddMessage(DWORD tshow, int type, const TCHAR *Text);

  // repeats last non-visible message of specified type (or any message
  // type=0)
  static void Repeat(int type);

  // clears all visible messages (of specified type or if type=0, all)
  static bool Acknowledge(int type);

  static void Lock();
  static void Unlock();

  static void CheckTouch(HWND wmControl);

  static void BlockRender(bool doblock);

 private:
  static struct singleMessage messages[MAXMESSAGES];
  static RECT rcmsg; // maximum message size
  static WndMessage WndMsg;
  static TCHAR msgText[2000];
  static void Resize();
  static unsigned GetEmptySlot();
  static bool hidden;
  static int nvisible;
  static int block_ref;

};


#endif
