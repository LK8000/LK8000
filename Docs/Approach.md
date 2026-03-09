# Approach – Feature documentation

Feature for planning and displaying the approach to a landable waypoint (airfield/airport) with a choice between **direct** approach and **traffic circuit**.

## Access

- **Target screen**: **Approach** button next to **OK** (same row). Visible when the Target dialog is open; the reference waypoint is the current task point.
- The Approach button has been removed from the Waypoint details dialog.

## Approach dialog

On opening, the same map rendering as Target is used (centred on the waypoint, same view).

### Approach type (two buttons)

- **Direct**: straight-in approach from the 5 km point on the runway extended centreline to runway centre.
- **Circuit**: VFR traffic circuit (downwind, base, final).

The active choice is highlighted (black border, light background) to show the current mode.

### Assigned runway

Two buttons with runway directions (e.g. 09 / 27). The selected runway is highlighted. Required to draw both direct and circuit.

### Circuit: side

When **Circuit** is selected, **Left** and **Right** appear for the circuit side. The active choice is highlighted here too.

## Map drawing

- **Direct** (only when the Direct button and a runway are selected): segment from **5 km** on the runway extended centreline (approach direction) to **runway centre**.
- **Circuit** (only when Circuit, a runway and a side are selected):  
  - Downwind at ~800 m from centre (≈ 30 s at 50 kt).  
  - Base turn at the intersection with the 45° radial from threshold.  
  - Base and final to runway centre.

Drawing is performed only when the required choices are complete (Direct + runway, or Circuit + runway + side).

## Track / bearing

- In **Direct** mode, the **track** (bearing line from the aircraft) ends at the **5 km point** (start of the direct leg), not at waypoint centre.
- In **Circuit** mode, the track ends at waypoint centre.

## Technical details

- **Waypoint**: same graphic representation as in Target (runway + label), also in Approach Pan.
- **WndButton**: `SetSelected(bool)` state to highlight the choice (“reverse” look: raised, black border, highlighted background).
- **Constants**: Direct = 5 km (`DIRECT_5KM_M`); circuit: downwind offset 800 m, base turn 800 m along downwind.
- **Main files**: `DrawApproach.cpp`, `DrawBearing.cpp`, `dlgApproach.cpp`, `dlgTarget.cpp`, `WindowControls.cpp` (SetSelected), XML dialogs `dlgApproach_P.xml` / `dlgApproach_L.xml`.
