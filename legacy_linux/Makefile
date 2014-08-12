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
CFLAGS+=-D_TIME_CHECK

INCLUDEDIR+=-I../Externals/CiUtils/Include
INCLUDEDIR+=-I../Externals/CiUtils/Externals/Include
LIBDIR+=

CFLAGS+=$(CDEBUG) $(INCLUDEDIR)
LDFLAGS+=$(LDEBUG) $(LIBDIR)

OBJS = CiHBRequester.o CiHBResponser.o CiHBAddress.o CiHBAddressList.o

ifeq ($(KERNELTYPE), 32)
	LIB = libCiHB.a
else
	LIB = libCiHB_64.a
endif

all: $(OBJS) 
	ar rcs $(LIB) $(OBJS) 

%.o : ./%.cpp ./%.h
	$(CPP) $(CFLAGS) -c $<

ctags:
	find . -name '*.h' -or -name '*.cpp' -or -name '*.c' | ctags -L - -f tags

install: all
	cp $(LIB) ../Lib
	
clean:
	$(RM) $(OBJS) *~ $(LIB) tags
