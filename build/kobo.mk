ifeq ($(TARGET_IS_KOBO),y)

FONTS_DIR ?= Common/Data/Fonts
FONTS_NAMES =	DejaVuSansCondensed \
		DejaVuSansCondensed-Bold \
		DejaVuSansCondensed-Oblique \
		DejaVuSansCondensed-BoldOblique \
		DejaVuSansMono
	
FONTS_FILES = $(patsubst %,$(FONTS_DIR)/%.ttf,$(FONTS_NAMES))

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

KOBO_POWER_OFF_BIN = PowerOff

KOBO_POWER_OFF_SOURCES = \
    $(SRC)/kobo/PowerOff.cpp \
    $(SRC)/xcs/Kobo/Model.cpp \
    $(LIB)/MathFunctions.cpp	\
    $(XCS_SCREEN) \
    $(XCS_EVENT) \
    $(XCS_OS)

KOBO_POWER_OFF_OBJ     = \
    $(patsubst $(SRC)%.cpp,$(BIN)%.o,$(KOBO_POWER_OFF_SOURCES)) \
    $(BIN)/poco.a 


KOBO_KERNEL_SOURCES = \
    kobo/kernel/uImage-E50610 \
    kobo/kernel/uImage-E60Q90 \
    kobo/kernel/uImage-E606B0 \


KOBO_KERNEL = \
 $(patsubst kobo/kernel/%, LK8000/kobo/%, $(KOBO_KERNEL_SOURCES))


define build-kobofs
$(Q)rm -rf $(BIN)/$(1)/KoboRoot

$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/drivers	
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/drivers/mx6sl-ntx
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/drivers/mx6sl-ntx/usb
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/drivers/mx6sl-ntx/usb/gadget
$(Q)install -m 0644 kobo/modules/mx6sl-ntx/usb/gadget/g_serial.ko $(BIN)/$(1)/KoboRoot/drivers/mx6sl-ntx/usb/gadget

$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/drivers/mx50-ntx
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/drivers/mx50-ntx/usb
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/drivers/mx50-ntx/usb/gadget
$(Q)install -m 0644 kobo/modules/mx50-ntx/usb/gadget/g_serial.ko $(BIN)/$(1)/KoboRoot/drivers/mx50-ntx/usb/gadget

$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/etc
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/opt/LK8000/bin
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/opt/LK8000/lib
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/opt/LK8000/lib/kernel
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/opt/LK8000/share/fonts
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/opt/LK8000/share/_System/_Bitmaps
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Airspaces
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Configuration
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Language
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Logger
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Maps
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Polars
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Tasks
$(Q)install -m 0755 -d  $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Waypoints
$(Q)install -m 0644 kobo/inittab $(BIN)/$(1)/KoboRoot/etc
$(Q)install -m 0755 $(OUTPUTS) $(KOBO_MENU_BIN) $(KOBO_POWER_OFF_BIN) kobo/rcS $(BIN)/$(1)/KoboRoot/opt/LK8000/bin
$(Q)install --strip --strip-program=$(STRIP) -m 0755 $(KOBO_SYS_LIB_PATHS) $(BIN)/$(1)/KoboRoot/opt/LK8000/lib
$(Q)install -m 0644 $(FONTS_FILES) $(BIN)/$(1)/KoboRoot/opt/LK8000/share/fonts
$(Q)install -m 0644 $(SYSTEM_FILES) $(BIN)/$(1)/KoboRoot/opt/LK8000/share/_System
$(Q)install -m 0644 $(BITMAP_FILES) $(BIN)/$(1)/KoboRoot/opt/LK8000/share/_System/_Bitmaps
$(Q)install -m 0644 $(POLAR_FILES) $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Polars
$(Q)install -m 0644 $(LANGUAGE_FILES) $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Language
$(Q)install -m 0644 $(CONFIG_FILES) $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Configuration
$(Q)install -m 0644 $(WAYPOINT_FILES) $(BIN)/$(1)/KoboRoot/mnt/onboard/LK8000/_Waypoints
endef


Kobo-install-otg.zip: $(BIN)/otg/.kobo/KoboRoot.tgz
	@$(NQ)echo "  Zip     $@"
	$(Q)cd $(BIN)/otg/ && zip -q $@ .kobo/KoboRoot.tgz
	$(Q)cp $(BIN)/otg/$@ $@


Kobo-install.zip: $(BIN)/std/.kobo/KoboRoot.tgz
	@$(NQ)echo "  Zip     $@"
	$(Q)cd $(BIN)/std/ && zip -q $@ .kobo/KoboRoot.tgz
	$(Q)cp $(BIN)/std/$@ $@


# /mnt/onboard/.kobo/KoboRoot.tgz is a file that is picked up by
# /etc/init.d/rcS, extracted to / on each boot; we can use it to
# install LK8000
$(BIN)/std/.kobo/KoboRoot.tgz: $(OUTPUTS) $(KOBO_MENU_BIN) $(KOBO_POWER_OFF_BIN) \
	$(SYSTEM_FILES) $(BITMAP_FILES) \
	$(FONTS_FILES) $(POLAR_FILES) $(LANGUAGE_FILES) \
	$(CONFIG_FILES)  kobo/inittab kobo/rcS
	@$(NQ)echo "  TAR     $@"
	
	$(call build-kobofs,std)
	$(Q)install -m 0755 -d  $(BIN)/std/.kobo
	$(Q)fakeroot tar czfC $@ $(BIN)/std/KoboRoot .

$(BIN)/otg/.kobo/KoboRoot.tgz: $(OUTPUTS) $(KOBO_MENU_BIN) $(KOBO_POWER_OFF_BIN) \
	$(SYSTEM_FILES) $(BITMAP_FILES) \
	$(FONTS_FILES) $(POLAR_FILES) $(LANGUAGE_FILES) \
	$(CONFIG_FILES)  kobo/inittab kobo/rcS kobo/60-persistent-serial.rules $(KOBO_KERNEL_SOURCES)
	@$(NQ)echo "  TAR     $@"
	
	$(call build-kobofs,otg)

	$(Q)install -m 0755 -d  $(BIN)/otg/KoboRoot/lib/udev/rules.d
	$(Q)install -m 0644 kobo/60-persistent-serial.rules $(BIN)/otg/KoboRoot/lib/udev/rules.d
	$(Q)install -m 0755 -d  $(BIN)/otg/KoboRoot/mnt/onboard/LK8000/kobo
	$(Q)install -m 0644 $(KOBO_KERNEL_SOURCES) $(BIN)/otg/KoboRoot/mnt/onboard/LK8000/kobo

	$(Q)install -m 0755 -d  $(BIN)/otg/.kobo
	$(Q)fakeroot tar czfC $@ $(BIN)/otg/KoboRoot .


$(KOBO_POWER_OFF_BIN) : $(KOBO_POWER_OFF_BIN)_ns
	@$(NQ)echo "  STRIP   $@"
	$(Q)$(STRIP) $< -o $@
	$(Q)$(SIZE) $@

$(KOBO_POWER_OFF_BIN)_ns : $(KOBO_POWER_OFF_OBJ)
	@$(NQ)echo "  LINK    $@"
	$(Q)$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

endif
