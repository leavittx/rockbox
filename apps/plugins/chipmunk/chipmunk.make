#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

CHIPMUNK_SRCDIR := $(APPSDIR)/plugins/chipmunk
CHIPMUNK_BUILDDIR := $(BUILDDIR)/apps/plugins/chipmunk

CHIPMUNK_SRC := $(call preprocess, $(CHIPMUNK_SRCDIR)/SOURCES)
CHIPMUNK_OBJ := $(call c2obj, $(CHIPMUNK_SRC))

OTHER_SRC += $(CHIPMUNK_SRC)

INCLUDES += -I$(CHIPMUNK_SRCDIR)/include/chipmunk

ifndef SIMVER
ifneq (,$(strip $(foreach tgt,RECORDER ONDIO,$(findstring $(tgt),$(TARGET)))))
    ### lowmem targets
    ROCKS += $(CHIPMUNK_BUILDDIR)/chipmunk.ovl
    CHIPMUNK_OUTLDS = $(CHIPMUNK_BUILDDIR)/chipmunk.link
    CHIPMUNK_OVLFLAGS = -T$(CHIPMUNK_OUTLDS) -Wl,--gc-sections -Wl,-Map,$(basename $@).map
else
    ### all other targets
    ROCKS += $(CHIPMUNK_BUILDDIR)/chipmunk.rock
endif
else
    ### simulator
    ROCKS += $(CHIPMUNK_BUILDDIR)/chipmunk.rock
endif

$(CHIPMUNK_BUILDDIR)/chipmunk.rock: $(CHIPMUNK_OBJ)

$(CHIPMUNK_BUILDDIR)/chipmunk.refmap: $(CHIPMUNK_OBJ)

$(CHIPMUNK_OUTLDS): $(PLUGIN_LDS) $(CHIPMUNK_BUILDDIR)/chipmunk.refmap
	$(call PRINTS,PP $(@F))$(call preprocess2file,$<,$@,-DOVERLAY_OFFSET=$(shell \
		$(TOOLSDIR)/ovl_offset.pl $(CHIPMUNK_BUILDDIR)/chipmunk.refmap))

$(CHIPMUNK_BUILDDIR)/chipmunk.ovl: $(CHIPMUNK_OBJ) $(CHIPMUNK_OUTLDS)
	$(SILENT)$(CC) $(PLUGINFLAGS) -o $(basename $@).elf \
		$(filter %.o, $^) \
		$(filter %.a, $+) \
		-lgcc $(CHIPMUNK_OVLFLAGS)
	$(call PRINTS,LD $(@F))$(OC) -O binary $(basename $@).elf $@
