# Contributing: PR package — Target / Approach overlays (KOBO & Linux)

Use this document when opening a **pull request** on GitHub and when posting on the **forum** ([postfrontal.com — LK8000](https://www.postfrontal.com/forum/default.asp?CAT_ID=11)) so reviewers can approve the feature quickly.

---

## Suggested PR title

```
KOBO/Linux: fix Target & Approach map overlays (geometry, pan, docs)
```

---

## PR description (copy into GitHub)

### Summary

Improves **Task (Target)** and **Approach** dialogs when shown as map overlays: correct alignment to the **map client** (not screen Y=0), stable **strip width**, **full-height** strip in landscape so controls (including **Approve**) stay visible, and consistent **map pan / refresh** behaviour. Portrait-specific tightening remains where intended.

### Problem

- In **landscape**, overlays could show **gaps** above/below the panel, map visible behind the strip, or a **too-short** strip so bottom buttons were **clipped** or missing.
- **Target pan** in landscape could get out of sync with the real overlay size.

### Solution

- Position overlays using **`main_window->GetClientRect()`**: **`SetTop(rc.top)`**, **full client height** for the strip (landscape), **right-aligned** strip with existing **KOBO** horizontal inset; **width unchanged** (XML + same `dlgSize` logic).
- **Approach**: removed dynamic bottom-anchoring of **Approve** that created a large empty band; rely on dialog layout / XML positions after the form is resized.
- **WndForm**: client area sync after resize where needed (`SetCaption` / related fixes) so inner height matches outer height.
- **Map**: refresh/pan hooks aligned with overlay usage (see file list below).
- **Docs**: `Docs/Approach.md` and `Docs/Approach_Manual_Addendum.md` updated (landscape strip height, Circuit not in UI yet).

### How to test

**KOBO (primary)**

- [ ] **Landscape**: open **Target** on a task point — strip on the **right**, no gap at **top** or **bottom** of map area; **OK** / **Approach** usable.
- [ ] **Landscape**: open **Approach** — same strip width as Target; **Approve** visible and tappable; no huge empty gap above **Approve**.
- [ ] **Portrait**: Target and Approach — panel at bottom, no clipped buttons; map pan/zoom acceptable.

**LINUX (optional greyscale build)**

- [ ] Same flows with `TARGET=LINUX` if you use the same overlay code paths.

### Documentation

- `Docs/Approach.md` — feature + overlay layout + Circuit status.
- `Docs/Approach_Manual_Addendum.md` — manual addendum aligned with UI.

### Risk / scope

- Touches **dialog layout** and **map pan**; behaviour is intended for **KOBO / Linux** paths exercised by the branch. Other targets should be unaffected if guarded by the same `#ifdef` / platform checks as before — **confirm in review**.

---

## Files typically involved (adjust to your `git diff`)

| Area | Files |
|------|--------|
| Dialogs | `Common/Source/Dialogs/dlgTarget.cpp`, `dlgApproach.cpp`, `dlgTools.cpp`, `Common/Header/dlgTools.h` |
| Forms | `Common/Source/WindowControls.cpp`, `Common/Header/WindowControls.h` |
| Map / pan | `Common/Source/Draw/MapWindowZoom.cpp`, `MapWindow_Utils.cpp`, possibly `OrigAndOrient.cpp` |
| Orientation | `Common/Source/LKInterface/CScreenOrientation.cpp` (if part of the same feature branch) |
| XML | `Common/Data/Dialogs/dlgTarget_P.xml`, `dlgApproach_P.xml` (and landscape variants if touched) |
| Docs | `Docs/Approach.md`, `Docs/Approach_Manual_Addendum.md` |

---

## Maintainer checklist (for merge)

- [ ] CI / build for **KOBO** (and **LINUX** if applicable) passes.
- [ ] No unintended changes to **non-touch** targets (PNA, Android, etc.).
- [ ] **CREDITS.TXT** (distribution) updated for the release that includes this work — see snippet below.

---

## Snippet for `CREDITS.TXT` / release notes

Merge under the appropriate `[version x.y.z]` block (wording can be shortened by release manager):

```
* KOBO / Linux :
 - Target & Approach : map overlay aligned to map client; landscape strip full height; Approve visible; docs updated (Approach.md).
```

If the release already contains a generic line such as “Map layout in task target dialog”, replace or merge to avoid duplication.

---

## Forum announcement (short)

**English**

> We have a PR that fixes the **Target** and **Approach** side panels on **KOBO** (landscape): the overlay now matches the map area top-to-bottom, keeps the correct width, and the **Approve** button is visible again. Portrait behaviour is kept compact. Docs are in `Docs/Approach.md`. Please test on device if you can and comment on the PR or here.

**Italiano**

> C’è una PR che sistema i pannelli **Target** e **Approach** sul **KOBO** in orizzontale: l’overlay coincide con l’area mappa dall’alto al basso, la larghezza resta quella giusta e il tasto **Approve** torna visibile. Il portrait resta compatto. Documentazione aggiornata in `Docs/Approach.md`. Se potete, provate su dispositivo e lasciate feedback sulla PR o qui.

---

## Screenshots (recommended for community buy-in)

Attach **before/after** photos or QEMU captures showing:

1. Landscape **Target** — strip full height, no map leak at top.
2. Landscape **Approach** — **Approve** visible.

---

## Aligning [PR #1687](https://github.com/LK8000/LK8000/pull/1687) (draft → ready for review)

Use this when **marking the PR “Ready for review”** after KOBO validation:

1. **Edit the PR description** so the top says the feature is verified on **KOBO** (landscape + portrait), not only Linux; point to `Docs/Approach.md` and `Approach_Manual_Addendum.md`.
2. **Resolve or reply** to CodeRabbit threads: `Default.lkt` **rules inside `<options>`** (parser requirement), `dlgApproach` **globals only after successful XML load**, `DrawApproach` gated on **`MODE_APPROACH_PAN`**, **Direct** label (already English in XML), manual **autopilot** wording (revised in addendum).
3. **Clear draft** on GitHub once CI is green and you are happy with on-device tests.

Short paragraph to paste under “Summary” on the PR:

> **KOBO:** Overlays use the map window **client rect** (full-height strip in landscape, correct top alignment). Earlier drafts behaved on desktop Linux but mis-sized on Kobo; this is addressed in the latest commits on `feature/approach`. **Code review follow-ups:** task rules XML structure, approach globals on dialog load failure, draw only in approach-pan mode, selected-button outline uses hollow brush, shared landable check for Approach button.

---

*End of PR package.*
