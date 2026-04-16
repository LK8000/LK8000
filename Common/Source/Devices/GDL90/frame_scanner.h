/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/frame_scanner.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include "gdl90.h"

namespace gdl90_parser {
namespace detail {

// Static dispatch filtering:
// 1) `handles` is true when a variant type has a message-id mapping and the
//    dispatcher is callable with that type.
// 2) `handles_id` checks whether that mapped id equals the probed id.
// 3) `handled_by_variant` folds this check across all message alternatives.
// 4) `is_handled_by` exposes the variant-wide check at namespace scope.

template <typename T>
concept has_msg_id = requires {
  msg_id_for<T>::value;
};

template <typename Dispatcher, typename T>
concept dispatch_invocable =
    requires(const Dispatcher& dispatcher, const T& msg) {
      dispatcher(msg);
    };

template <typename Dispatcher, typename T>
concept handles = has_msg_id<T> && dispatch_invocable<Dispatcher, T>;

template <typename Dispatcher, typename T>
[[nodiscard]]
constexpr bool handles_id(uint8_t msg_id) noexcept {
  if constexpr (handles<Dispatcher, T>) {
    return msg_id_for<T>::value == msg_id;
  }
  return false;
}

template <typename Dispatcher, typename Variant>
struct handled_by_variant;

template <typename Dispatcher, typename... Ts>
struct handled_by_variant<Dispatcher, std::variant<Ts...>> {
  [[nodiscard]]
  static constexpr bool eval(uint8_t msg_id) noexcept {
    return (handles_id<Dispatcher, Ts>(msg_id) || ...);
  }
};

template <typename Dispatcher>
[[nodiscard]]
inline bool is_handled_by([[maybe_unused]] uint8_t msg_id) noexcept {
  return detail::handled_by_variant<Dispatcher, message>::eval(msg_id);
}

[[nodiscard]]
inline uint8_t peek_msg_id(detail::frame_data raw) noexcept {
  if (raw.size() < 3) {
    return 0xFF;
  }
  if (raw[1] == ESCAPE_BYTE) {
    return raw.size() >= 4 ? static_cast<uint8_t>(raw[2] ^ ESCAPE_XOR) : 0xFF;
  }
  return raw[1];
}

template<typename Dispatcher>
struct partial_dispatch_visitor {
  Dispatcher& dispatcher;

  template <typename Msg>
  void operator()(const Msg& msg) const {
    if constexpr (dispatch_invocable<Dispatcher, Msg>) {
      dispatcher(msg);
    }
  }
};

}  // namespace detail

/**
 * Frame scanner — feed bytes incrementally; dispatches each complete frame.
 *
 * Typical usage:
 *   gdl90_parser::frame_scanner scanner;
 *   scanner.push<MyDispatcher>(buf, len, arg1, arg2);
 */
class frame_scanner {
  static constexpr size_t MAX_FRAME_SIZE =
      1024;  // GDL90 frames are typically small

 public:
  /**
   * Feed raw bytes and dispatch each complete frame to Dispatcher.
   *  - fast prefilter by message id using is_handled_by<Dispatcher>()
   *  - decode_frame() + partial-dispatch friendly std::visit
   *  - exceptions are propagated to caller
   */
  template <typename Dispatcher, typename... DispatcherArgs>
  void push(detail::frame_data data, DispatcherArgs&&... args) {
    Dispatcher dispatcher(std::forward<DispatcherArgs>(args)...);
    auto on_frame = [&](detail::frame_data frame) {
      process_frame<Dispatcher>(frame, dispatcher);
    };
    push(data, on_frame);
  }

  template <typename Dispatcher, typename... DispatcherArgs>
  void push(const uint8_t* data, size_t len, DispatcherArgs&&... args) {
    push<Dispatcher>(detail::frame_data{data, len},
                     std::forward<DispatcherArgs>(args)...);
  }

  /**
   * Feed raw bytes and call `cb` for each complete, raw frame found.
   */
  template <typename Callback>
  void push(detail::frame_data data, Callback&& cb) {
    for (uint8_t b : data) {
      push_byte(b, cb);
    }
  }

  void reset() noexcept {
    buffer.clear();
    in_frame = false;
  }

 private:
  std::vector<uint8_t> buffer;
  bool in_frame = false;

  template <typename Dispatcher>
  static void process_frame(detail::frame_data frame, Dispatcher& dispatcher) {
    auto msg_id = detail::peek_msg_id(frame);
    if (!detail::is_handled_by<Dispatcher>(msg_id)) {
      return;
    }
    using visitor = detail::partial_dispatch_visitor<Dispatcher>;
    std::visit(visitor(dispatcher), decode_frame(frame));
  }

  template <typename Callback>
  void push_byte(uint8_t b, const Callback& cb) {
    if (b == FLAG_BYTE) {
      if (in_frame && buffer.size() >= 4) {
        buffer.push_back(FLAG_BYTE);
        cb(buffer);
      }
      buffer.clear();
      buffer.push_back(FLAG_BYTE);
      in_frame = true;
    }
    else if (in_frame) {
      if (buffer.size() >= MAX_FRAME_SIZE) {
        // Frame too large, discard and resync
        buffer.clear();
        in_frame = false;
        return;
      }
      buffer.push_back(b);
    }
  }
};

}  // namespace gdl90_parser
