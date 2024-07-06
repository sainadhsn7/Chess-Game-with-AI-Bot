QT       += core gui
QT       += svg
QT += svgwidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BoardRepresentation.cpp \
    aiplayer.cpp \
    aisettings.cpp \
    bitboard.cpp \
    board.cpp \
    boardui.cpp \
    book.cpp \
    bookcreator.cpp \
    evaluation.cpp \
    fennotation.cpp \
    gamemanager.cpp \
    humanplayer.cpp \
    main.cpp \
    mainwindow.cpp \
    movegenerator.cpp \
    moveordering.cpp \
    piece.cpp \
    piecelist.cpp \
    precomputedmovedata.cpp \
    pseudolegalmovegenerator.cpp \
    search.cpp \
    transpositiontable.cpp \
    zobrist.cpp

HEADERS += \
    BoardRepresentation.h \
    BoardTheme.h \
    Coord.h \
    PieceUI.h \
    Player.h \
    SearchDiagnostics.h \
    aiplayer.h \
    aisettings.h \
    bitboard.h \
    board.h \
    boardui.h \
    book.h \
    bookcreator.h \
    evaluation.h \
    fennotation.h \
    gamemanager.h \
    humanplayer.h \
    mainwindow.h \
    move.h \
    movegenerator.h \
    moveordering.h \
    piece.h \
    piecelist.h \
    piecesquaretable.h \
    precomputedmovedata.h \
    pseudolegalmovegenerator.h \
    search.h \
    transpositiontable.h \
    zobrist.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
