VERSION := $(shell cat ../utils/version.txt)

firmware:
	echo ${VERSION}
	PLATFORMIO_BUILD_FLAGS="-DVERSION=\\\"${VERSION}\\\"" platformio run

clean:
	platformio run --target clean