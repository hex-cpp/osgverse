SET(LIB_NAME osgVerseAI)
SET(LIBRARY_INCLUDE_FILES RecastManager.h McpServer.h)
SET(LIBRARY_FILES ${LIBRARY_INCLUDE_FILES}
    RecastManager.cpp RecastManager_Private.h
    RecastManager_Builder.cpp RecastManager_Builder.h
    McpServer.cpp)

INCLUDE_DIRECTORIES(../3rdparty/recastnavigation/Detour)
INCLUDE_DIRECTORIES(../3rdparty/libhv ../3rdparty/libhv/all
                    ../3rdparty/stb ../3rdparty/rapidjson)
ADD_DEFINITIONS(-DHV_STATICLIB)

NEW_LIBRARY(${LIB_NAME} STATIC)
TARGET_LINK_LIBRARIES(${LIB_NAME} osgVerseDependency osgVerseModeling)
TARGET_COMPILE_OPTIONS(${LIB_NAME} PUBLIC -D_SCL_SECURE_NO_WARNINGS)

INSTALL(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}
        RUNTIME DESTINATION ${INSTALL_BINDIR} COMPONENT libosgverse
        LIBRARY DESTINATION ${INSTALL_LIBDIR} COMPONENT libosgverse
        ARCHIVE DESTINATION ${INSTALL_ARCHIVEDIR} COMPONENT libosgverse-dev)
INSTALL(FILES ${LIBRARY_INCLUDE_FILES} DESTINATION ${INSTALL_INCDIR}/osgVerse/ai COMPONENT libosgverse-dev)
