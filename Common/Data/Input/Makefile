

FILES = \
	../../Header/InputEvents_LK8000.cpp \
	../../Header/InputEvents_Text2Event.cpp 

# ==============================================================================
build: $(FILES)

../../Header/InputEvents_LK8000.cpp : lk8000.xci xci2cpp.pl
	perl xci2cpp.pl lk8000.xci > $@

../../Header/InputEvents_Text2Event.cpp : ../../Header/InputEvents.h h2cpp.pl
	perl h2cpp.pl > $@

