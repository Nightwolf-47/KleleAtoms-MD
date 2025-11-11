SGDK_VERSION=1.80
SGDK_FOLDER=SGDK

.PHONY: compile	build
compile:
	docker run -it --rm \
	-v "${PWD}":/workdir \
	--user $(id -u):$(id -g) \
	sgdk:${SGDK_VERSION}

pull:
	git submodule update --init --remote --recursive
	cd ${SGDK_FOLDER} && git checkout tags/v${SGDK_VERSION}

build-sgdk:	pull
	cd ${SGDK_FOLDER} && docker build . -t sgdk:${SGDK_VERSION}

rm-sgdk:
	rm -rf ${SGDK_FOLDER}

shell:
	docker run -it --rm -v "${PWD}":/workdir -w /workdir --entrypoint=/bin/bash sgdk:${SGDK_VERSION} 

clean:
	rm -rf out/*

build:
	mkdir -p build && \
	cd build && \
	cmake -DSGDK_VERSION=${SGDK_VERSION} ..  && \
	make