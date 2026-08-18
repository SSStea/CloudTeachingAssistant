HEADERS += \
    $$PWD/capture/AudioBuffer.h \
    $$PWD/capture/AudioCapture.h \
    $$PWD/capture/GDIScreenScapture.h \
    $$PWD/capture/WASAPICapture.h

SOURCES += \
    $$PWD/capture/AudioCapture.cpp \
    $$PWD/capture/GDIScreenScapture.cpp \
    $$PWD/capture/WASAPICapture.cpp

INCLUDEPATH += $(FFMPEG_HOME)/include

LIBS += -L$(FFMPEG_HOME)/lib -lavcodec -lavdevice -lavformat -lavutil -lswresample -lswscale

LIBS += -lws2_32 \
        -lgdi32 \
        -luser32 \
        -lole32 \
        -lksuser
