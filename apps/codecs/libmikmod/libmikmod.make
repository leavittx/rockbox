#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id: Makefile 19082 2008-11-10 23:54:24Z zagor $
#

# libmikmod
MIKMODLIB := $(CODECDIR)/libmikmod.a
MIKMODLIB_SRC := $(call preprocess, $(APPSDIR)/codecs/libmikmod/SOURCES)
MIKMODLIB_OBJ := $(call c2obj, $(MIKMODLIB_SRC))
OTHER_SRC += $(MIKMODLIB_SRC)

$(MIKMODLIB): $(MIKMODLIB_OBJ)
	$(SILENT)$(shell rm -f $@)
	$(call PRINTS,AR $(@F))$(AR) rs $@ $^ >/dev/null 2>&1

MIKMODFLAGS = $(filter-out -O%,$(CODECFLAGS))
MIKMODFLAGS += -I$(APPSDIR)/codecs/libmikmod -w
ifeq ($(CPU),coldfire)
    MIKMODCFLAGS += -O0
else
    MIKMODCFLAGS += -O3
endif

$(CODECDIR)/libmikmod/%.o: $(ROOTDIR)/apps/codecs/libmikmod/%.c
	$(SILENT)mkdir -p $(dir $@)
	$(call PRINTS,CC $(subst $(ROOTDIR)/,,$<))$(CC) $(MIKMODFLAGS) -c $< -o $@

