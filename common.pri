# allow c++11
QMAKE_CXXFLAGS +=  -std=c++0x

#-Wno-psabi is to remove next g++ warning/note:
#the mangling of 'va_list' has changed in GCC 4.4
QMAKE_CXXFLAGS += -Wno-psabi
