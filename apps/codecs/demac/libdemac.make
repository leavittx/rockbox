#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

# libdemac
DEMACLIB := $(CODECDIR)/libdemac.a
DEMACLIB_SRC := $(call preprocess, $(APPSDIR)/codecs/demac/libdemac/SOURCES)
DEMACLIB_OBJ := $(call c2obj, $(DEMACLIB_SRC))
OTHER_SRC += $(DEMACLIB_SRC)
ifeq ($(CPU),arm)
OTHER_SRC += $(APPSDIR)/codecs/demac/libdemac/udiv32_arm-pre.S
endif
DEMACLIB_PRE := $(subst .a,-pre.a,$(DEMACLIB))
DEMACLIB_OBJ_PRE := $(subst udiv32_arm.o,udiv32_arm-pre.o,$(DEMACLIB_OBJ))

$(DEMACLIB_PRE): $(DEMACLIB_OBJ_PRE)
	$(SILENT)$(shell rm -f $@)
	$(call PRINTS,AR $(@F))$(AR) rcs $@ $^ >/dev/null

$(DEMACLIB): $(DEMACLIB_OBJ)
	$(SILENT)$(shell rm -f $@)
	$(call PRINTS,AR $(@F))$(AR) rcs $@ $^ >/dev/null

DEMACFLAGS = $(filter-out -O%,$(CODECFLAGS))

ifeq ($(CPU),coldfire)
    DEMACFLAGS += -O2
else
    DEMACFLAGS += -O3
endif

$(CODECDIR)/ape_free_iram.h: $(CODECDIR)/ape-pre.map
	$(call PRINTS,GEN $(@F))perl -an \
		-e 'if(/^PLUGIN_IRAM/){$$istart=hex($$F[1]);$$ilen=hex($$F[2])}' \
		-e 'if(/iend = /){$$iend=hex($$F[0]);}' \
		-e '}{if($$ilen){print"#define FREE_IRAM ".($$ilen+$$istart-$$iend)."\n";}' \
		$(CODECDIR)/ape-pre.map \
		> $@

$(CODECDIR)/demac/%.o: $(ROOTDIR)/apps/codecs/demac/%.c
	$(SILENT)mkdir -p $(dir $@)
	$(call PRINTS,CC $(subst $(ROOTDIR)/,,$<))$(CC) $(DEMACFLAGS) -c $< -o $@
