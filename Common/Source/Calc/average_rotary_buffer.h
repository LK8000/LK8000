#ifndef _calc_average_rotary_buffer_h_
#define _calc_average_rotary_buffer_h_
#include <list>
#include <optional>

template<typename DataT>
class average_rotary_buffer {
public:
  average_rotary_buffer() = default;

  void init(double time) {
    buffer_data.clear();
    sum_data = {};
    period = time;
  }

  bool empty() const {
    return buffer_data.empty();
  }

  void insert(double time, DataT&& new_data) {
    sum_data += new_data;
    buffer_data.push_back({ time, std::move(new_data) });

    while (!buffer_data.empty()) {
      const auto& old_data = buffer_data.front();
      if ((old_data.time + period) <= time) {
        sum_data -= old_data.data;
        buffer_data.pop_front();
      } else {
        return;
      }
    }
  }

  std::optional<DataT> average() const {
    double delta_time = buffer_data.back().time - buffer_data.front().time;
    if (delta_time <= 0.1) {
      return {};
    }
    return sum_data / delta_time;
  }

  const DataT& sum() const {
    return sum_data;
  }

private:
  struct TimedDataT {
    double time;
    DataT data;
  };

  std::list<TimedDataT> buffer_data;
  DataT sum_data = {};

  double period = 15; //second;
};

#endif // _calc_average_rotary_buffer_h_
