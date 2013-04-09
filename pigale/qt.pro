TEMPLATE = app

win32 {
      include(../wpigale.inc)      
      DEFINES +=  __MINGW32__ _WIN32
      win32-msvc2010:DEFINES += _CRT_SECURE_NO_WARNINGS
      } else {
      include(../pigale.inc)
      }
     

MOC_DIR = moc
INCLUDEPATH += ../incl
DEPENDPATH =  ../incl $$QTINCLUDE

#files that need moc
HEADERS = pigaleWindow.h \
    ClientSocket.h \
    glcontrolwidget.h \
    gprop.h \
    GraphGL.h \
    GraphSym.h \
    GraphWidget.h \
    mouse_actions.h 

SOURCES = main.cpp \
    CanvasGrid.cpp \
    CanvasItem.cpp \
    CanvasSpring.cpp \
    ClientSocket.cpp \
    EmbedCurves.cpp \
    glcontrolwidget.cpp \
    gprop.cpp \
    GraphGL.cpp \
    graphml.cpp \
    graphmlparser.cpp \
    GraphSym.cpp \
    Handler.cpp \
    Macro.cpp \
    MiscQt.cpp \
    mouse_actions.cpp \
    pigaleCanvas.cpp \
    pigaleFile.cpp \
    pigaleMenus.cpp \
    pigalePaint.cpp \
    pigaleUndo.cpp \
    pigaleWindow.cpp \
    Settings.cpp \
    Test.cpp


CONFIG += qt thread link_prl depend_prl $$MODE

macx:CONFIG += x86 x86_64

CONFIG(debug, debug|release) {
       LIBTGRAPH=libtgraph_debug.a
       LIBGLUT=libfreeglut_debug.a
       win32-msvc2010:LIBTGRAPH=tgraph_debug.lib 
       win32-msvc2010:LIBGLUT=freeglut_debug.lib 
       }  else  {
       LIBTGRAPH=libtgraph.a
       LIBGLUT=-lglut
       win32-msvc2010:LIBTGRAPH=tgraph.lib 
       win32-msvc2010:LIBGLUT=freeglut.lib 
       }
        

CONFIG(debug, debug|release) {
    TARGET = pigale_debug
    DEFINES += TDEBUG
    OBJECTS_DIR = ./debug
    } else {
    TARGET = pigale
    OBJECTS_DIR = ./release
    }

macx {   
     LIBS += -framework GLUT
     } else {
     DEFINES += FREEGLUT FREEGLUT_STATIC
     #unix:LIBS += $$GLUTLIB
     unix:LIBS += $$LIBGLUT
     win32:LIBS +=../lib/$$LIBGLUT -lopengl32 -lglu32 -lgdi32 -luser32 -lwinmm
     }
     
QT += opengl network xml svg
DESTDIR=$$PWD
LIBS += ../lib/$$LIBTGRAPH    
win32:RC_FILE=pigale.rc

unix {	
      # awk
      awk.target = ../incl/QT/Action.h
      awk.depends = ../incl/QT/Action_def.h Action.awk
      awk.commands = $$AWK -f Action.awk ../incl/QT/Action_def.h > ../incl/QT/Action.h
      QMAKE_EXTRA_TARGETS += awk
      # Translations
      TRANSLATIONS    = pigale_fr.ts 
      translation.target = $$TRANSLATIONS
      translation.depends = $$HEADERS $$SOURCES
      translation.commands  =  $$QTLUPDATE qt.pro 
      translationUp.target = pigale_fr.qm 
      translationUp.depends = $$TRANSLATIONS
      translationUp.commands  =  $$QTLRELEASE -verbose $$TRANSLATIONS
      QMAKE_EXTRA_TARGETS += translation translationUp 
      
      # needed by ../makefile     
      distdir.commands =
      QMAKE_EXTRA_TARGETS += distdir
     
      macx {   
          RC_FILE = pigale.icns
          #install
          target.path=$$DISTPATH/bin
          INSTALLS += target
          #Deployement
          man.path =  Contents/Resources
          man.files = ../Doc/manual.html ../Doc/style.css  pigale_fr.qm qt_fr.qm
          icon.path = Contents/Resources/icones
          icon.files += ../Doc/icones/misspost.png
          icon.files += ../Doc/icones/sleft.png
          icon.files += ../Doc/icones/sright.png
          icon.files += ../Doc/icones/sleftarrow.png
          icon.files += ../Doc/icones/srightarrow.png
          icon.files += ../Doc/icones/reload.png
          icon.files += ../Doc/icones/sreload.png
          icon.files += ../Doc/icones/filesave.png
          icon.files += ../Doc/icones/xman.png
          icon.files += ../Doc/icones/help.png
          icon.files += ../Doc/icones/macroplay.png
          icon.files += ../Doc/icones/sfilesave.png
          QMAKE_BUNDLE_DATA  += man icon
          } else {
          # Installation
          translations.files = pigale_fr.qm qt_fr.qm
          translations.path = $$DISTPATH/translations
          target.path =  $$DISTPATH/bin
          }
          
      # Distribution
      DISTDIR=..
      DISTFILES += gnumakefile
      PRE_TARGETDEPS = pigale_fr.qm ../incl/QT/Action.h
#      POST_TARGETDEPS += translation translationUp
}
win32 {
      # Installation
      target.path =  $$DISTPATH/bin
      translations.files = pigale_fr.qm qt_fr.qm
      translations.path = $$DISTPATH/translations
      doc.files = ..\\Doc\\*.html ..\\Doc\\*.css
      doc.path = $$DISTPATH/Doc
      ico.files = ../Doc/icones\\*.png ../Doc/icones\\*.gif
      ico.path  = $$DISTPATH/Doc/icones
      tgf.files = ../tgf\\*.*
      tgf.path = $$DISTPATH/tgf
      macro.files = ../macro/maxPlanar4Connex.mc
      macro.path = $$DISTPATH/macro
      lib.path = $$DISTPATH/lib
      lib.files = ../tgraph/libtgraph.a 

      INSTALLS += target translations doc ico tgf macro lib
      contains(DLL,"yes") {
           dll.files = $$MQTDIR/bin/QtCore4.dll $$MQTDIR/bin/QtGui4.dll
           dll.files += $$MQTDIR/bin/QtNetwork4.dll $$MQTDIR/bin/QtOpenGL4.dll $$MQTDIR/bin/QtXml4.dll
           dll.files += $$MQTDIR/bin/QtSql4.dll $$MQTDIR/bin/QtSvg4.dll
           dll.files += $$MINGW//bin/mingwm10.dll
           dll.files += $$MINGW//bin/libgcc_s_dw2-1.dll
           dll.path = $$DISTPATH/bin  
           INSTALLS += dll
      } 
}

build_pass:CONFIG(debug, debug|release) {
    message(Debug: configuring $$TARGET using QT version $$[QT_VERSION])
    } else:build_pass{
    message(Release: configuring $$TARGET using QT version $$[QT_VERSION])
}

#message(configuring $$TARGET using QT version $$[QT_VERSION])
     
