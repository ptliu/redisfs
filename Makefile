BUILD_TYPE?=Release
TIMEOUT?=1m

BUILD_DIR=./build
VCPKG_DIR=./vcpkg_files

.PHONY: all configure build clean cleanall init run test

all: configure build test

configure:
	cmake -B "${BUILD_DIR}" -S . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}

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

run:
	${BUILD_DIR}/redisfs

test:
	timeout --verbose ${TIMEOUT} ${BUILD_DIR}/redisfs_test
