# Approach – Feature documentation

Feature for planning and displaying the approach to a landable waypoint (airfield/airport). The shipped dialog supports **Direct** approach and runway selection; **traffic circuit** UI and buttons are **not implemented yet** (related drawing code may exist for future use).

## Access

- **Target screen**: **Approach** button next to **OK** (same row). Visible when the Target dialog is open; the reference waypoint is the current task point.
- The Approach button has been removed from the Waypoint details dialog.

## Approach dialog

On opening, the same map rendering as Target is used (centred on the waypoint, same view).

### Overlay layout (Target and Approach)

- **Landscape**: a **vertical strip on the right**. **Width** is defined by the landscape XML (`dlgTarget_L.xml` / `dlgApproach_L.xml`) and positioning code; it is not stretched to full screen width. **Height** matches the **full height of the main map window client area** (`main_window->GetClientRect()`), with the top of the overlay aligned to the map client top, so the panel reaches the **bottom** of the map area (including room for **Approve** on Approach).
- **Portrait**: the panel sits at the **bottom** of the screen; height is kept compact where the code adjusts it (see `dlgTarget.cpp` / `dlgApproach.cpp` and portrait overlay helpers in `dlgTools`).

### Approach type

- **Direct** (only mode exposed in the UI): straight-in approach from the **5 km** point on the runway extended centreline to runway centre. The active choice is highlighted (black border, light background) where applicable.
- **Circuit** (VFR traffic circuit: downwind, base, final): **not available in the dialog yet** — buttons are commented out in `dlgApproach_P.xml` / `dlgApproach_L.xml` and callbacks are disabled in `dlgApproach.cpp`.

### Assigned runway

Two buttons with runway directions (e.g. 09 / 27). The selected runway is highlighted. Required for **Direct** drawing and task creation.

## Map drawing

- **Direct** (when **Direct** is selected and a runway is chosen): segment from **5 km** on the runway extended centreline (approach direction) to **runway centre**.
- **Circuit** (if enabled in a future build): downwind at ~800 m from centre (≈ 30 s at 50 kt), base turn at the intersection with the 45° radial from threshold, base and final to runway centre — only when Circuit mode and side selection exist in the UI.

Drawing for **Direct** is performed when Direct and a runway are selected.

## Track / bearing

- In **Direct** mode, the **track** (bearing line from the aircraft) ends at the **5 km point** (start of the direct leg), not at waypoint centre.
- In **Circuit** mode (when implemented end-to-end), the track would end at waypoint centre.

## Technical details

- **Waypoint**: same graphic representation as in Target (runway + label), also in Approach Pan.
- **WndButton**: `SetSelected(bool)` state to highlight the choice (“reverse” look: raised, black border, highlighted background).
- **Constants**: Direct = 5 km (`DIRECT_5KM_M`); circuit (when used): downwind offset 800 m, base turn 800 m along downwind.
- **Main files**: `DrawApproach.cpp`, `DrawBearing.cpp`, `dlgApproach.cpp`, `dlgTarget.cpp`, `WindowControls.cpp` (SetSelected), XML dialogs `dlgApproach_P.xml` / `dlgApproach_L.xml`.
