CPP = /usr/bin/g++

KERNELTYPE_STR = $(shell getconf LONG_BIT)
KERNELTYPE = 32

ifneq ($(findstring 32, $(KERNELTYPE_STR)),)
	KERNELTYPE = 32
endif
ifneq ($(findstring 64, $(KERNELTYPE_STR)),)
	KERNELTYPE = 64
endif

CFLAGS+=-D_BARBADOS_=1 -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -g -Os
CFLAGS+=-fPIC
INCLUDEDIR+=-I../Externals/Include
LIBDIR+=

CFLAGS+=$(CDEBUG) $(INCLUDEDIR)

OBJS = CiConfig.o CiQueue.o CiRealTimeThread.o CiSafeString.o CiMutex.o CiSemaphore.o CiSocket.o CiMulticastSocket.o CiThread2.o CiThreadComplex.o CiThreadRealTimeComplex.o CiUtils.o MTime2.o MTimeSpan2.o UTime.o UTimeSpan.o NetUtil.o NetworkThread.o StringLinkedList.o crc32.o RealTimeFlavor.o RealTimeNetworkThread.o AuthNetworkThread.o MessageDigest.o MessageDigestMD5.o MessageDigestSHA1.o NetIOStream.o utf8_codecvt.o CiLogger.o

ifeq ($(KERNELTYPE), 32)
	LIB = libCiUtils.a
else
	LIB = libCiUtils_64.a
endif

all: $(OBJS) 
	ar rcs $(LIB) $(OBJS) 

%.o : ../Src/%.cpp ../Src/%.h
	$(CPP) $(CFLAGS) -c $<

ctags:
	find ../Src -name '*.h' -or -name '*.cpp' -or -name '*.c' | ctags -L - -f tags

install: all
	cp $(LIB) ../Lib
	
clean:
	$(RM) $(OBJS) *~ $(LIB) ../Src/*.gch tags

#pch: ../Src/internal_CiUtils.h
#	$(CPP) $(CFLAGS) -x c++-header ../Src/internal_CiUtils.h -o ../Src/internal_CiUtils.h.gch
