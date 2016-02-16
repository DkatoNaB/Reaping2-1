cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR})

include( FindBreakpadTools )

if( NOT BREAKPAD_DUMPSYMS_EXE_FOUND )
    message( SEND_ERROR "Cannot dump symbols, breakpad tools not found!" )
endif()

get_filename_component( BIN_FNAME ${BINARY} NAME )
get_filename_component( BIN_DIR ${BINARY} DIRECTORY )
set( SYMBOL_NAME ${BIN_FNAME}.sym )
set( SYMBOL ${BIN_DIR}/${SYMBOL_NAME} )

message( "Generating symbols for " ${BINARY} " to " ${SYMBOL_NAME})

execute_process(
    COMMAND ${BREAKPAD_DUMPSYMS_EXE} ${BINARY}
    OUTPUT_FILE ${SYMBOL}
    )

file( STRINGS ${SYMBOL} HEAD LIMIT_COUNT 1 )
string( REPLACE " " ";" HEAD_LIST ${HEAD} )
message( "Sym head: " ${HEAD} )
list( GET HEAD_LIST -1 SYM_NAME )
list( GET HEAD_LIST -2 DEBUG_ID )
set( OUTDIR_FINAL ${OUTDIR}/${SYM_NAME}/${DEBUG_ID} )
message( "Dir: " ${OUTDIR_FINAL} )
file( MAKE_DIRECTORY ${OUTDIR_FINAL} )
file( COPY ${SYMBOL} DESTINATION ${OUTDIR_FINAL} )
