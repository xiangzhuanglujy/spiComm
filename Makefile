CXX=arm-linux-gnueabihf-g++
CC=arm-linux-gnueabihf-gcc

INCLUDEDIR = ./ 
INCLUDEDIR += $(HOME)/rpi/mntrpi/usr/include/qt4/
INCLUDEDIR += $(HOME)/rpi/mntrpi/usr/include/qt4/QtCore
INCLUDEDIR += $(HOME)/rpi/mntrpi/usr/include/qt4/QtGui

LIBRARYDIR = $(HOME)/rpi/mntrpi/usr/lib/arm-linux-gnueabihf/
LIBRARYDIR += $(HOME)/rpi/mntrpi/lib/arm-linux-gnueabihf/
LIBRARY +=  QtCore QtGui 
XLINK_LIBDIR += $(HOME)/rpi/mntrpi/lib/arm-linux-gnueabihf
XLINK_LIBDIR += $(HOME)/rpi/mntrpi/usr/lib/arm-linux-gnueabihf

INCDIR  = $(patsubst %,-I%,$(INCLUDEDIR))
LIBDIR  = $(patsubst %,-L%,$(LIBRARYDIR))
LIB    = $(patsubst %,-l%,$(LIBRARY))
XLINKDIR = $(patsubst %,-Xlinker -rpath-link=%,$(XLINK_LIBDIR))
 
OPT = -O3
DEBUG = -g
WARN= -Wall
PTHREAD= -pthread 
GDemo=-lGDemodual

CXXFLAGS= $(OPT) $(DEBUG) $(WARN) $(INCDIR)
LDFLAGS= $(LIBDIR) $(LIB) $(XLINKDIR) -L/usr/lib $(GDemo) $(PTHREAD)
#LDFLAGS= $(LIBDIR) $(LIB) $(XLINKDIR) $(PTHREAD)



INC = mainwindow.h qextserialbase.h posix_qextserialport.h dsp.h qcustomplot.h bcm2835.h thread.h define.h
SRC = main.cpp mainwindow.cpp qextserialbase.cpp posix_qextserialport.cpp dsp.cpp qcustomplot.cpp bcm2835.c thread.cpp

OBJ = $(SRC:.cpp=.cpp.o) $(C:.c=.c.o) $(INC:.h=.moc.o)

all: $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o spiComm

%.moc.cpp: $(INC)
	moc-qt4 $*.h -o $@

%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.c.o: %.c
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -f *.o helloCom spiComm
