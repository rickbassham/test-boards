clean:
	@rm -Rf build
	-rm firmware.zip
	cd ../dragon-light-firmware && $(MAKE) clean

build:
	@mkdir -p build

fsimage: build
	cd ../dragon-light-web && $(MAKE) clean && $(MAKE) pre-package && $(MAKE) package && $(MAKE) deploy
	cp ../dragon-light-web/dist/*.gz build

firmware: build
	cd ../dragon-light-firmware && $(MAKE) firmware
	cp ../dragon-light-firmware/.pio/build/pico/firmware.bin build/fw.bin
	md5sum build/fw.bin | awk '{ print $$1 }' > build/fw.bin.md5

package:
	cd build && zip ../firmware.zip *.gz fw.bin.md5 fw.bin
