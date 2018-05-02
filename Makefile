# this flag selects the rack version to compile against.
# 
# possible values are v040 v_050_dev

# FLAGS += -D v040
FLAGS += -D v_050_dev

SOURCES = $(wildcard src/*.cpp portaudio/*.c)

include ../../plugin.mk

FLAGS += -Iportaudio

dist: all
	mkdir -p    dist/vcvrecorder
	cp LICENSE* dist/vcvrecorder/
	cp plugin.* dist/vcvrecorder/
	cp -R res   dist/vcvrecorder/
