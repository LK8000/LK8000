
ifneq ($(USE_SOUND_EXTDEV), y)
 SOUND_FILES = $(wildcard Common/Distribution/LK8000/_System/_Sounds/*.WAV)
 SOUND_FILES += Common/Distribution/LK8000/_System/_Sounds/_SOUNDS
endif

SYSTEM_FILES = $(wildcard Common/Distribution/LK8000/_Configuration/_System/*.TXT)
SYSTEM_FILES += Common/Distribution/LK8000/_System/_SYSTEM
SYSTEM_FILES += Common/Distribution/LK8000/_System/CREDITS.TXT
SYSTEM_FILES += Common/Data/Language/DEFAULT_MENU.TXT

ifeq ($(CONFIG_LINUX),y)
 BITMAP_FILES = $(PNG) 
 BITMAP_FILES += $(MASKED_PNG) 
else
 BITMAP_FILES = $(BITMAP)
 BITMAP_FILES += $(BITMAP_MASK)
endif

BITMAP_FILES += Common/Distribution/LK8000/_System/_Bitmaps/_BITMAPSH

POLAR_FILES = $(wildcard Common/Distribution/LK8000/_Polars/*.plr)
POLAR_FILES += Common/Distribution/LK8000/_Polars/_POLARS

LANGUAGE_FILES = $(wildcard Common/Data/Language/Translations/*.json)
LANGUAGE_FILES += Common/Data/Language/language.json

CONFIG_FILES = Common/Distribution/LK8000/_Configuration/NOTEDEMO.TXT

WAYPOINT_FILES = Common/Distribution/LK8000/_Waypoints/WAYNOTES.TXT


define build_distrib_common
 $(Q)install -m 0755 -d  $(1)/LK8000/_Airspaces
 $(Q)install -m 0755 -d  $(1)/LK8000/_Configuration
 $(Q)install -m 0755 -d  $(1)/LK8000/_Language
 $(Q)install -m 0755 -d  $(1)/LK8000/_Logger
 $(Q)install -m 0755 -d  $(1)/LK8000/_Maps
 $(Q)install -m 0755 -d  $(1)/LK8000/_Polars
 $(Q)install -m 0755 -d  $(1)/LK8000/_Tasks
 $(Q)install -m 0755 -d  $(1)/LK8000/_Waypoints

 $(Q)install -m 0644 $(POLAR_FILES) $(1)/LK8000/_Polars
 $(Q)install -m 0644 $(LANGUAGE_FILES) $(1)/LK8000/_Language
 $(Q)install -m 0644 $(CONFIG_FILES) $(1)/LK8000/_Configuration
 $(Q)install -m 0644 $(WAYPOINT_FILES) $(1)/LK8000/_Waypoints

endef
