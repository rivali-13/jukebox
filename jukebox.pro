QT += core gui widgets multimedia

CONFIG += c++17

SOURCES += \
    forget.cpp \
    main.cpp \
    menu.cpp \
    login.cpp \
    new_playlist.cpp \
    sign_in.cpp \
    user.cpp \
    panel_user.cpp \
    info.cpp \
    home.cpp \
    style_playlistitem.cpp

HEADERS += \
    forget.h \
    menu.h \
    login.h \
    new_playlist.h \
    sign_in.h \
    user.h \
    panel_user.h \
    info.h \
    home.h \
    style_playlistitem.h

FORMS += \
    forget.ui \
    menu.ui \
    login.ui \
    new_playlist.ui \
    sign_in.ui \
    user.ui \
    panel_user.ui \
    info.ui \
    home.ui

RESOURCES += \
        resources.qrc
