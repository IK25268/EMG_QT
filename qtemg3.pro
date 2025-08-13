QT       += core gui serialport opengl widgets openglwidgets charts multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

INCLUDEPATH += D:/Programm/qwt/library/include
INCLUDEPATH += D:/Programm/fftw

LIBS += "D:/Programm/fftw/libfftw3f-3.dll"
LIBS += "D:/Programm/fftw/libfftw3-3.dll"
LIBS += "D:/Programm/fftw/libfftw3l-3.dll"

win32 {
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y D:/Programm/fftw/libfftw3f-3.dll $$OUT_PWD)
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y D:/Programm/fftw/libfftw3-3.dll $$OUT_PWD)
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y D:/Programm/fftw/libfftw3l-3.dll $$OUT_PWD)
}

CONFIG += c++17 qwt

include( D:/Programm/qwt/library/features/qwt.prf )
include( D:/Programm/qwt/library/features/qwtconfig.pri )
include( D:/Programm/qwt/library/features/qwtfunctions.pri )

SOURCES += \
    fft_thread1.cpp \
    main.cpp \
    mainwindow.cpp \
    rwfile.cpp \
    serialread.cpp \
    wavelet.cpp

HEADERS += \
    fft_thread1.h \
    mainwindow.h \
    rwfile.h \
    serialread.h \
    wavelet.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
