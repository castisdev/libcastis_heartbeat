CPP = /usr/bin/g++

ifndef KERNELTYPE
KERNELTYPE_STR = $(shell getconf LONG_BIT)
KERNELTYPE = 32

ifneq ($(findstring 32, $(KERNELTYPE_STR)),)
	KERNELTYPE = 32
endif
ifneq ($(findstring 64, $(KERNELTYPE_STR)),)
	KERNELTYPE = 64
endif
endif

CFLAGS+=-D_BARBADOS_=1 -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -g -Os
CFLAGS+=-fPIC
INCLUDEDIR+=-IExternals/Include
INCLUDEDIR+=-ISrc
INCLUDEDIR+=-IInclude

CFLAGS+=$(CDEBUG) $(INCLUDEDIR)
LDFLAGS+=$(LIBDIR) -pthread

vpath %.cpp ./Src
vpath %.cpp ./UnitTest

OBJS_LIST = CiConfig.o CiQueue.o CiRealTimeThread.o CiSafeString.o CiMutex.o CiSemaphore.o CiSocket.o CiMulticastSocket.o CiThread2.o CiThreadComplex.o CiThreadRealTimeComplex.o CiUtils.o MTime2.o MTimeSpan2.o UTime.o UTimeSpan.o NetUtil.o NetworkThread.o StringLinkedList.o crc32.o RealTimeFlavor.o RealTimeNetworkThread.o AuthNetworkThread.o MessageDigest.o MessageDigestMD5.o MessageDigestSHA1.o NetIOStream.o utf8_codecvt.o CiLogger.o

OBJS_UNITTEST_LIST = RealTimeFlavorTest.o UTimeTest.o CiSocketTest.o

LINKAGE_UNITTEST = -lgtest_main -lgcov
ifeq ($(KERNELTYPE), 32)
	LIB = libCiUtils.a
	OBJ_DIR = obj32
	CFLAGS += -m32
	LDFLAGS += -m32
	PROG_UNITTEST = UnitTestCiUtils
	LINKAGE = -lboost_filesystem -luuid -lssl
else
	LIB = libCiUtils_64.a
	OBJ_DIR = obj64
	CFLAGS += -m64
	LDFLAGS += -m64
	PROG_UNITTEST = UnitTestCiUtils
	LINKAGE = -lboost_filesystem -luuid -lssl
endif

OBJS = $(addprefix $(OBJ_DIR)/,$(OBJS_LIST))
OBJS_UNITTEST = $(addprefix $(OBJ_DIR)/,$(OBJS_UNITTEST_LIST))

all: $(OBJS) 
	@echo "archiving..." $(LIB)
	@ar rcs $(LIB) $(OBJS) 

$(OBJS): |$(OBJ_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o : %.cpp
	@echo "compiling..." $@
	@$(CPP) $(CFLAGS) -c $< -o $@

test: CFLAGS+=-fprofile-arcs -ftest-coverage
test: $(OBJS) $(OBJS_UNITTEST)
	@echo "linking..." $(PROG_UNITTEST)
	@$(CPP) $(LDFLAGS) $(OBJS) $(OBJS_UNITTEST) $(LINKAGE) $(LINKAGE_UNITTEST) -o $(PROG_UNITTEST)

ctags:
	find -name '*.h' -or -name '*.cpp' -or -name '*.c' | ctags -L - -f tags

install: all
	cp $(LIB) Lib
	
clean:
	$(RM) $(OBJS) $(LIB) Src/*.gch Src/*.gcno

#pch: Src/internal_CiUtils.h
#	$(CPP) $(CFLAGS) -x c++-header Src/internal_CiUtils.h -o Src/internal_CiUtils.h.gch
