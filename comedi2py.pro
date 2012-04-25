## Modify these variables:
TEMPLATE	= app
CONFIG		+= qt warn_on debug
HEADERS		= comedi2py.h comediasync.h
SOURCES		= comedi2py.cpp comediasync.cpp
TARGET		= comedi2py
LIBS            += -lcomedi -liir -lpython2.6
target.path     = /usr/local/bin
INSTALLS        += target
INCLUDEPATH     += /usr/include/python2.6
