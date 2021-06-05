BUILD_DIR=./build

.PHONY: all configure build clean cleanall init

all: configure build

configure:
	cmake -B "${BUILD_DIR}" -S . -DCMAKE_TOOLCHAIN_FILE="./vcpkg/scripts/buildsystems/vcpkg.cmake"

build:
	cmake --build "${BUILD_DIR}"

clean: 
	cmake --build "${BUILD_DIR}" --target clean

cleanall:
	rm -rf "${BUILD_DIR}"

init:
	./vcpkg/bootstrap-vcpkg.sh
