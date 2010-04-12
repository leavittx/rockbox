#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

BALLS_SRCDIR := $(APPSDIR)/plugins/goban
BALLS_BUILDDIR := $(BUILDDIR)/apps/plugins/goban

BALLS_SRC := $(call preprocess, $(BALLS_SRCDIR)/SOURCES)
BALLS_OBJ := $(call c2obj, $(BALLS_SRC))

OTHER_SRC += $(BALLS_SRC)

ifndef SIMVER
ifneq (,$(strip $(foreach tgt,RECORDER ONDIO,$(findstring $(tgt),$(TARGET)))))
    ### lowmem targets
    ROCKS += $(BALLS_BUILDDIR)/goban.ovl
    BALLS_OUTLDS = $(BALLS_BUILDDIR)/goban.link
    BALLS_OVLFLAGS = -T$(BALLS_OUTLDS) -Wl,--gc-sections -Wl,-Map,$(basename $@).map
else
    ### all other targets
    ROCKS += $(BALLS_BUILDDIR)/goban.rock
endif
else
    ### simulator
    ROCKS += $(BALLS_BUILDDIR)/goban.rock
endif

$(BALLS_BUILDDIR)/goban.rock: $(BALLS_OBJ)

$(BALLS_BUILDDIR)/goban.refmap: $(BALLS_OBJ)

$(BALLS_OUTLDS): $(PLUGIN_LDS) $(BALLS_BUILDDIR)/goban.refmap
	$(call PRINTS,PP $(@F))$(call preprocess2file,$<,$@,-DOVERLAY_OFFSET=$(shell \
		$(TOOLSDIR)/ovl_offset.pl $(BALLS_BUILDDIR)/goban.refmap))

$(BALLS_BUILDDIR)/goban.ovl: $(BALLS_OBJ) $(BALLS_OUTLDS)
	$(SILENT)$(CC) $(PLUGINFLAGS) -o $(basename $@).elf \
		$(filter %.o, $^) \
		$(filter %.a, $+) \
		-lgcc $(BALLS_OVLFLAGS)
	$(call PRINTS,LD $(@F))$(OC) -O binary $(basename $@).elf $@
