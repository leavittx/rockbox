#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

BALLS_SRCDIR := $(APPSDIR)/plugins/balls
BALLS_BUILDDIR := $(BUILDDIR)/apps/plugins/balls

BALLS_SRC := $(call preprocess, $(BALLS_SRCDIR)/SOURCES)
BALLS_OBJ := $(call c2obj, $(BALLS_SRC))

OTHER_SRC += $(BALLS_SRC)

ifndef SIMVER
ifneq (,$(strip $(foreach tgt,RECORDER ONDIO,$(findstring $(tgt),$(TARGET)))))
    ### lowmem targets
    ROCKS += $(BALLS_BUILDDIR)/balls.ovl
    BALLS_OUTLDS = $(BALLS_BUILDDIR)/balls.link
    BALLS_OVLFLAGS = -T$(BALLS_OUTLDS) -Wl,--gc-sections -Wl,-Map,$(basename $@).map
else
    ### all other targets
    ROCKS += $(BALLS_BUILDDIR)/balls.rock
endif
else
    ### simulator
    ROCKS += $(BALLS_BUILDDIR)/balls.rock
endif

$(BALLS_BUILDDIR)/balls.rock: $(BALLS_OBJ)

$(BALLS_BUILDDIR)/balls.refmap: $(BALLS_OBJ)

$(BALLS_OUTLDS): $(PLUGIN_LDS) $(BALLS_BUILDDIR)/balls.refmap
	$(call PRINTS,PP $(@F))$(call preprocess2file,$<,$@,-DOVERLAY_OFFSET=$(shell \
		$(TOOLSDIR)/ovl_offset.pl $(BALLS_BUILDDIR)/balls.refmap))

$(BALLS_BUILDDIR)/balls.ovl: $(BALLS_OBJ) $(BALLS_OUTLDS)
	$(SILENT)$(CC) $(PLUGINFLAGS) -o $(basename $@).elf \
		$(filter %.o, $^) \
		$(filter %.a, $+) \
		-lgcc $(BALLS_OVLFLAGS)
	$(call PRINTS,LD $(@F))$(OC) -O binary $(basename $@).elf $@
