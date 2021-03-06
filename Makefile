# Project Name
TARGET = birds-and-the-bees

# Sources
CPP_SOURCES += \
../Epiphyte/vibrato.cpp \
../Epiphyte/tapesaturator.cpp \
../Epiphyte/scale.cpp \
../Epiphyte/slewLimiter.cpp \
birds-and-the-bees.cpp \


# Library Locations
LIBDAISY_DIR = ../libDaisy
DAISYSP_DIR = ../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# C_INCLUDES += \
# -I. \

C_INCLUDES += -I../Terrarium # Include terrarium.h
C_INCLUDES += -I../Epiphyte # Include Epiphyte
