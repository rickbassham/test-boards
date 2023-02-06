FILES := $(wildcard dist/*.gz)
.PHONY: clean package pre-package deploy

dist:
	mkdir -p ./dist

pre-package: dist
	cp -r ./src/* ./dist
	gzip -f -r dist
	rm ./dist/status.json.gz

package: dist
	mkdir -p filesystem
	zip ./filesystem/filesystem.zip $(FILES) -x \*status.json.gz
	md5sum ./filesystem/filesystem.zip | cut -d " " -f1 > ./filesystem/filesystem.zip.md5

clean:
	rm -Rf ./dist/*
	rm -Rf ./filesystem/*

deploy:
	rm -Rf ../dragon-light-firmware/data/*
	cp -r ./dist/* ../dragon-light-firmware/data
