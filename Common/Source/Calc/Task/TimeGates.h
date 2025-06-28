/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TimeGates.h
 */

#ifndef CALC_TASK_TIMEGATES_H
#define CALC_TASK_TIMEGATES_H

namespace TimeGates {

// parameters saved in task
enum open_type {
    anytime,
    fixed_gates,
    pev_start
};

extern open_type GateType;

// Paraglider's time gates
// ------------------------------

// Open and close time, gate 0  ex. 12:00
// M and H for registry

extern int PGOpenTimeH;
extern int PGOpenTimeM;

extern int PGCloseTimeH;
extern int PGCloseTimeM;

// Interval, in minutes
extern int PGGateIntervalTime;
// How many gates, 1-x
extern int PGNumberOfGates;
// ------------------------------

// -----------------------------
// Glider's PEV Start parameters

// The time you need to wait for your personal start gate to open after you press the PEV button
extern int WaitingTime;

// How long your personal start gate window is open
extern int StartWindow;
// -----------------------------

void ResetSettings();

} // TimeGates

// gates are configured, and used by gliders also? todo
bool UseGates();
int ActiveGate();

// Returns the specified gate time (hours), negative -1 if invalid
int OpenGateTime();
int NextGateTime();

// return the CloseTime of Last Gate
int GateCloseTime();

// Returns the gatetime difference to current local time.
// Positive if gate is in the future.
int NextGateTimeDiff(int utc_time);

// Do we have some gates available, either running right now or in the future?
// Basically mytime <CloseTime...
bool HaveGates(int utc_time);

// returns the current gate we are in, either in the past or in the future.
// It does not matter if it is still valid (it is expired).
// There is ALWAYS an activegate, it cannot be negative!
//   but returns -1 if no gates are configured
int InitActiveGate();
void TriggerPevStart(int utc_time);

// Called when the task is reset
void ResetGates();

// autonomous check for usegates, and current chosen activegate is open, so a valid start
// is available crossing the start sector..
bool ValidGate(int utc_time);

// Notify the user about the current gate state
//   - Remaining time to next open
//   - Open
//   - Closed
void NotifyGateState(int utc_time);


bool PilotEventEnabled();
bool WaitForPilotEvent();

#endif  // CALC_TASK_TIMEGATES_H
