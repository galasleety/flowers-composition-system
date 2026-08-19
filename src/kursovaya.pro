QT += core gui widgets sql multimedia

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        accessoriespage.cpp \
        addvariationdialog.cpp \
        adminwindow.cpp \
        appwindow.cpp \
        bouquetcard.cpp \
        bouquetdetailsdialog.cpp \
        bouquetspanel.cpp \
        catalogbottomgrid.cpp \
        catalogminicard.cpp \
        catalogrightcolumn.cpp \
        catalogtopbar.cpp \
        catalogview.cpp \
        constructorfilterdialog.cpp \
        constructorview.cpp \
        detailscolumn.cpp \
        errordialog.cpp \
        favoritespanel.cpp \
        flowerspage.cpp \
        greetingwindow.cpp \
        helpdialog.cpp \
        loginwindow.cpp \
        main.cpp \
        mainwindow.cpp \
        notecard.cpp \
        notespanel.cpp \
        packagingpage.cpp \
        palettepage.cpp \
        profileview.cpp \
        registerwindow.cpp \
        rulespage.cpp \
        savebouquetdialog.cpp \
        seasonalitypage.cpp \
        successdialog.cpp \
        userspage.cpp \
        vasecolumn.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    accessoriespage.h \
    addvariationdialog.h \
    adminwindow.h \
    appwindow.h \
    bouquetcard.h \
    bouquetdetailsdialog.h \
    bouquetspanel.h \
    catalogbottomgrid.h \
    catalogminicard.h \
    catalogrightcolumn.h \
    catalogtopbar.h \
    catalogview.h \
    constructorfilterdialog.h \
    constructorview.h \
    detailscolumn.h \
    errordialog.h \
    favoritespanel.h \
    flowerspage.h \
    greetingwindow.h \
    helpdialog.h \
    loginwindow.h \
    mainwindow.h \
    notecard.h \
    notespanel.h \
    packagingpage.h \
    palettepage.h \
    profileview.h \
    registerwindow.h \
    rulespage.h \
    savebouquetdialog.h \
    seasonalitypage.h \
    successdialog.h \
    userspage.h \
    vasecolumn.h

RESOURCES += \
    resources.qrc

DISTFILES += \
    sources/music/sound.mp3

