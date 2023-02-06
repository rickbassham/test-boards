.PHONY: clean docker-build docker-images

clean:
	@rm -Rf build

build: clean
	@mkdir build && cd build && cmake .. && cmake --build . && cpack .

docker-images:
	docker build --no-cache --build-arg UBUNTU_VERSION=focal -t darkdragons/indi-build:focal -f ubuntu-amd64.Dockerfile .
	docker build --no-cache --build-arg UBUNTU_VERSION=impish -t darkdragons/indi-build:impish -f ubuntu-amd64.Dockerfile .

docker-build:
	docker run -v `pwd`:/src -v `pwd`/output:/output darkdragons/indi-build:focal /bin/bash -c "cmake /src && cmake --build . && cpack . && cp *.deb /output"
	docker run -v `pwd`:/src -v `pwd`/output:/output darkdragons/indi-build:impish /bin/bash -c "cmake /src && cmake --build . && cpack . && cp *.deb /output"
