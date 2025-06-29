QT += core gui widgets multimedia
INCLUDEPATH += C:/taggg/taglib
INCLUDEPATH += C:/taggg/taglib/toolkit
INCLUDEPATH += C:/taggg/taglib/mpeg
INCLUDEPATH += C:/taggg/taglib/mpeg/id3v2
LIBS += -LC:\taggg\taglib\build\taglib\Release -ltag
CONFIG += c++17

QMAKE_CXXFLAGS += -MD
QMAKE_LFLAGS += -MD

# برای Debug (اینها باید کامنت شده باشند اگر در Release بیلد می کنید)
# QMAKE_CXXFLAGS += -MDd
# QMAKE_LFLAGS += -MDd


SOURCES += \
    forget.cpp \
    main.cpp \
    menu.cpp \
    login.cpp \
    new_playlist.cpp \
    new_queue.cpp \
    sign_up.cpp \
    user.cpp \
    panel_user.cpp \
    info.cpp \
    home.cpp \
    style_playlistitem.cpp \
    video.cpp \
    serverdialog.cpp\
    musicnetwork.cpp


HEADERS += \
    forget.h \
    menu.h \
    login.h \
    new_playlist.h \
    new_queue.h \
    sign_up.h \
    user.h \
    panel_user.h \
    info.h \
	home.h \
    style_playlistitem.h \
    video.h\
    serverdialog.h\
    musicnetwork.h


FORMS += \
    forget.ui \
    menu.ui \
    new_playlist.ui \
    new_queue.ui \
    user.ui \
    panel_user.ui \
    home.ui \
    info.ui  \
    video.ui\
    serverdialog.ui

RESOURCES += \
        resources.qrc
