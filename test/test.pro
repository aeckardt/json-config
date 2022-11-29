QT += core

CONFIG += \
    c++17 \
    console

ROOT_DIR = $$PWD/..
SRC_DIR = $$ROOT_DIR/src
GTEST_SRCDIR = $$ROOT_DIR/googletest/googletest
GMOCK_SRCDIR = $$ROOT_DIR/googletest/googlemock

SOURCES += 
    main.cpp \
    $$SRC_DIR/configitem.cpp \
    $$SRC_DIR/jsontreeitem.cpp \
    $$GTEST_SRCDIR/src/gtest-all.cc \
    $$GMOCK_SRCDIR/src/gmock-all.cc

HEADERS += \
    test_configitem.h \
    $$SRC_DIR/configitem.h \
    $$SRC_DIR/jsontreeitem.h

INCLUDEPATH += \
    $$SRC_DIR \
    $$GTEST_SRCDIR \
    $$GTEST_SRCDIR/include \
    $$GMOCK_SRCDIR \
    $$GMOCK_SRCDIR/include