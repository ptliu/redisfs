BUILD_DIR=./build
VCPKG_DIR=./vcpkg_files

.PHONY: all configure build clean cleanall init

all: configure build

configure:
	cmake -B "${BUILD_DIR}" -S .

build:
	cmake --build "${BUILD_DIR}"

clean: 
	cmake --build "${BUILD_DIR}" --target clean

cleanall:
	rm -rf "${BUILD_DIR}"

init:
	git submodule update --init --recursive
	${VCPKG_DIR}/bootstrap-vcpkg.sh
	ln --symbolic --force -T "${VCPKG_DIR}/vcpkg" "./vcpkg"
