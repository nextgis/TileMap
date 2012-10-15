#include(updateqm.pri)
isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
PRE_TARGETDEPS += compiler_updateqm_make_all

TEMPLATE = lib
QT =\
  core\
  gui\
  xml
DEFINES +=\
  CORE_EXPORT=\
  GUI_EXPORT=
INCLUDEPATH +=\
  C:/Programme/OSGeo4W/include\
  C:/Programme/OSGeo4W/apps/qgis/include\
  "C:/Program Files (x86)/Quantum GIS Lisboa/include"
LIBS =\
  C:/Programme/OSGeo4W/apps/qgis/lib/qgis_core.lib\
  C:/Programme/OSGeo4W/apps/qgis/lib/qgis_gui.lib\
  C:/Programme/OSGeo4W/lib/zlib.lib
HEADERS =\
  dialog.h\
  plugin.h\
  qzipreader_p.h\
  qzipwriter_p.h\
  thread.h\
  tile.h
SOURCES =\
  dialog.cpp\
  export.cpp\
  plugin.cpp\
  qzip.cpp\
  thread.cpp\
  tile.cpp
FORMS =\
  dialog.ui
RESOURCES = tile_map_plugin.qrc
TRANSLATIONS = i18n/tile_map_ru.ts
