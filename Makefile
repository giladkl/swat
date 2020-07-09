export BUILD_DIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C src/record

clean:
	$(MAKE) -C src/record clean
