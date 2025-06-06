SET(LIB_NAME osgdb_verse_ffmpeg)
SET(LIBRARY_FILES
    ReaderWriterFFmpeg.cpp ResourceDemuxer.h
    FFmpegClocks.cpp FFmpegClocks.hpp
    FFmpegDecoderAudio.cpp FFmpegDecoderAudio.hpp
    FFmpegDecoder.cpp FFmpegDecoder.hpp
    FFmpegDecoderVideo.cpp FFmpegDecoderVideo.hpp
    FFmpegImageStream.cpp FFmpegImageStream.hpp
    FFmpegAudioStream.cpp FFmpegAudioStream.hpp
    FFmpegParameters.cpp FFmpegParameters.hpp
    FFmpegHeaders.hpp FFmpegPacket.hpp
    BoundedMessageQueue.hpp MessageQueue.hpp
)

#########################################
IF(FFMPEG_FOUND)
    FILE(READ "${FFMPEG_INCLUDE_DIR}/libavutil/version.h" AVUTIL_FILE_CONTENTS)
    STRING(FIND "${AVUTIL_FILE_CONTENTS}" "LIBAVUTIL_VERSION_MAJOR" AVUTIL_VERSION_INDEX)
    MATH(EXPR AVUTIL_VERSION_INDEX "${AVUTIL_VERSION_INDEX} + 24")
    STRING(SUBSTRING "${AVUTIL_FILE_CONTENTS}" "${AVUTIL_VERSION_INDEX}" "4" AVUTIL_VERSION)
    STRING(STRIP ${AVUTIL_VERSION} AVUTIL_VERSION)
    IF(AVUTIL_VERSION LESS 57)
        MESSAGE("[osgVerse] FFmpeg found but version is too low (avutil: ${AVUTIL_VERSION} < 57). Plugin ffmpeg will be ignored.")
        SET(FFMPEG_FOUND FALSE)
    ENDIF()
ENDIF(FFMPEG_FOUND)

IF(FFMPEG_FOUND)
    SET_PROPERTY(GLOBAL APPEND PROPERTY VERSE_PLUGIN_LIBRARIES "${LIB_NAME}")
    ADD_DEFINITIONS(-D__STDC_CONSTANT_MACROS -DUSE_SWSCALE -DUSE_SWRESAMPLE)
    IF(VERSE_STATIC_BUILD)
        NEW_PLUGIN(${LIB_NAME} STATIC)
    ELSE()
        NEW_PLUGIN(${LIB_NAME} SHARED)
    ENDIF()

    SET_PROPERTY(TARGET ${LIB_NAME} PROPERTY FOLDER "PLUGINS")
    TARGET_COMPILE_OPTIONS(${LIB_NAME} PUBLIC -D_SCL_SECURE_NO_WARNINGS)
    TARGET_LINK_LIBRARIES(${LIB_NAME} osgVerseDependency avcodec avformat avdevice avutil swscale swresample)
    LINK_OSG_LIBRARY(${LIB_NAME} OpenThreads osg osgDB osgUtil)
    IF(MSVC)
        SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES LINK_FLAGS "/OPT:NOREF")
    ENDIF(MSVC)

    INSTALL(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}
            RUNTIME DESTINATION ${INSTALL_PLUGINDIR} COMPONENT libosgverse
            LIBRARY DESTINATION ${INSTALL_LIBDIR} COMPONENT libosgverse
            ARCHIVE DESTINATION ${INSTALL_ARCHIVEDIR} COMPONENT libosgverse-dev)
    IF(NOT VERSE_STATIC_BUILD)
        IF(MSVC AND VERSE_INSTALL_PDB_FILES)
            INSTALL(FILES $<TARGET_PDB_FILE:${LIB_NAME}> DESTINATION ${INSTALL_PLUGINDIR} OPTIONAL)
        ENDIF()
    ENDIF()

ENDIF(FFMPEG_FOUND)
