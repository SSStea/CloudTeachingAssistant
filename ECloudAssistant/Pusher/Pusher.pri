INCLUDEPATH += $$PWD \
               $$PWD/capture \
               $$PWD/rtmp

HEADERS += \
    $$PWD/capture/AudioBuffer.h \
    $$PWD/capture/AudioCapture.h \
    $$PWD/capture/GDIScreenScapture.h \
    $$PWD/capture/WASAPICapture.h \
    $$PWD/rtmp/Amf.h \
    $$PWD/rtmp/Rtmp.h \
    $$PWD/rtmp/RtmpHandshake.h \
    $$PWD/rtmp/RtmpMessage.h

SOURCES += \
    $$PWD/capture/AudioCapture.cpp \
    $$PWD/capture/GDIScreenScapture.cpp \
    $$PWD/capture/WASAPICapture.cpp \
    $$PWD/rtmp/Amf.cpp \
    $$PWD/rtmp/RtmpHandshake.cpp

INCLUDEPATH += $(FFMPEG_HOME)/include

LIBS += -L$(FFMPEG_HOME)/lib -lavcodec -lavdevice -lavformat -lavutil -lswresample -lswscale

LIBS += -lws2_32 \
        -lgdi32 \
        -luser32 \
        -lole32 \
        -lksuser
