# LK8000 Tracking Guide for End Users

## Scope

This document summarizes the user-visible tracking changes.

It is written for pilots and end users, with a focus on setup, usage, and practical impact.

---

## What Is New

### Multiple tracking services at the same time

Tracking is no longer limited to a single service profile.
You can now create multiple tracking profiles and use them in parallel.

Examples:
- LiveTrack24 + Skylines
- VLSafe + Traccar
- OsmAnd + LiveTrack24

Why this matters:
- one flight can be shared to multiple platforms
- easier setup for clubs, events, and personal workflows
- more flexibility if different audiences use different tracking services

### New support for OsmAnd and Traccar

Two additional tracking platforms are now available:
- OsmAnd
- Traccar

Main setup fields:
- Interval
- Server URL
- Device ID

Why this matters:
- easier integration with general tracking ecosystems
- useful for club servers, private servers, or fleet-style monitoring

### Improved LiveTrack24 reliability

LiveTrack24 handling has been reworked internally to improve stability.

Expected user impact:
- fewer intermittent tracking problems
- more predictable behavior during long flights
- better overall reliability when tracking is active

### Clearer tracking interface

The tracking interface has been improved to make configuration easier to understand.

Visible improvements:
- clearer localized dialog captions
- icon-based presentation for supported tracking services
- improved visual consistency across platforms
- corrected FFVL image availability on Android builds

Why this matters:
- easier service recognition at a glance
- cleaner configuration screens
- more consistent user experience across devices

---

## Quick Setup

1. Open the Tracking configuration page.
2. Add a new profile.
3. Select the target platform.
4. Enter the required fields for that service.
5. Repeat the process for each additional tracking service you want to use.
6. Review the profile list and test before flight.

Typical fields by service:
- LiveTrack24: server, port, user, password, interval, tracking mode, radar
- skylines.aero: ID, interval, radar
- VLSafe: tracking key
- OsmAnd and Traccar: server URL, device ID, interval

---

## Tracking Mode

For services that support it, two operating modes are available:

- `in Flight Only (default)`
- `permanent`

Recommended usage:
- Use `in Flight Only` for normal flying.
- Use `permanent` only when you need to test tracking on the ground and a valid GPS fix is available.

---

## Migration From Older Settings

Older tracking settings are migrated to the new profile-based format.

What this means in practice:
- existing tracking settings should be preserved when possible
- moving to the new multi-profile setup should be smoother

---

## Known Limits

- Some tracking services may depend on optional features enabled in the build.
- Real-time tracking still depends on GPS availability and network quality.
- Ground testing may not reflect full in-flight behavior for all services.

---

## Recommended Checks After Updating

After updating, verify the following:
- the expected tracking profiles are present
- the interval is correct for each service
- server URLs, IDs, and keys are correct
- position reports are received by each target platform
- both flight mode and ground test mode behave as expected

---

## Summary

LK8000 tracking is now more flexible, easier to configure, and better suited to real-world use with multiple simultaneous tracking services, including new support for OsmAnd and Traccar.

