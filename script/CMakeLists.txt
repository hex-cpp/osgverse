SET(LIB_NAME osgVerseScript)
SET(LIBRARY_INCLUDE_FILES
    Entry.h ScriptBase.h JsonScript.h PythonScript.h
    GenericReserializer.h GenericInputStream.h
)
SET(LIBRARY_FILES ${LIBRARY_INCLUDE_FILES}
    Entry.cpp ScriptBase.cpp
    JsonScript.cpp PythonScript.cpp PythonWrapper_osg.cpp
    GenericReserializer.cpp GenericInputStream.cpp
)

INCLUDE_DIRECTORIES(. ../3rdparty/libhv ../3rdparty/libhv/all)
ADD_DEFINITIONS(-DHV_STATICLIB)
IF(PYTHON_FOUND)
    ADD_DEFINITIONS(-DWITH_PYTHON)
ENDIF(PYTHON_FOUND)
NEW_LIBRARY(${LIB_NAME} STATIC)

TARGET_LINK_LIBRARIES(${LIB_NAME} osgVerseDependency osgVersePipeline)
TARGET_COMPILE_OPTIONS(${LIB_NAME} PUBLIC -D_SCL_SECURE_NO_WARNINGS)

INSTALL(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}
        RUNTIME DESTINATION ${INSTALL_BINDIR} COMPONENT libosgverse
        LIBRARY DESTINATION ${INSTALL_LIBDIR} COMPONENT libosgverse
        ARCHIVE DESTINATION ${INSTALL_ARCHIVEDIR} COMPONENT libosgverse-dev)
INSTALL(FILES ${LIBRARY_INCLUDE_FILES} DESTINATION ${INSTALL_INCDIR}/osgVerse/ui COMPONENT libosgverse-dev)
