.PHONY: clean upload

SOURCES := $(wildcard src/*)


.pioenvs/nanoatmega328/firmware.hex: $(SOURCES)
	platformio run

clean:
	rm -rf .pioenvs

upload: .pioenvs/nanoatmega328/firmware.hex
	platformio run -t upload
