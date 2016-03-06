#ifndef MESSAGE_H
#define MESSAGE_H

#define MAXMESSAGES 20

enum {
  MSG_UNKNOWN=0,
  MSG_AIRSPACE,
  MSG_USERINTERFACE,
  MSG_GLIDECOMPUTER,
  MSG_COMMS
};


struct Message_t {
  tstring text;
  int type;
  unsigned tstart; // time message was created
  unsigned texpiry; // time message will expire
  unsigned tshow; // time message is visible for
};

class WndMessage;

class Message {
 public:
  static void Initialize(RECT rc);
  static void Destroy();
  static bool Render(); // returns true if messages have changed

  static void AddMessage(unsigned tshow, int type, const TCHAR *Text);

  // repeats last non-visible message of specified type (or any message
  // type=0)
  static void Repeat(int type);

  // clears all visible messages (of specified type or if type=0, all)
  static bool Acknowledge(int type);

  static void Lock();
  static void Unlock();

  static void BlockRender(bool doblock);

 private:
  typedef std::list<Message_t> messages_t;

  static messages_t messages; // from older to newer
  static messages_t messagesHistory; // from newer to older
  static RECT rcmsg; // maximum message size
  static WndMessage WndMsg;
  static tstring msgText;
  static void Resize();
  static bool hidden;
  static int nvisible;
  static int block_ref;

};


#endif
