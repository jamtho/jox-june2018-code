SHELL=/bin/bash -o pipefail
.DELETE_ON_ERROR:


##  --------------------------------------------------------------------------
##  Building each class file

JAVAS := $(shell find src/ -name "*.java" -type f)
CLASSES := $(patsubst src/%.java,bin/classes/%.class,$(JAVAS))

bin/classes/%.class: src/%.java
	javac -g -Werror -Xlint:all -d bin/classes -sourcepath src $<


##  --------------------------------------------------------------------------
##  Utils

all-classes: $(CLASSES)

clean:
	rm -rf bin/classes/*


##  ----------------------------------------------------------------------
##  Flags for running the JVM

# ## Java 8
# GCDIAGFLAGS := -XX:+PrintGCDetails -XX:+PrintTenuringDistribution  \
#                -XX:+PrintGCTimeStamps -XX:+PrintGCDateStamps
## Java 9
GCDIAGFLAGS := -Xlog:gc*

JITDIAGFLAGS := -XX:+UnlockDiagnosticVMOptions      \
                -XX:+PrintCompilation   -XX:+CITime
####                 -XX:+PrintAssembly
JITDIAGFLAGs :=
##JITDIAGFLAGS := -XX:+LogCompilation -XX:+TraceClassLoading 


DISABLEEA := -XX:-DoEscapeAnalysis

#### This is the final set we use:
JAVA_FLAGS := -cp bin/classes            \
              -XX:+PreserveFramePointer  \
              # $(GCDIAGFLAGS)             \
              #$(JITDIAGFLAGS)            \




##  ----------------------------------------------------------------------
##  Runing progams

AISMMF_FILE := ../../../../ais_data/aismmf/cefas/april_2018/2017.40mm.aismmf
SAR_FILE    := ../sar_data/sar_detections.csv.postime.arrbin

run-AISMMFTimestampRange: $(CLASSES)
	time  \
	java $(JAVA_FLAGS)  -Xmx5G  \
		joxmar.mains.AISMMFTimestampRange  \
		$(AISMMF_FILE)

run-CorrelateAisSarSimple: $(CLASSES)
	time  \
	java $(JAVA_FLAGS)  -Xmx5G    \
		joxmar.mains.CorrelateAisSarSimple \
		$(AISMMF_FILE) $(SAR_FILE)     \
#	  | tee CORRELATE_OUTPUT.csv
