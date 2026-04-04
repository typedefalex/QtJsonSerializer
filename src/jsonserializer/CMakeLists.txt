qt_internal_add_module(JsonSerializer
    EXCEPTIONS

    SOURCES
    cborserializer_p.h
    cborserializer.cpp
    cborserializer.h
    exception_p.h
    exception.cpp
    exception.h
    exceptioncontext_p.h
    exceptioncontext.cpp
    jsonserializer_p.h
    jsonserializer.cpp
    jsonserializer.h
    metawriters_p.h
    metawriters.cpp
    metawriters.h
    qjsonreggen.py
    qtjsonserializer_global.h
    qtjsonserializer_helpertypes.h
    serializerbase_p.h
    serializerbase.cpp
    serializerbase.h
    typeconverter.cpp
    typeconverter.h
    typeextractors.h

    typeconverters/bitarrayconverter_p.h
    typeconverters/bitarrayconverter.cpp
    typeconverters/bytearrayconverter_p.h
    typeconverters/bytearrayconverter.cpp
    typeconverters/cborconverter_p.h
    typeconverters/cborconverter.cpp
    typeconverters/datetimeconverter_p.h
    typeconverters/datetimeconverter.cpp
    typeconverters/enumconverter_p.h
    typeconverters/enumconverter.cpp
    typeconverters/gadgetconverter_p.h
    typeconverters/gadgetconverter.cpp
    typeconverters/geomconverter_p.h
    typeconverters/geomconverter.cpp
    typeconverters/legacygeomconverter_p.h
    typeconverters/legacygeomconverter.cpp
    typeconverters/listconverter_p.h
    typeconverters/listconverter.cpp
    typeconverters/localeconverter_p.h
    typeconverters/localeconverter.cpp
    typeconverters/mapconverter_p.h
    typeconverters/mapconverter.cpp
    typeconverters/multimapconverter_p.h
    typeconverters/multimapconverter.cpp
    typeconverters/objectconverter_p.h
    typeconverters/objectconverter.cpp
    typeconverters/pairconverter_p.h
    typeconverters/pairconverter.cpp
    typeconverters/smartpointerconverter_p.h
    typeconverters/smartpointerconverter.cpp
    typeconverters/stdchronodurationconverter_p.h
    typeconverters/stdchronodurationconverter.cpp
    typeconverters/stdoptionalconverter_p.h
    typeconverters/stdoptionalconverter.cpp
    typeconverters/stdtupleconverter_p.h
    typeconverters/stdtupleconverter.cpp
    typeconverters/stdvariantconverter_p.h
    typeconverters/stdvariantconverter.cpp
    typeconverters/versionnumberconverter_p.h
    typeconverters/versionnumberconverter.cpp

    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
    GENERATE_CPP_EXPORTS
)

set(JSON_TYPES
    bool
    char
    "signed char"
    uchar
    short
    ushort
    int
    uint
    long
    ulong
    qlonglong
    qulonglong
    float
    double
    QObject*
    QChar
    QString
    QDate
    QTime
    QDateTime
    QUrl
    QUuid
    QCborValue
    QCborMap
    QCborArray
    QJsonValue
    QJsonObject
    QJsonArray
    QMimeType
    QVersionNumber
    QLocale
    QRegularExpression
    QSize
    QPoint
    QLine
    QRect
    QSizeF
    QPointF
    QLineF
    QRectF
    QByteArray
)

set(QSize_modes list)
set(QPoint_modes list)
set(QLine_modes list)
set(QRect_modes list)
set(QSizeF_modes list)
set(QPointF_modes list)
set(QLineF_modes list)
set(QRectF_modes list)
set(QByteArray_modes map set)

set(QT_JSONSERIALIZER_REGGEN_DIR ${CMAKE_BINARY_DIR}/.reggen)
file(MAKE_DIRECTORY ${QT_JSONSERIALIZER_REGGEN_DIR})
set(QT_JSONSERIALIZER_TYPESPLIT_PY ${CMAKE_CURRENT_SOURCE_DIR}/qjsonreggen.py)

foreach(type IN LISTS JSON_TYPES)
    string(REGEX REPLACE "[^A-Za-z0-9]" "_" type_base "${type}")
    set(target_base qjsonconverterreg_${type_base}.cpp)
    cmake_path(ABSOLUTE_PATH target_base BASE_DIRECTORY ${QT_JSONSERIALIZER_REGGEN_DIR} OUTPUT_VARIABLE target_path)

    add_custom_command(
        OUTPUT ${target_path}
        COMMAND ${QT_JSONSERIALIZER_TYPESPLIT_PY} ${target_path} ${type} ${${type_base}_modes}
    )

    qt_internal_extend_target(JsonSerializer
        SOURCES
        ${target_path}
    )
endforeach()

set(escaped_types "")
foreach(type IN LISTS JSON_TYPES)
    list(APPEND escaped_types ${type})
endforeach()

set(hook_file_name "qjsonconverterreg_hook.cpp")
cmake_path(ABSOLUTE_PATH hook_file_name BASE_DIRECTORY ${QT_JSONSERIALIZER_REGGEN_DIR} OUTPUT_VARIABLE hook_target_path)

add_custom_command(
    OUTPUT ${hook_target_path}
    COMMAND ${QT_JSONSERIALIZER_TYPESPLIT_PY} super ${hook_target_path} ${escaped_types}
    VERBATIM
)

qt_internal_extend_target(JsonSerializer
    SOURCES
    ${hook_target_path}
)

# TODO
# see https://github.com/qt/qthttpserver/tree/6.5.0
# https://github.com/qt/qtwebchannel/tree/dev
# https://github.com/qt/qtnetworkauth/tree/6.5.0/src/oauth
# qt_internal_add_docs(JsonSerializer
#                      ../../doc/config/qthttpserver.qdocconf
# )
