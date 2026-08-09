ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error Set PVSNESLIB_HOME to a PVSnesLib 4.6.0 checkout)
endif

export ROMNAME := earthbound_background_lab
export ROMTITLE := EARTHBOUND BG LAB
export ROMBANKS := 32
export ROMSIZE := 0A

CFLAGS += -I$(CURDIR)/src -I$(CURDIR)/generated

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

OFILES += src/distortion_snes.obj generated/generated_assets.obj generated/data.obj

.PHONY: all clean generate host-test test

all: buildWithSummary
buildActual: generated/generated_assets.obj generated/data.obj $(ROMNAME).sfc

generate:
	node tools/generate-assets.mjs

host-test:
	cc -std=c99 -Wall -Wextra -Werror -Isrc tests/state_test.c src/state.c -o /tmp/earthbound-state-test
	/tmp/earthbound-state-test
	cc -std=c99 -Wall -Wextra -Werror -Isrc tests/distortion_test.c src/distortion.c -o /tmp/earthbound-distortion-test
	/tmp/earthbound-distortion-test
	node tests/generate-assets.test.mjs

test: host-test

clean: cleanBuildRes cleanRom cleanLogs
