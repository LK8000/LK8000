#ifndef CALC_TASK_TIMEGATES_H
#define CALC_TASK_TIMEGATES_H

// gates are configured, and used by gliders also? todo
bool UseGates();

// we are inside open and close time so a gate is open
bool IsGateOpen();

// returns the next gate number or -1
int NextGate();

// Returns the specified gate time (hours), negative -1 if invalid
int GateTime(int gate);

// return the CloseTime of Last Gate
int GateCloseTime();

// Returns the gatetime difference to current local time.
// Positive if gate is in the future.
int GateTimeDiff(int gate);

// Returns the current open gate number, 0-x, or -1 (negative) if out of time.
// This is NOT the next start! It tells you if a gate is open right now, within time limits.
int RunningGate();

// Do we have some gates available, either running right now or in the future?
// Basically mytime <CloseTime...
bool HaveGates();

// returns the current gate we are in, either in the past or in the future.
// It does not matter if it is still valid (it is expired).
// There is ALWAYS an activegate, it cannot be negative!
//   but returns -1 if no gates are configured
int InitActiveGate();

// autonomous check for usegates, and current chosen activegate is open, so a valid start
// is available crossing the start sector..
bool ValidGate();

#endif  // CALC_TASK_TIMEGATES_H
