.PHONY: clean upload

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*.h) $(shell find lib -type f -name *.h) $(shell find lib -type f -name *.cpp)


.pioenvs/nanoatmega328/firmware.hex: $(SOURCES)
	platformio run

clean:
	platformio run --target clean

upload: .pioenvs/nanoatmega328/firmware.hex
	platformio run -t upload --upload-port $(UPLOAD_PORT)
