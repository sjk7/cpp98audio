TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXX_FLAGS += std=c++98

SOURCES += \
    cpp98audio_test.cpp

HEADERS += \
    ../include/cpp_98_audio_envelope.hpp
