ifeq ($(TARGET_IS_KOBO),y)

BITSTREAM_VERA_DIR ?= Common/Data/Fonts
BITSTREAM_VERA_NAMES = Vera VeraBd VeraIt VeraBI VeraMono
BITSTREAM_VERA_FILES = $(patsubst %,$(BITSTREAM_VERA_DIR)/%.ttf,$(BITSTREAM_VERA_NAMES))

SYSROOT = $(shell $(CC) -print-sysroot)

# install our version of the system libraries in /opt/LK8000/lib; this
# is necessary because:
# - we cannot link statically because we need NSS (name service
#   switch) modules
# - Kobo's stock glibc may be incompatible on some older firmware
#   versions
KOBO_SYS_LIB_NAMES = \
	libstdc++.so.6 \
	libm.so.6 \
	ld-linux-armhf.so.3 \
	libc.so.6 \
	libgcc_s.so.1 \
	librt.so.1 \
	libpthread.so.0	\
	
KOBO_SYS_LIB_PATHS = $(addprefix $(SYSROOT)/lib/,$(KOBO_SYS_LIB_NAMES))

KOBO_SYS_LIB_PATHS += $(KOBO)/lib/libzzip-0.so.13
KOBO_SYS_LIB_PATHS += $(KOBO)/lib/libz.so.1
KOBO_SYS_LIB_PATHS += $(KOBO)/lib/libpng16.so.16
KOBO_SYS_LIB_PATHS += $(KOBO)/lib/libfreetype.so.6


# /mnt/onboard/.kobo/KoboRoot.tgz is a file that is picked up by
# /etc/init.d/rcS, extracted to / on each boot; we can use it to
# install LK8000
KoboRoot.tgz: $(OUTPUTS) $(KOBO_MENU_BIN) $(KOBO_POWER_OFF_BIN) \
	$(SYSTEM_FILES) $(BITMAP_FILES) \
	$(BITSTREAM_VERA_FILES) $(POLAR_FILES) $(LANGUAGE_FILES) \
	$(CONFIG_FILES)  kobo/inittab kobo/rcS
	@$(NQ)echo "  TAR     $@"
	$(Q)rm -rf $(BIN)/KoboRoot
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/etc
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/opt/LK8000/bin 
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/opt/LK8000/lib/
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/opt/LK8000/share/fonts 
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/opt/LK8000/share/_System/_Bitmaps 
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/mnt/onboard/LK8000/_Airspaces 
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/mnt/onboard/LK8000/_Configuration 
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/mnt/onboard/LK8000/_Language 
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/mnt/onboard/LK8000/_Logger 
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/mnt/onboard/LK8000/_Maps
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/mnt/onboard/LK8000/_Polars 
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/mnt/onboard/LK8000/_Tasks
	$(Q)install -m 0755 -d  $(BIN)/KoboRoot/mnt/onboard/LK8000/_Waypoints
	$(Q)install -m 0644 kobo/inittab $(BIN)/KoboRoot/etc
	$(Q)install -m 0755 $(OUTPUTS) $(KOBO_MENU_BIN) $(KOBO_POWER_OFF_BIN) kobo/rcS $(BIN)/KoboRoot/opt/LK8000/bin
	$(Q)install -m 0755 $(KOBO_SYS_LIB_PATHS) $(BIN)/KoboRoot/opt/LK8000/lib
	$(Q)install -m 0644 $(BITSTREAM_VERA_FILES) $(BIN)/KoboRoot/opt/LK8000/share/fonts
	$(Q)install -m 0644 $(SYSTEM_FILES) $(BIN)/KoboRoot/opt/LK8000/share/_System
	$(Q)install -m 0644 $(BITMAP_FILES) $(BIN)/KoboRoot/opt/LK8000/share/_System/_Bitmaps
	$(Q)install -m 0644 $(POLAR_FILES) $(BIN)/KoboRoot/mnt/onboard/LK8000/_Polars
	$(Q)install -m 0644 $(LANGUAGE_FILES) $(BIN)/KoboRoot/mnt/onboard/LK8000/_Language
	$(Q)install -m 0644 $(CONFIG_FILES) $(BIN)/KoboRoot/mnt/onboard/LK8000/_Configuration
	$(Q)install -m 0644 $(WAYPOINT_FILES) $(BIN)/KoboRoot/mnt/onboard/LK8000/_Waypoints
	$(Q)fakeroot tar czfC $@ $(BIN)/KoboRoot .

endif
