INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/BufferReader.h \
    $$PWD/BufferWriter.h \
    $$PWD/Channel.h \
    $$PWD/EventLoop.h \
    $$PWD/ReactorBase.h \
    $$PWD/ReactorSelect.h \
    $$PWD/TcpConnection.h \
    $$PWD/TcpSocket.h \
    $$PWD/TimeStamp.h \
    $$PWD/Timer.h

SOURCES += \
    $$PWD/BufferReader.cpp \
    $$PWD/BufferWriter.cpp \
    $$PWD/EventLoop.cpp \
    $$PWD/ReactorBase.cpp \
    $$PWD/ReactorSelect.cpp \
    $$PWD/TcpConnection.cpp \
    $$PWD/TcpSocket.cpp \
    $$PWD/Timer.cpp
