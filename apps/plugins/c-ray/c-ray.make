#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

C-RAYSRCDIR := $(APPSDIR)/plugins/c-ray
C-RAYBUILDDIR := $(BUILDDIR)/apps/plugins/c-ray

ROCKS += $(C-RAYBUILDDIR)/c-ray.rock

C-RAY_SRC := $(call preprocess, $(C-RAYSRCDIR)/SOURCES)
C-RAY_OBJ := $(call c2obj, $(C-RAY_SRC))

# add source files to OTHERSRC to get automatic dependencies
OTHER_SRC += $(C-RAY_SRC)

$(C-RAYBUILDDIR)/c-ray.rock: $(C-RAY_OBJ) $(CODECDIR)/libtlsf.a

##C-RAYFLAGS = $(PLUGINFLAGS) \
##             -DFIXEDPOINT -DSTATIC -DPD -DUSEAPI_ROCKBOX
