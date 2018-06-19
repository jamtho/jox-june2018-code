SHELL=/bin/bash -o pipefail
.DELETE_ON_ERRR:


### Default target
all: bin/aismmf_timestamp_range        \
     bin/aismmf_servetracks_zmqreqrep  \
     bin/correlate_ais_sar_simple      \
     bin/aismmf_get_mmsi               \
     bin/aismmf_mmsis


##  ----------------------------------------------------------------------
##  Compiler config

CFLAGS := $(CFLAGS) -g -Wall -Wextra -Werror -O3 -fno-omit-frame-pointer


##  ----------------------------------------------------------------------
##  Header class files

HEADS := $(shell find src/ -name "*.h" -type f)


##  ----------------------------------------------------------------------
##  Main targets

bin/aismmf_timestamp_range: src/main_aismmf_timestamp_range.c  \
                               $(HEADS)
	$(CC) $(CFLAGS) -o $@ $< -lczmq -lm

bin/aismmf_servetracks_zmqreqrep: src/main_aismmf_servetracks_zmqreqrep.c  \
                               $(HEADS)
	$(CC) $(CFLAGS) -o $@ $< -lczmq -lm

bin/correlate_ais_sar_simple: src/main_correlate_ais_sar_simple.c  \
                               $(HEADS)
	$(CC) $(CFLAGS) -o $@ $< -lczmq -lm

bin/aismmf_get_mmsi: src/main_aismmf_get_mmsi.c  \
                     $(HEADS)
	$(CC) $(CFLAGS) -o $@ $< -lczmq -lm

bin/aismmf_mmsis: src/main_aismmf_mmsis.c  \
                     $(HEADS)
	$(CC) $(CFLAGS) -o $@ $< -lczmq -lm


##  ----------------------------------------------------------------------
##  Utility targets

clean:
	rm -f bin/aismmf_timestamp_range        \
	      bin/aismmf_servetracks_zmqreqrep  \
	      bin/correlate_ais_sar_simple      \
	      bin/aismmf_get_mmsi               \
	      bin/aismmf_mmsis

## -- Timestamp range finder

AISMMF_FILE := ../../../../ais_data/aismmf/cefas/april_2018/2017.40mm.aismmf

## -- Checking what's in an aismmfile

run-aismmf_timestamp_range: bin/aismmf_timestamp_range
	time $< $(AISMMF_FILE)

check-aismmf_timestamp_range: bin/aismmf_timestamp_range
	valgrind $< $(AISMMF_FILE)

run-aismmf_mmsis: bin/aismmf_mmsis
	time $< $(AISMMF_FILE)

## -- ZMQ track server

run-aismmf_servetracks_zmqreqrep: bin/aismmf_servetracks_zmqreqrep
	$< $(AISMMF_FILE) 898989

check-aismmf_servetracks_zmqreqrep: bin/aismmf_servetracks_zmqreqrep
	valgrind $< $(AISMMF_FILE) 898989

## -- AIS/SAR correlation, straight lines version

SAR_FILE := ../sar_data/sar_detections.csv.postime.arrbin

run-correlate_ais_sar_simple: bin/correlate_ais_sar_simple
	time $< $(AISMMF_FILE) $(SAR_FILE)  \
#		| tee CORRELATE_OUTPUT.csv
