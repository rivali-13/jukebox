QT += core gui widgets multimedia

CONFIG += c++17

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
    video.cpp

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
    video.h

FORMS += \
    forget.ui \
    menu.ui \
    new_playlist.ui \
    new_queue.ui \
    user.ui \
    panel_user.ui \
    home.ui \
    info.ui  \
    video.ui
RESOURCES += \
        resources.qrc
