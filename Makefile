.PHONY: clean upload

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*.h)


.pioenvs/nanoatmega328/firmware.hex: $(SOURCES)
	platformio run

clean:
	platformio run --target clean

upload: .pioenvs/nanoatmega328/firmware.hex
	platformio run -t upload
