#ifndef MESSAGE_H
#define MESSAGE_H

#include <list>

#define MAXMESSAGES 20

enum {
  MSG_UNKNOWN=0,
  MSG_AIRSPACE,
  MSG_USERINTERFACE,
  MSG_GLIDECOMPUTER,
  MSG_COMMS,
  MSG_ALARM,
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
  static void InitFont();

  static void Destroy();
  static void Render();

  static void AddMessage(unsigned tshow, int type, const TCHAR *Text);

  // repeats last non-visible message of specified type (or any message
  // type=0)
  static void Repeat(int type);

  // clears all visible messages (of specified type or if type=0, all)
  static bool Acknowledge(int type);

  static void Lock();
  static void Unlock();

  class ScopeBlockRender {
  public:
    ScopeBlockRender() {
        assert(_Block>=0);
        ++_Block;
    }

    ~ScopeBlockRender() {
        --_Block;
        assert(_Block>=0);
    }

    static bool isBlocked() {
        return (_Block > 0);
    }
    private:
        static int _Block;
  };

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

};


#endif
