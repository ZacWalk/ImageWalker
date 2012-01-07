# ##########################################################################################
# Copyright 2008 Adobe Systems Incorporated
# All Rights Reserved.
#
# NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with
# the terms of the Adobe license agreement accompanying it.
#
# ##########################################################################################

# Define internal use variables.

Error =

Sample = ${NAME}

ifeq "${Sample}" ""
	Sample = ${name}
endif

ifeq "${Sample}" ""
	Error += The sample name must be provided
endif

# case tolerance
ifeq "${Sample}" "dumpfile"
	Sample = dumpfile/main
endif

TargetOS = ${OS}

ifeq "${TargetOS}" ""
	TargetOS = ${os}
endif

ifeq "${TargetOS}" ""
	TargetOS = ${MACHTYPE}${OSTYPE}
endif

ifeq "${TargetOS}" "i386linux"	# Linux ${MACHTYPE}${OSTYPE} is i386linux.
	TargetOS = i80386linux
endif

ifeq "${TargetOS}" "linux"
	TargetOS = i80386linux
endif

ifeq "${TargetOS}" "solaris"
	TargetOS = sparcsolaris
endif

ifneq "${TargetOS}" "i80386linux"
	ifneq "${TargetOS}" "sparcsolaris"
		Error += Invalid target OS "${TargetOS}"
	endif
endif

TargetStage = ${STAGE}

ifeq "${TargetStage}" ""
	TargetStage = ${stage}
endif

ifeq "${TargetStage}" ""
	TargetStage = debug
endif

ifneq "${TargetStage}" "debug"
	ifneq "${TargetStage}" "release"
		Error += Invalid target stage "${TargetStage}"
	endif
endif

ifeq "${TargetStage}" "debug"
   LibSuffix = StaticDebug
endif

ifeq "${TargetStage}" "release"
   LibSuffix = StaticRelease
endif

BuildRoot  = ../..
TargetRoot = ${BuildRoot}/target/${TargetOS}/${TargetStage}
TempRoot   = ${BuildRoot}/intermediate/${TargetOS}/${TargetStage}

# NB: this is SDKRoot to be exakt
XMPRoot    = ${BuildRoot}/..

LibXMP = ${XMPRoot}/public/libraries/${TargetOS}/${TargetStage}/libXMPCore${LibSuffix}.a

################################################################################################

CC  = gcc
CPP = gcc -x c++
LD  = gcc

CPPFlags =  -fexceptions -funsigned-char -fPIC -Wno-multichar -Wno-implicit -Wno-ctor-dtor-privacy
CPPFlags += -DUNIX_ENV=1 -D_FILE_OFFSET_BITS=64

LDFlags = 
LDLibs  =  ${LibXMP} -Xlinker -R -Xlinker .
LDLibs  += -lc -lm -lpthread -lstdc++

ifeq "${TargetOS}" "i80386linux"
	CPPFlags += -D__LITTLE_ENDIAN__=1
	LDLibs += -lgcc_eh
endif
ifeq "${TargetOS}" "sparcsolaris"
	CPPFlags += -mtune=ultrasparc -D__BIG_ENDIAN__=1
endif

ifeq "${TargetStage}" "debug"
	CPPFlags += -g -O0 -DDEBUG=1 -D_DEBUG=1
endif
ifeq "${TargetStage}" "release"
	CPPFlags += -O2 -Os -DNDEBUG=1
endif

################################################################################################

vpath %.cpp\
    ${BuildRoot}/source: \
    ${BuildRoot}/source/common:\
    ${XMPRoot}/source/common:

Includes = \
   -I${XMPRoot}/public/include \
   -I${XMPRoot}/samples/source/common \
   -I${XMPRoot}/source/common

DumfileCommonObj = \
	${TempRoot}/QELog.o \
	${TempRoot}/QETagTree.o \
	${TempRoot}/QEMemLeak.o \
	${TempRoot}/QEDumpFile.o \
	${TempRoot}/QEBuginese.o \
	${TempRoot}/LargeFileAccess.o \
	${TempRoot}/QEScanner.o
	
################################################################################################

${TempRoot}/%.o : %.c
	@echo ""
	@echo "Compiling $<"
	${CC} ${CPPFlags} ${Includes} -c $< -o $@

${TempRoot}/%.o : %.cpp
	@echo ""
	@echo "Compiling $<"
	${CPP} ${CPPFlags} ${Includes} -c $< -o $@

ifeq "${Sample}" "dumpfile/main"
${TargetRoot}/% :${TempRoot}/%.o ${DumfileCommonObj}
	@echo ""
	@echo "Linking ${TargetRoot}/dumpfile"
	${LD} ${LDFlags} $< ${DumfileCommonObj}  ${LDLibs} -o ${TargetRoot}/dumpfile
else
${TargetRoot}/% :${TempRoot}/%.o ${TempRoot}/XMPScanner.o
	@echo "Linking $@"
	${LD} ${LDFlags} $< ${TempRoot}/XMPScanner.o ${LDLibs} -o $@
endif
	

################################################################################################

Sample : Sample_ann msg create_dirs ${TargetRoot}/${Sample}
	@echo ""

Sample_ann : 
ifeq "${Error}" ""
	@echo ""
	@echo Building XMP sample ${Sample} for ${TargetOS} ${TargetStage}
endif

msg :
ifneq "${Error}" ""
	@echo ""
	@echo "Error: ${Error}"
	@echo ""
	@echo "# To build one of the XMP samples:"
	@echo "#   make -f XMPSamples.mak [os=<os>] [stage=<stage>] name=<sample>"
	@echo "# where"
	@echo "#   os    = i80386linux | sparcsolaris"
	@echo "#   stage = debug | release"
	@echo "#"

	@echo "# The name argument is the "simple name" of the sample:"
	@echo "# XMPCoreCoverage, CustomSchema or DumpScannedXMP."
	@echo "#"
	@echo "# If the os is omitted it will try to default from the OSTYPE and"
	@echo "# MACHTYPE environment variables. If the stage is omitted it"
	@echo "# defaults to debug."
	@echo ""
	exit 1
endif

create_dirs :
	mkdir -p ${TempRoot}
	mkdir -p ${TempRoot}/dumpfile
	mkdir -p ${TargetRoot}
	
.PHONY : clean

clean : msg
	# just to be extra safe:
	ifneq "${TempRoot}" ""
	ifneq "${TargetRoot}" ""
		rm -rf ${TempRoot}/*
	endif
	endif

