# LK8000 Manual — Addendum: Task (Target) and Approach Screens

*This addendum describes the Task (Target) view and the Approach feature for the LK8000 manual. It may be converted to DOCX and appended to the English manual.*

---

## 1. Task (Target) screen

When viewing a task waypoint in **Target** mode, the map centres on the selected task point and a panel appears (on the right in landscape, at the bottom in portrait). In **landscape**, the panel is a **narrow strip on the right** whose **height matches the full map area** (from the top to the bottom of the map), so controls are not cut off at the bottom.

### 1.1 Panel contents

- **OK** — closes the Target dialog and returns to the map.
- **Approach** — opens the Approach dialog for the current target waypoint.  
  **This button is shown only when the current task point is a landable waypoint** (airfield/airport). For non-landable waypoints it is hidden.
- Runway heading buttons (e.g. **16** / **34**) and other target-related controls, depending on task type.

### 1.2 Opening Approach from Target

Tap **Approach** (when visible). The Approach dialog opens and the map remains centred on the same waypoint with the approach panel visible.

---

## 2. Approach screen

When you open Approach from the Target screen, the **first screen** shows:

- **Map** — centred on the landable waypoint (airfield), with the same style as in Target view. Once **Direct** and a runway are selected, a **blue segment** is drawn on the map from the 5 km point on the extended centreline to the runway centre. The bottom of the map may show distance and “PAN” (e.g. “DIRECT 34 1.5km PAN”).
- **Approach panel** — on the right in landscape (or at the bottom in portrait): title bar with “Approach”; below it the buttons and runway selectors described in 2.1. In **landscape**, the panel runs the **full height of the map area** (same vertical extent as the map), with a fixed **width** for the strip, so **Approve** remains visible above the bottom of the screen.

This is the main Approach view before you tap **Approve**. From here you choose direct approach, select the runway (e.g. 16 / 34), check the blue segment on the map, then tap **Approve** to create the approach task (see 2.3).

### 2.1 Panel layout

- **OK** — closes the Approach dialog and returns to the map (approach overlay is cleared).
- **Direct** — selects direct approach. The active option is highlighted. (A “Circuit” mode is not offered in the current dialog.)
- Two runway heading buttons (e.g. **06** / **34**) — select the approach runway. The selected runway is highlighted.
- **Approve** — lower part of the panel (layout from the dialog). Opens the approval dialog to create the approach task (see below).

### 2.2 Map drawing

When **Direct** is selected and a runway is chosen, the map shows a **blue segment** from a point **5 km** along the extended runway centreline (approach direction) to the **runway centre**. This is the same geometry used for the approach task.

### 2.3 Creating an approach task (Approve)

1. Select **Direct** and the desired runway (e.g. 34).
2. Tap **Approve**.
3. A **popup** appears with the warning:  
   *"LK8000 creates approach task ignoring traffic and terrain. Pilot situational awareness must be at maximum."*
4. Two buttons:
   - **Approve** — confirms. The current task is **replaced** by a two-point approach task:
     - **Point 1**: “DIRECT nn” (e.g. DIRECT 34) — the 5 km point on the extended centreline.
     - **Point 2**: the airfield (runway centre).
   The Approach dialog closes and the map returns to normal navigation with the new task active; the pilot may use the autopilot to follow the approach track.
   - **IGNORE!!** — cancels. The popup closes and no task change is made.

---

## 3. Summary

| Screen      | Purpose |
|------------|---------|
| **Target** | View and edit task waypoint; open Approach for landables via the **Approach** button. |
| **Approach** | Choose direct approach and runway; view the 5 km segment on the map; optionally create the approach task with **Approve** and confirm in the warning popup. |

---

*End of addendum.*
