.PHONY: clean upload

SOURCES := $(wildcard src/*)


.pioenvs/nanoatmega328/firmware.hex: $(SOURCES)
	platformio run

clean:
	platformio run --target clean

upload: .pioenvs/nanoatmega328/firmware.hex
	platformio run -t upload
