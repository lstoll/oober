.PHONY: clean upload


.pioenvs/nanoatmega328/firmware.hex: src/main.ino
	platformio run

clean:
	rm -rf .pioenvs

upload: .pioenvs/nanoatmega328/firmware.hex
	platformio run -t upload
