export BUILD_DIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C src/

clean:
	$(MAKE) -C src/ clean
