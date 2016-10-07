#ifndef CLIMBAVERAGECALCULATOR_H
#define CLIMBAVERAGECALCULATOR_H

#include <math.h>
#include <array>
#include <algorithm>
#include <functional>
#include <assert.h>

template<unsigned average_time, unsigned max_data_rate = 1>
class ClimbAverageCalculator {

  ClimbAverageCalculator(const ClimbAverageCalculator& ) = delete;
  ClimbAverageCalculator& operator=( const ClimbAverageCalculator& ) = delete;    

  static_assert(average_time > 0, "invalide average_time");

public:
  ClimbAverageCalculator(): next_history() { }
  ~ClimbAverageCalculator() {};

  ClimbAverageCalculator(ClimbAverageCalculator&& ) = default;

  double GetAverage(double curTime, double curAltitude);

private:
  #define PRECISION 1000

  struct AltHistoryItem {
    AltHistoryItem() : time(), altitude() { }
    explicit AltHistoryItem(double T, double A ) : time(T*PRECISION), altitude(A*PRECISION) { }
    
    unsigned time;
    int altitude;
  };
  
  typedef std::array<AltHistoryItem, average_time * max_data_rate + 1> history_t;

  typedef typename history_t::const_iterator const_iterator;
  typedef typename history_t::iterator iterator;

  /** 
   * return true if Item1 is inside periode and if Item1 is older then Item2 
   */
  struct compare_time {
    explicit compare_time(double curTime) : _curTime(curTime*PRECISION) {}
    
    bool operator()(const AltHistoryItem& Item1, const AltHistoryItem& Item2 ) {
      return ((Item1.time + average_time) >= _curTime) && Item1.time < Item2.time;
    }
    
    unsigned _curTime;
  };

  history_t history;
  size_t next_history;
};


template<unsigned average_time, unsigned max_data_rate>
double ClimbAverageCalculator<average_time, max_data_rate>::GetAverage(double curTime, double curAltitude) {
  assert(curTime > 0);

  const AltHistoryItem new_history(curTime, curAltitude);
  // add the new sample
  history[next_history] = new_history;

  if(next_history >= history.size()) {
    next_history = 0;
  }  

  const_iterator bestHistory = std::min_element(history.begin(), history.end(), compare_time(curTime));
  if (bestHistory->time < new_history.time) {
    return (double)(new_history.altitude - bestHistory->altitude) / (double)(new_history.time - bestHistory->time);
  }
  return 0;
}

#endif
