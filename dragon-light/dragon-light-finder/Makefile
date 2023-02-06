BINARY_NAME=dragon-light-finder

build:
	GOARCH=amd64 GOOS=darwin go build -o ./build/${BINARY_NAME}-osx-amd64 main.go
	GOARCH=arm64 GOOS=darwin go build -o ./build/${BINARY_NAME}-osx-arm64 main.go
	GOARCH=amd64 GOOS=linux go build -o ./build/${BINARY_NAME}-linux-amd64 main.go
	GOARCH=arm64 GOOS=linux go build -o ./build/${BINARY_NAME}-linux-arm64 main.go
	GOARCH=arm GOOS=linux go build -o ./build/${BINARY_NAME}-linux-arm main.go
	GOARCH=amd64 GOOS=windows go build -o ./build/${BINARY_NAME}-windows-amd64.exe main.go

clean:
	rm -Rf build
