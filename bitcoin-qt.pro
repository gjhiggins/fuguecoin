TEMPLATE = app
TARGET = fuguecoin-qt
macx:TARGET = "Fuguecoin-Qt"
VERSION = 0.8.6
INCLUDEPATH += src src/json src/qt
QT += core gui network
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE QT_NO_PRINTER BOOST_NO_CXX11_SCOPED_ENUMS ENABLE_PRECOMPILED_HEADERS=OFF

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += no_include_pwd
CONFIG += thread
CONFIG += debug # release
CONFIG += qt_framework
QT += core gui network testlib
CONFIG += link_pkgconfig
CONFIG += moc

QMAKE_CFLAGS_ISYSTEM=
# qmake on Qt 5.3 and lower doesn't recognize c++14.
contains(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 4) {
    CONFIG += c++11
    QMAKE_CXXFLAGS_CXX11 = $$replace(QMAKE_CXXFLAGS_CXX11, "std=c\+\+11", "std=c++1y")
    QMAKE_CXXFLAGS_CXX11 = $$replace(QMAKE_CXXFLAGS_CXX11, "std=c\+\+0x", "std=c++1y")
} else {
     CONFIG += c++14
     QT_WARNING_DISABLE_DEPRECATED=1
}

# Qt 4 doesn't even know about C++11.
contains(QT_MAJOR_VERSION, 4) {
    QMAKE_CXXFLAGS += -std=c++1y
}

static:DEFINES += STATIC_QT

isEmpty(BDB_LIB_SUFFIX) {
    # !macx:unix:BDB_LIB_SUFFIX = -5.3
    windows:macx:BDB_LIB_SUFFIX = -4.8
}

exists( /usr/local/Cellar/* ) {
      message( "Configuring for homebrew..." )
      CONFIG += brew
}

!windows:!unix {
    CONFIG += static
}

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
}

# for boost 1.37, add -mt to the boost libraries
# use: qmake BOOST_LIB_SUFFIX=-mt
# for boost thread win32 with _win32 sufix
# use: BOOST_THREAD_LIB_SUFFIX=_win32-...
# or when linking against a specific BerkelyDB version: BDB_LIB_SUFFIX=-4.8

# Dependency library locations can be customized with:
#    BOOST_INCLUDE_PATH, BOOST_LIB_PATH, BDB_INCLUDE_PATH,
#    BDB_LIB_PATH, OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively


# winbuild dependencies
windows {
    contains(MXE, 1) {
        # DEPLOYMENT_PLUGIN += qsqlite
        DEFINES += WIN32
        BDB_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include
        BDB_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
        BOOST_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include/boost
        BOOST_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
        BOOST_LIB_SUFFIX=-mt
        BOOST_THREAD_LIB_SUFFIX=_win32-mt
        CXXFLAGS=-std=c++11 -march=i686
        LDFLAGS=-march=i686
        MINIUPNPC_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include
        MINIUPNPC_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
        OPENSSL_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include/openssl
        OPENSSL_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
        PATH=/usr/lib/mxe/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
        QMAKE_LRELEASE=/usr/lib/mxe/usr/i686-w64-mingw32.static/qt5/bin/lrelease
        QTDIR=/usr/lib/mxe/usr/i686-w64-mingw32.static/qt5
    }else{
        lessThan(QT_VERSION, 5.4) {
            BOOST_LIB_SUFFIX=-mgw48-mt-s-1_55
        } else {
            BOOST_LIB_SUFFIX=-mgw49-mt-s-1_55
        }
        BOOST_INCLUDE_PATH=C:/deps/boost_1_55_0
        BOOST_LIB_PATH=C:/deps/boost_1_55_0/stage/lib
        BDB_INCLUDE_PATH=C:/deps/db-4.8.30.NC/build_unix
        BDB_LIB_PATH=C:/deps/db-4.8.30.NC/build_unix
        OPENSSL_INCLUDE_PATH=C:/deps/openssl-1.0.1i/include
        OPENSSL_LIB_PATH=C:/deps/openssl-1.0.1i
        MINIUPNPC_INCLUDE_PATH=C:/deps
        MINIUPNPC_LIB_PATH=C:/deps/miniupnpc
    }
}

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    # Mac: compile for maximum compatibility (10.5, 32-bit)
    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk
    macx:QMAKE_CFLAGS += -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk
    macx:QMAKE_OBJECTIVE_CFLAGS += -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk

    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_CFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_LFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_OBJECTIVE_CFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk

    !win32:!macx {
        # Linux: static link and extra security (see: https://wiki.debian.org/Hardening)
        LIBS += -Wl,-Bstatic -Wl,-z,relro -Wl,-z,now
    }
}

!win32 {
    # for extra security against potential buffer overflows: enable GCCs Stack Smashing Protection
    QMAKE_CXXFLAGS *= -fstack-protector-all
    QMAKE_LFLAGS *= -fstack-protector-all
    # Exclude on Windows cross compile with MinGW 4.2.x, as it will result in a non-working executable!
    # This can be enabled for Windows, when we switch to MinGW >= 4.4.x.
}
# for extra security (see: https://wiki.debian.org/Hardening): this flag is GCC compiler-specific
QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2
# for extra security on Windows: enable ASLR and DEP via GCC linker flags
win32:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat
# on Windows: enable GCC large address aware linker flag
win32:QMAKE_LFLAGS *= -Wl,--large-address-aware

# use: qmake "USE_UPNP=1" ( enabled by default; default)
#  or: qmake "USE_UPNP=0" (disabled by default)
#  or: qmake "USE_UPNP=-" (not supported)
# miniupnpc (http://miniupnp.free.fr/files/) must be installed for support
contains(USE_UPNP, -) {
    message(Building without UPNP support)
} else {
    message(Building with UPNP support)
    count(USE_UPNP, 0) {
        USE_UPNP=1
    }
    DEFINES += USE_UPNP=$$USE_UPNP STATICLIB
    INCLUDEPATH += $$MINIUPNPC_INCLUDE_PATH
    LIBS += $$join(MINIUPNPC_LIB_PATH,,-L,) -lminiupnpc
    win32:LIBS += -liphlpapi
}

# use: qmake "USE_DBUS=1"
contains(USE_DBUS, 1) {
    message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
    QT += dbus
}

# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
    macx:win32:LIBS += -lqrencode
}

# use: qmake "USE_IPV6=1" ( enabled by default; default)
#  or: qmake "USE_IPV6=0" (disabled by default)
#  or: qmake "USE_IPV6=-" (not supported)
contains(USE_IPV6, -) {
    message(Building without IPv6 support)
} else {
    count(USE_IPV6, 0) {
        USE_IPV6=1
    }
    DEFINES += USE_IPV6=$$USE_IPV6
}

contains(BITCOIN_NEED_QT_PLUGINS, 1) {
    DEFINES += BITCOIN_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}

INCLUDEPATH += src/leveldb/include src/leveldb/helpers
LIBS += $$PWD/src/leveldb/libleveldb.a $$PWD/src/leveldb/libmemenv.a
!win32 {
    # we use QMAKE_CXXFLAGS_RELEASE even without RELEASE=1 because we use RELEASE to indicate linking preferences not -O preferences
    genleveldb.commands = cd $$PWD/src/leveldb && CC=$$QMAKE_CC CXX=$$QMAKE_CXX $(MAKE) OPT=\"$$QMAKE_CXXFLAGS $$QMAKE_CXXFLAGS_RELEASE\" libleveldb.a libmemenv.a
} else {
    # make an educated guess about what the ranlib command is called
    isEmpty(QMAKE_RANLIB) {
        QMAKE_RANLIB = $$replace(QMAKE_STRIP, strip, ranlib)
    }
    LIBS += -lshlwapi
    genleveldb.commands = cd $$PWD/src/leveldb && CC=$$QMAKE_CC CXX=$$QMAKE_CXX TARGET_OS=OS_WINDOWS_CROSSCOMPILE $(MAKE) OPT=\"$$QMAKE_CXXFLAGS $$QMAKE_CXXFLAGS_RELEASE\" libleveldb.a libmemenv.a && $$QMAKE_RANLIB $$PWD/src/leveldb/libleveldb.a && $$QMAKE_RANLIB $$PWD/src/leveldb/libmemenv.a
}
genleveldb.target = $$PWD/src/leveldb/libleveldb.a
genleveldb.depends = FORCE
PRE_TARGETDEPS += $$PWD/src/leveldb/libleveldb.a
QMAKE_EXTRA_TARGETS += genleveldb
# Gross ugly hack that depends on qmake internals, unfortunately there is no other way to do it.
QMAKE_CLEAN += $$PWD/src/leveldb/libleveldb.a; cd $$PWD/src/leveldb ; $(MAKE) clean

# regenerate src/build.h
!win32|contains(USE_BUILD_INFO, 1) {
    genbuild.depends = FORCE
    genbuild.commands = cd $$PWD; /bin/sh share/genbuild.sh $$OUT_PWD/build/build.h
    genbuild.target = $$OUT_PWD/build/build.h
    PRE_TARGETDEPS += $$OUT_PWD/build/build.h
    QMAKE_EXTRA_TARGETS += genbuild
    DEFINES += HAVE_BUILD_INFO
}

QMAKE_CXXFLAGS_WARN_ON = -fdiagnostics-show-option -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter -Wstack-protector

# Input
DEPENDPATH += src src/json src/qt
HEADERS += src/qt/bitcoingui.h \
    src/qt/transactiontablemodel.h \
    src/qt/addresstablemodel.h \
    src/qt/optionsdialog.h \
    src/qt/sendcoinsdialog.h \
    src/qt/coincontroldialog.h \
    src/qt/coincontroltreewidget.h \
    src/qt/addressbookpage.h \
    src/qt/signverifymessagedialog.h \
    src/qt/aboutdialog.h \
    src/qt/editaddressdialog.h \
    src/qt/bitcoinaddressvalidator.h \
    src/qt/messagepage.h \
    src/alert.h \
    src/addrman.h \
    src/base58.h \
    src/bignum.h \
    src/checkpoints.h \
    src/coincontrol.h \
    src/compat.h \
    src/sync.h \
    src/util.h \
    src/hash.h \
    src/uint256.h \
    src/serialize.h \
    src/main.h \
    src/net.h \
    src/key.h \
    src/db.h \
    src/walletdb.h \
    src/script.h \
    src/init.h \
    src/bloom.h \
    src/mruset.h \
    src/checkqueue.h \
    src/json/json_spirit_writer_template.h \
    src/json/json_spirit_writer.h \
    src/json/json_spirit_value.h \
    src/json/json_spirit_utils.h \
    src/json/json_spirit_stream_reader.h \
    src/json/json_spirit_reader_template.h \
    src/json/json_spirit_reader.h \
    src/json/json_spirit_error_position.h \
    src/json/json_spirit.h \
    src/qt/clientmodel.h \
    src/qt/guiutil.h \
    src/qt/transactionrecord.h \
    src/qt/guiconstants.h \
    src/qt/optionsmodel.h \
    src/qt/monitoreddatamapper.h \
    src/qt/transactiondesc.h \
    src/qt/transactiondescdialog.h \
    src/qt/bitcoinamountfield.h \
    src/wallet.h \
    src/keystore.h \
    src/qt/transactionfilterproxy.h \
    src/qt/transactionview.h \
    src/qt/walletmodel.h \
    src/qt/walletview.h \
    src/qt/walletstack.h \
    src/qt/walletframe.h \
    src/bitcoinrpc.h \
    src/qt/overviewpage.h \
    src/qt/csvmodelwriter.h \
    src/crypter.h \
    src/qt/sendcoinsentry.h \
    src/qt/qvalidatedlineedit.h \
    src/qt/bitcoinunits.h \
    src/qt/qvaluecombobox.h \
    src/qt/askpassphrasedialog.h \
    src/protocol.h \
    src/qt/notificator.h \
    src/qt/paymentserver.h \
    src/allocators.h \
    src/ui_interface.h \
    src/qt/rpcconsole.h \
    src/version.h \
    src/netbase.h \
    src/script_error.h \
    src/clientversion.h \
    src/txdb.h \
    src/leveldb.h \
    src/threadsafety.h \
    src/limitedmap.h \
    src/qt/macnotificationhandler.h \
    src/qt/splashscreen.h \
    src/qt/intro.h \
    src/qt/qcustomplot.h \
    src/qt/blockexplorer.h \
    src/qt/miningpage.h \
    src/sph_fugue.h \
    src/sph_types.h \
    src/ecies/ecies.h

SOURCES += src/qt/bitcoin.cpp \
    src/qt/bitcoingui.cpp \
    src/qt/transactiontablemodel.cpp \
    src/qt/addresstablemodel.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/sendcoinsdialog.cpp \
    src/qt/coincontroldialog.cpp \
    src/qt/coincontroltreewidget.cpp \
    src/qt/addressbookpage.cpp \
    src/qt/messagepage.cpp \
    src/qt/signverifymessagedialog.cpp \
    src/qt/aboutdialog.cpp \
    src/qt/editaddressdialog.cpp \
    src/qt/bitcoinaddressvalidator.cpp \
    src/alert.cpp \
    src/version.cpp \
    src/sync.cpp \
    src/util.cpp \
    src/hash.cpp \
    src/netbase.cpp \
    src/key.cpp \
    src/script.cpp \
    src/main.cpp \
    src/init.cpp \
    src/net.cpp \
    src/bloom.cpp \
    src/checkpoints.cpp \
    src/addrman.cpp \
    src/db.cpp \
    src/walletdb.cpp \
    src/qt/clientmodel.cpp \
    src/qt/guiutil.cpp \
    src/qt/transactionrecord.cpp \
    src/qt/optionsmodel.cpp \
    src/qt/monitoreddatamapper.cpp \
    src/qt/transactiondesc.cpp \
    src/qt/transactiondescdialog.cpp \
    src/qt/bitcoinstrings.cpp \
    src/qt/bitcoinamountfield.cpp \
    src/wallet.cpp \
    src/keystore.cpp \
    src/qt/transactionfilterproxy.cpp \
    src/qt/transactionview.cpp \
    src/qt/walletmodel.cpp \
    src/qt/walletview.cpp \
    src/qt/walletstack.cpp \
    src/qt/walletframe.cpp \
    src/bitcoinrpc.cpp \
    src/rpcdump.cpp \
    src/rpcnet.cpp \
    src/rpcmining.cpp \
    src/rpcwallet.cpp \
    src/rpcblockchain.cpp \
    src/rpcrawtransaction.cpp \
    src/qt/overviewpage.cpp \
    src/qt/csvmodelwriter.cpp \
    src/crypter.cpp \
    src/qt/sendcoinsentry.cpp \
    src/qt/qvalidatedlineedit.cpp \
    src/qt/bitcoinunits.cpp \
    src/qt/qvaluecombobox.cpp \
    src/qt/askpassphrasedialog.cpp \
    src/protocol.cpp \
    src/qt/notificator.cpp \
    src/qt/paymentserver.cpp \
    src/qt/rpcconsole.cpp \
    src/noui.cpp \
    src/leveldb.cpp \
    src/txdb.cpp \
    src/qt/splashscreen.cpp \
    src/qt/intro.cpp \
    src/qt/qcustomplot.cpp \
    src/qt/blockexplorer.cpp \
    src/qt/miningpage.cpp \
    src/script_error.cpp \
    src/fugue.c \
    src/ecies/ecies.c \
    src/ecies/kdf.c \
    src/ecies/secure.c

RESOURCES += src/qt/bitcoin.qrc

FORMS += src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/coincontroldialog.ui \
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/signverifymessagedialog.ui \
    src/qt/forms/aboutdialog.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/transactiondescdialog.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/rpcconsole.ui \
    src/qt/forms/optionsdialog.ui \
    src/qt/forms/intro.ui \
    src/qt/forms/blockexplorer.ui \
    src/qt/forms/miningpage.ui \
    src/qt/forms/messagepage.ui

contains(USE_QRCODE, 1) {
HEADERS += src/qt/qrcodedialog.h
SOURCES += src/qt/qrcodedialog.cpp
FORMS += src/qt/forms/qrcodedialog.ui
}

contains(BITCOIN_QT_TEST, 1) {
SOURCES -= src/qt/bitcoin.cpp
SOURCES += src/qt/test/test_main.cpp \
           src/qt/test/uritests.cpp \
           src/qt/qrcodedialog.cpp
HEADERS += src/qt/test/uritests.h \
           src/qt/qrcodedialog.h
DEPENDPATH += src/qt/test
QT += testlib
DEFINES += USE_QRCODE
LIBS += -lqrencode
TARGET = fuguecoin-qt_test
DEFINES += BITCOIN_QT_TEST
  macx: CONFIG -= app_bundle
}

CODECFORTR = UTF-8

# for lrelease/lupdate
# also add new translations to src/qt/bitcoin.qrc under translations/
TRANSLATIONS = $$files(src/qt/locale/bitcoin_*.ts)

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(QM_DIR):QM_DIR = $$PWD/src/qt/locale
# automatically build translations, so they can be included in resource file
TSQM.name = lrelease ${QMAKE_FILE_IN}
TSQM.input = TRANSLATIONS
TSQM.output = $$QM_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM

# "Other files" to show in Qt Creator
OTHER_FILES += README.md \
    doc/*.rst \
    doc/*.txt \
    doc/*.md \
    src/qt/res/bitcoin-qt.rc \
    src/test/*.cpp \
    src/test/*.h \
    src/qt/test/*.cpp \
    src/qt/test/*.h

# platform specific defaults, if not overridden on command line
isEmpty(BOOST_LIB_SUFFIX) {
    macx:BOOST_LIB_SUFFIX = -mt
    win32:BOOST_LIB_SUFFIX = -mgw44-mt-s-1_50
}

isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

isEmpty(BDB_LIB_PATH) {
    macx:BDB_LIB_PATH = /opt/local/lib/db48
}

isEmpty(BDB_LIB_SUFFIX) {
    macx:BDB_LIB_SUFFIX = -4.8
}

isEmpty(BDB_INCLUDE_PATH) {
    contains(CONFIG, brew) {
        contains(BDB_LIB_SUFFIX, -4.8) {
            macx:BDB_INCLUDE_PATH = /usr/local/opt/berkeley-db4/include
        }else{
            macx:BDB_INCLUDE_PATH = /usr/local/opt/berkeley-db/include
        }
    }else{
        contains(BDB_LIB_SUFFIX, -4.8) {
            macx:BDB_INCLUDE_PATH = /opt/local/berkeley-db4/include
        }else{
            macx:BDB_INCLUDE_PATH = /opt/local/berkeley-db/include
        }
    }
    windows:BDB_INCLUDE_PATH = C:/dev/coindeps32/bdb-4.8/include
    # For backward compatibility specify, else assume currency
    contains(BDB_LIB_SUFFIX, 4.8) {
        !macx:unix:BDB_INCLUDE_PATH = /usr/local/BerkeleyDB/include
    } # else{
      #   !macx:unix:BDB_INCLUDE_PATH = /usr/include
    # }
    INCLUDEPATH += $$BDB_INCLUDE_PATH
}

isEmpty(BDB_LIB_PATH) {
    contains(CONFIG, brew) {
        contains(BDB_LIB_SUFFIX, -4.8) {
            macx:BDB_LIB_PATH = /usr/local/opt/berkeley-db4/lib
        }else{
            macx:BDB_LIB_PATH = /usr/local/opt/berkeley-db/lib
        }
    }else{
        contains(BDB_LIB_SUFFIX, -4.8) {
            macx:BDB_LIB_PATH = /opt/local/berkeley-db4/lib
        }else{
            macx:BDB_LIB_PATH = /opt/local/berkeleydb/lib
        }
    }
    windows:BDB_LIB_PATH = C:/dev/coindeps32/bdb-4.8/lib
    # For backward compatibility specify, else assume currency
    contains(BDB_LIB_SUFFIX, -4.8) {
        !macx:unix:BDB_LIB_PATH = /usr/local/BerkeleyDB/lib
    } # else{
      #   !macx:unix:BDB_LIB_PATH = /usr/lib/x86_64-linux-gnu/
    # }
    LIBS += $$join(BDB_LIB_PATH,,-L,)
}

isEmpty(BOOST_INCLUDE_PATH) {
    contains(CONFIG, brew) {
        macx:BOOST_INCLUDE_PATH = /usr/local/opt/boost/include
    }else{
        macx:BOOST_INCLUDE_PATH = /opt/local/include
    }
    windows:BOOST_INCLUDE_PATH = C:/dev/coindeps32/boost_1_57_0/include
    !macx:unix:BOOST_INCLUDE_PATH = /usr/include/boost
    INCLUDEPATH += $$BOOST_INCLUDE_PATH
}

isEmpty(BOOST_LIB_PATH) {
    contains(CONFIG, brew) {
        macx:BOOST_LIB_PATH = /usr/local/opt/boost/lib
    }else{
        macx:BOOST_LIB_PATH = /opt/local/lib
    }
    windows:BOOST_LIB_PATH = C:/dev/coindeps32/boost_1_57_0/lib
    # !macx:unix:BOOST_LIB_PATH = /usr/lib
    LIBS += $$join(BOOST_LIB_PATH,,-L,)
}

isEmpty(OPENSSL_INCLUDE_PATH) {
    contains(CONFIG, brew) {
        macx:OPENSSL_INCLUDE_PATH = /usr/local/opt/openssl/include
    }else{
        macx:OPENSSL_INCLUDE_PATH = /opt/local/include
    }
    windows:OPENSSL_INCLUDE_PATH = C:/dev/coindeps32/openssl-1.0.1p/include
    !macx:unix:OPENSSL_INCLUDE_PATH = /usr/include/openssl
    INCLUDEPATH += $$OPENSSL_INCLUDE_PATH
}

isEmpty(OPENSSL_LIB_PATH) {
    contains(CONFIG, brew) {
        macx:OPENSSL_LIB_PATH = /usr/local/opt/openssl/lib
    }else{
        macx:OPENSSL_LIB_PATH = /opt/local/lib
    }
    windows:OPENSSL_LIB_PATH = C:/dev/coindeps32/openssl-1.0.1p/lib
    # !macx:unix:OPENSSL_LIB_PATH = /usr/lib
    LIBS += $$join(OPENSSL_LIB_PATH,,-L,)}

# Force OS X Sierra specifics
macx {
    CONFIG += 11 x86_64
    HEADERS += src/qt/macdockiconhandler.h src/qt/macnotificationhandler.h
    INCLUDEPATH += $$MOC_DIR # enable #include of moc_* files
    OBJECTIVE_SOURCES += src/qt/macdockiconhandler.mm src/qt/macnotificationhandler.mm
    LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
    LIBS += /usr/local/opt/miniupnpc/lib/libminiupnpc.a
    LIBS += /usr/local/opt/berkeley-db/lib/libdb_cxx.a
    LIBS += /usr/local/opt/openssl/lib/libcrypto.a
    LIBS += /usr/local/opt/openssl/lib/libssl.a
    LIBS += /usr/local/opt/boost/lib/libboost_system-mt.a
    LIBS += /usr/local/opt/boost/lib/libboost_filesystem-mt.a
    LIBS += /usr/local/opt/boost/lib/libboost_program_options-mt.a
    LIBS += /usr/local/opt/boost/lib/libboost_thread-mt.a
    DEFINES += MAC_OSX MSG_NOSIGNAL=0
    ICON = src/qt/res/icons/slimcoin.icns
    TARGET = "SLIMCoin-Qt"
    QMAKE_CFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.12
    QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.12
    QMAKE_MAC_SDK = macosx10.12
    CXXFLAGS += -std=c++11 -march=i686
    QMAKE_INFO_PLIST = share/qt/Info.plist
    CONFIG -= brew
}

windows:!contains(MINGW_THREAD_BUGFIX, 0) {
    # At least qmake's win32-g++-cross profile is missing the -lmingwthrd
    # thread-safety flag. GCC has -mthreads to enable this, but it doesn't
    # work with static linking. -lmingwthrd must come BEFORE -lmingw, so
    # it is prepended to QMAKE_LIBS_QT_ENTRY.
    # It can be turned off with MINGW_THREAD_BUGFIX=0, just in case it causes
    # any problems on some untested qmake profile now or in the future.
    DEFINES += _MT
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

!win32:!macx {
    DEFINES += LINUX
    LIBS += -lrt
    # _FILE_OFFSET_BITS=64 lets 32-bit fopen transparently support large files.
    DEFINES += _FILE_OFFSET_BITS=64
}

macx:HEADERS += src/qt/macdockiconhandler.h src/qt/macnotificationhandler.h
macx:OBJECTIVE_SOURCES += src/qt/macdockiconhandler.mm src/qt/macnotificationhandler.mm
macx:LIBS += -framework Foundation -framework ApplicationServices -framework AppKit -framework CoreServices
macx:DEFINES += MAC_OSX MSG_NOSIGNAL=0
macx:ICON = src/qt/res/icons/bitcoin.icns
macx:QMAKE_CFLAGS_THREAD += -pthread
macx:QMAKE_LFLAGS_THREAD += -pthread
macx:QMAKE_CXXFLAGS_THREAD += -pthread
macx:QMAKE_INFO_PLIST = share/qt/Info.plist

# Set libraries and includes at end, to use platform-defined defaults if not overridden
INCLUDEPATH += $$BOOST_INCLUDE_PATH $$BDB_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH $$QRENCODE_INCLUDE_PATH
LIBS += $$join(BOOST_LIB_PATH,,-L,) $$join(BDB_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,) $$join(QRENCODE_LIB_PATH,,-L,)
LIBS += -lssl -lcrypto -ldb_cxx$$BDB_LIB_SUFFIX
# -lgdi32 has to happen after -lcrypto (see  #681)
win32:LIBS += -lws2_32 -lshlwapi -lmswsock -lole32 -loleaut32 -luuid -lgdi32
LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX
win32:LIBS += -lboost_chrono$$BOOST_LIB_SUFFIX
macx:LIBS += -lboost_chrono$$BOOST_LIB_SUFFIX

contains(RELEASE, 1) {
    !win32:!macx {
        # Linux: turn dynamic linking back on for c/c++ runtime libraries
        LIBS += -Wl,-Bdynamic -ldl
    }
}
macx:{
    QMAKE_RPATHDIR += @executable_path/../Frameworks
    QMAKE_RPATHDIR += @executable_path/lib
}

contains(USE_QRCODE, 1) {
    !macx:!win32:LIBS += -lqrencode
}

system($$QMAKE_LRELEASE -silent $$TRANSLATIONS)
