# Airspace Settings Update

The goal is to make airspace setup easier, cleaner, and more relevant to the data loaded on your device.

## What Changed

### 1) New "Used only" filter in Airspace dialogs

In both Airspace dialogs (mode/warnings and colors), there is now a **Used only** option.

- Default: **ON**
- When ON: the list shows only airspace types/classes that are actually present in your loaded airspace files
- When OFF: the list shows the full list of available airspace entries

This reduces clutter and speeds up configuration.

### 2) Airspace settings now work with Class + Type

Airspace handling was reorganized to distinguish and combine:

- Airspace class (for example: Class C, Class D)
- Airspace type/category (for example: CTR, Danger, Restricted)

For display and warning behavior, LK8000 now evaluates both dimensions. In practical terms, an airspace can be affected by its class rule and by its type rule.

This makes configuration more consistent across different input formats and sources.

### 3) Color behavior is aligned with the new model

In the color dialog:

- You can still assign a custom color per entry
- If no custom color is assigned, LK8000 uses the default class/type color
- The UI text **Class Color** is shown for this default-color state

## Selection Algorithm (Class + Type)

For each airspace, LK8000 evaluates both:

- `Class` (e.g. Class D)
- `Type` (e.g. CTR)

### Display decision

An airspace is displayed if **either** the class rule or the type rule is enabled.

- Display = Display(Class) OR Display(Type)

### Warning decision

An airspace can generate warnings if **either** the class rule or the type rule is enabled.

- Warning = Warning(Class) OR Warning(Type)

### Color decision

Color selection uses a strict priority plus inheritance model:

1. If Type is OTHER or NONE, LK8000 directly inherits from Class.
2. Otherwise, LK8000 evaluates Type first.
3. If Type has no resolved color, LK8000 falls back to Class.

For each step above, a color can come from:

1. User custom color (highest priority for that entry).
2. Built-in default color table.

This gives the following effective order:

1. Type custom color
2. Type default color
3. Class custom color
4. Class default color
5. Final renderer safety color (if nothing is resolved)

In practice, Type usually drives the final color, while Class acts as inherited fallback.

### Class/Type heritage

In this context, heritage means a Type can inherit visual attributes from its Class when Type is not explicitly resolved.

- Explicit Type color: no heritage used.
- No Type color: heritage from Class.
- Type = OTHER or NONE: immediate heritage from Class.

### Practical examples

1. If `Class D` display is OFF but `CTR` display is ON, a `Class D CTR` airspace is still displayed.
2. If `Class D` warning is ON but `CTR` warning is OFF, warnings are still active for that airspace.
3. If `CTR` has no custom color but has a default color, this type color is used.
4. If `CTR` has no custom/default color, LK8000 inherits color from `Class D`.
5. If airspace Type is `OTHER` or `NONE`, LK8000 directly uses the class color path.

## What You Will Notice As an End User

- Airspace lists are shorter by default (only relevant entries)
- Less time spent scrolling through unused airspace categories
- Display and warning logic behaves more consistently when airspaces define both class and type information
- Color configuration follows the same class/type logic

## How To Use The New Filter

1. Open Airspace configuration.
2. Locate the **Used only** option.
3. Keep it ON for daily use (recommended).
4. Turn it OFF only if you want to preconfigure categories not currently present in loaded files.

## Existing Profiles And Saved Settings

Airspace settings storage has been modernized internally.

- Profiles can be loaded as usual.
- Airspace display/warning/color settings are now saved with the new class/type-oriented format.

If you maintain shared profile templates, it is recommended to review airspace settings and save them again with this version so they include the new setting layout.

## Quick Recommendation

- Keep **Used only** enabled in normal operations.
- Configure warnings and visibility on entries that matter in your current area.
- Review colors after loading a new country/region dataset, because visible entries can change with loaded airspaces.
