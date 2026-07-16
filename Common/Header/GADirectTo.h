/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights
*/

#ifndef GA_DIRECT_TO_H
#define GA_DIRECT_TO_H

struct NMEA_INFO;
struct DERIVED_INFO;

// --- Calc/Task functions (GADirectTo.cpp) ---

// Slide the XTE origin forward until GPS track aligns with bearing to fix.
void GA_UpdateDirectToOriginForCourseCapture(NMEA_INFO* Basic);

// Check arrival at the off-task fix; transition back to task leg when inside radius.
void GA_CheckDirectToOffTaskArrival(NMEA_INFO* Basic);

// DistanceToNext wrapper: fills WaypointDistance/Bearing/ZoomDistance for the DirectTo fix.
// Returns true if GA DirectTo is active and the calculation was handled (caller must unlock + return).
bool GA_ComputeDirectToDistanceBearing(NMEA_INFO* Basic, DERIVED_INFO* Calculated);

// OverTargets wrapper: returns DirectToWaypointIndex when GA DirectTo is active, else -1.
int GA_GetDirectToNavIndex();

// devGenericAutopilot wrapper: overrides prev/next indices with the DirectTo fix when active.
void GA_ApplyDirectToAutopilotOverride(int& prev_index, int& next_index);

// Find the first remaining task WP (from ActiveTaskPoint onwards) that is logically
// ahead of the given position. Caller must hold CritSec_TaskData.
// Returns the Task array index of that WP, or ActiveTaskPoint as fallback.
int GA_FindNextForwardTaskWP(double from_lat, double from_lon);

// Register the waypoint the pilot is currently browsing in the Target dialog
// (Next/Prev navigation).  While set, GA_GetDirectToNavIndex() returns wp_index
// so the bearing line always points aircraft → browsed fix.
// task_idx is the corresponding Task[] array index (used by GA_GetTargetPanLoopStart).
// Call with both -1 to clear (e.g. when the Target dialog closes).
void GA_SetTargetBrowseWP(int wp_index, int task_idx = -1);

// Returns the task-array index from which the TARGET_PAN leg loop should start
// when a GA browse override is active.  For all non-GA cases returns 'fallback'
// unchanged, so DrawBearing behaviour is identical to the original for gliders.
// Safe to call while LockTaskData() is held (no lock acquired internally).
int GA_GetTargetPanLoopStart(int fallback);

// --- Target dialog functions (dlgTargetGA.cpp) ---

class WndForm;
class WndButton;

// Called at dialog open: saves wf pointer and target_point address.
void GA_InitTargetDialog(WndForm* wf, int* target_point_ptr);

// Called at dialog close: clears saved state and resets GA browse flags.
void GA_ResetTargetDialog();

// True when the pilot is browsing forward task WPs during an off-task Direct To.
bool GA_IsOfftaskBrowsing();

// Returns true if GA allows opening the Target dialog without a valid task WP.
// Sets TaskPoint to -1 when there is no task WP.
bool GA_CanOpenWithoutTask(int& TaskPoint);

// Notify that the user manually selected a different task WP (e.g. from dropdown).
void GA_OnTargetPointSelected();

// Show/hide Prev, Next, DirectTo nav buttons. No-op for non-GA.
void GA_UpdateNavButtons();

// Refresh GA fields: visibility + Dist/ETE/ETA values + off-task title.
// Always safe to call; hides fields for non-GA aircraft.
void GA_RefreshTargetFields();

// Apply GA off-task pan. Returns true if handled (caller should return early).
bool GA_ApplyTargetPanOverride(unsigned& dlgSize);

// Returns landable waypoint index for Approach in GA off-task mode. Returns -1 if not overriding.
int GA_GetApproachWPForTarget();

// Handle Prev/Next button clicks. Returns true if action was taken
// (caller should call RefreshTargetPoint + GA_UpdateNavButtons + ApplyTargetPanIfNeeded).
bool GA_OnTargetPrev();
bool GA_OnTargetNext();

// Handle DirectTo button click (fully self-contained, closes parent form on activation).
void GA_OnTargetDirectTo(WndButton* pWnd);

// --- Dialog functions (dlgDirectToCountdown.cpp) ---

// Task-point Direct To (from Target dialog): counts down, then advances ActiveTaskPoint.
bool ShowDirectToCountdownDialog(int new_tp);

// Off-task waypoint Direct To (from WayQuick): counts down, then sets DirectToWaypointIndex.
bool ShowDirectToOffTaskDialog(int wp_index);

// Pan-mode Direct To: builds Oracle description, then counts down.  The candidate
// position is staged in RESWP_UNUSED (never the live DirectTo target) and only
// promoted to RESWP_PANPOS if the pilot confirms; wp_index is normally
// RESWP_UNUSED but may be a real landable waypoint if pan snapped to one.
bool ShowDirectToFromPanDialog(int wp_index, double pan_lat, double pan_lon);

#endif
