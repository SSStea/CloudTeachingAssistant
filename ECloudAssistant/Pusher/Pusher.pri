HEADERS += \
    $$PWD/capture/AudioBuffer.h \
    $$PWD/capture/AudioCapture.h \
    $$PWD/capture/WASAPICapture.h

SOURCES += \
    $$PWD/capture/AudioCapture.cpp \
    $$PWD/capture/WASAPICapture.cpp

LIBS += -lws2_32 \
        -lgdi32 \
        -luser32 \
        -lole32 \
        -lksuser
