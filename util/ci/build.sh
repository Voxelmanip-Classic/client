#! /bin/bash -e

cmake -B build \
	-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-Debug} \
	-DRUN_IN_PLACE=TRUE \
	${CMAKE_FLAGS}

cmake --build build --parallel $(($(nproc) + 1))
