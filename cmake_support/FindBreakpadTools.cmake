# https://github.com/johnjianfang/megaglestng/blob/master/mk/cmake/Modules/FindGoogleBreakpad.cmake

FIND_PROGRAM(BREAKPAD_DUMPSYMS_EXE
    dump_syms NAMES dumpsyms dump_syms.exe
    PATHS
    ENV
    PATH
    /usr/local/bin/
    /usr/bin/

    )
MESSAGE(STATUS "Looking for dump-symbols result: ${BREAKPAD_DUMPSYMS_EXE}" )
IF(BREAKPAD_DUMPSYMS_EXE)
    SET(BREAKPAD_DUMPSYMS_EXE_FOUND TRUE)

    MESSAGE(STATUS "*** FOUND BREAKPAD TOOLS ${BREAKPAD_DUMPSYMS_EXE}}")

ELSE(BREAKPAD_DUMPSYMS_EXE)
    SET(BREAKPAD_DUMPSYMS_EXE_FOUND FALSE)
    #SET(BREAKPAD_FOUND FALSE)
ENDIF(BREAKPAD_DUMPSYMS_EXE)

