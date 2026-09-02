#ifndef MESSAGE_H
#define MESSAGE_H

#include <cassert>
#include <chrono>
#include <list>
#include <optional>
#include "Screen/Point.hpp"
#include "Util/tstring.hpp"
#include "tchar.h"

#define MAXMESSAGES 20

enum {
  MSG_UNKNOWN=0,
  MSG_AIRSPACE,
  MSG_USERINTERFACE,
  MSG_GLIDECOMPUTER,
  MSG_COMMS,
  MSG_ALARM,
};


class WndMessage;

class Message {
 public:
  using steady_clock = std::chrono::steady_clock;
  using time_point = steady_clock::time_point;
  using optional_time_point = std::optional<time_point>;
  using duration_ms = std::chrono::duration<uint32_t, std::milli>;

  static void Initialize(PixelRect rc);
  static void InitFont();

  static void Destroy();
  static void Render();

  static void AddMessage(unsigned tshow, int type, const TCHAR *text);

  // repeats last non-visible message of specified type (or any message
  // type=0)
  static void Repeat(int type);

  // clears all visible messages (of specified type or if type=0, all)
  static bool Acknowledge(int type);

  static void Lock();
  static void Unlock();

  struct Message_t {        
    Message_t(const tstring& text, int type, duration_ms tshow)
        : text(text), type(type), tshow(tshow) {}
    
    tstring text;
    int type = 0;
    optional_time_point tstart; // time message was created (absolute time)
    duration_ms tshow = {};     // duration message is visible for

    bool pending() const {
        return !tstart.has_value();
    }

    time_point expire() const {
        if(tstart.has_value()) {
            return *tstart + tshow;
        }
        return time_point::min(); // or some appropriate default for expired time
    }
  };

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
  using messages_t = std::list<Message_t>;

  static messages_t messages; // from older to newer
  static messages_t messagesHistory; // from newer to older
  static PixelRect rcmsg; // maximum message size
  static WndMessage WndMsg;
  static tstring msgText;
  static void Resize();
  static bool hidden;
};


#endif
