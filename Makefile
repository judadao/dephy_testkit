DEPHY_IO_ROOT ?= ../dephy_industrial_io
CC            ?= gcc
CFLAGS        ?= -Wall -Wextra -std=c11 -g
OUTDIR        ?= out

.PHONY: all test io-sim clean

all: io-sim

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/io_sim: tools/io_sim.c \
                  $(DEPHY_IO_ROOT)/src/industrial_io.c \
                  $(DEPHY_IO_ROOT)/platform/posix/io_posix_sim.c \
                  $(DEPHY_IO_ROOT)/include/dephy_industrial_io/industrial_io.h \
                  $(DEPHY_IO_ROOT)/include/dephy_industrial_io/posix_sim.h | $(OUTDIR)
	$(CC) $(CFLAGS) -I$(DEPHY_IO_ROOT)/include \
	    tools/io_sim.c \
	    $(DEPHY_IO_ROOT)/src/industrial_io.c \
	    $(DEPHY_IO_ROOT)/platform/posix/io_posix_sim.c \
	    -o $@

io-sim: $(OUTDIR)/io_sim

test: io-sim
	sh tests/selftest.sh
	sh tests/test_io_sim.sh

clean:
	rm -rf out build_out
