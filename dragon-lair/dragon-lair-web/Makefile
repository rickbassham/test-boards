FILES := $(wildcard dist/*/*.gz) $(wildcard dist/*/*/main*.gz) $(wildcard dist/*.gz)
.PHONY: clean build purgecss package pre-package

dist:
	mkdir -p ./dist

pre-package:
	gzip -f -r dist

package: dist
	zip ./dist/filesystem.zip $(FILES) -x \*.map.gz -x \*LICENSE*.gz -x \*status.json.gz
	md5sum ./dist/filesystem.zip | cut -d " " -f1 > ./dist/filesystem.zip.md5

purgecss:
	purgecss --css build/static/css/*.css --content build/index.html build/static/js/*.js --output build/static/css

build:
	npm run build

clean:
	rm -Rf ./dist/*
	rm -Rf ./build/*
