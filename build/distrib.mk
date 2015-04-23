
SOUND_FILES = $(wildcard Common/Distribution/LK8000/_System/_Sounds/*.WAV)
SOUND_FILES += Common/Distribution/LK8000/_System/_Sounds/_SOUNDS

SYSTEM_FILES = $(wildcard Common/Distribution/LK8000/_Configuration/_System/*.TXT)
SYSTEM_FILES += Common/Distribution/LK8000/_System/_SYSTEM
SYSTEM_FILES += Common/Distribution/LK8000/_System/CREDITS.txt
SYSTEM_FILES += Common/Data/Language/DEFAULT_MENU.TXT

BITMAP_FILES = $(PNG) 
BITMAP_FILES += Common/Distribution/LK8000/_System/_Bitmaps/_BITMAPSH

POLAR_FILES = $(wildcard Common/Distribution/LK8000/_Polars/*.plr)
POLAR_FILES += Common/Distribution/LK8000/_Polars/_POLARS

LANGUAGE_FILES = $(wildcard Common/Data/Language/Translations/*.TXT)
LANGUAGE_FILES += $(wildcard Common/Data/Language/Translations/*.LNG)
LANGUAGE_FILES += Common/Data/Language/ENGLISH.LNG
LANGUAGE_FILES += Common/Data/Language/ENG_MSG.TXT
LANGUAGE_FILES += Common/Data/Language/ENG_HELP.TXT
LANGUAGE_FILES += Common/Data/Language/_LANGUAGE

CONFIG_FILES = Common/Distribution/LK8000/_Configuration/NOTEPAD.txt

WAYPOINT_FILES = Common/Distribution/LK8000/_Waypoints/WAYNOTES.txt
