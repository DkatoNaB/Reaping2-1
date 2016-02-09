#include "crash_report.h"
#include "log.h"
#include <boost/predef.h>
#include <boost/filesystem.hpp>

#if ( BOOST_OS_MACOS )
#include <client/mac/handler/exception_handler.h>
#elif ( BOOST_OS_LINUX )
#include <client/linux/handler/exception_handler.h>
#elif ( BOOST_OS_WINDOWS )
#include <client/windows/handler/exception_handler.h>
#endif

namespace platform {

struct CrashReportImpl
{
    CrashReportImpl()
        : mHandler( NULL )
    {
    }

    ~CrashReportImpl()
    {
        delete mHandler;
    }

    void InitCrashReport( boost::filesystem::path const& path);
    google_breakpad::ExceptionHandler* mHandler;
    bool mReportCrashesToSystem;
};

#if ( BOOST_OS_WINDOWS )
bool DumpCallback( const wchar_t* _dump_dir,
        const wchar_t* _minidump_id,
        void* context,
        EXCEPTION_POINTERS* exinfo,
        MDRawAssertionInfo* assertion,
        bool success)
#elif ( BOOST_OS_LINUX )
bool DumpCallback( const google_breakpad::MinidumpDescriptor &md, void *context, bool success)
#elif ( BOOST_OS_MACOS )
bool DumpCallback( const char* _dump_dir, const char* _minidump_id, void *context, bool success)
#endif
{
    L1( "Breakpad dump callback" );

    // NO STACK USE, NO HEAP USE THERE !!!
    return success;
}

void CrashReportImpl::InitCrashReport( boost::filesystem::path const & path )
{
    if ( mHandler != NULL )
        return;

#if ( BOOST_OS_WINDOWS )
    mHandler = new google_breakpad::ExceptionHandler(
        path.native(),
        /*FilterCallback*/ 0,
        DumpCallback,
        /*context*/
        0,
        true
        );
#elif ( BOOST_OS_LINUX )
    google_breakpad::MinidumpDescriptor md( path.native() );
    mHandler = new google_breakpad::ExceptionHandler(
        md,
        /*FilterCallback*/ 0,
        DumpCallback,
        /*context*/ 0,
        true,
        -1
        );
#elif ( BOOST_OS_MACOS )
    mHandler = new google_breakpad::ExceptionHandler(
        path.native(),
        /*FilterCallback*/ 0,
        DumpCallback,
        /*context*/
        0,
        true,
        NULL
        );
#endif
}

CrashReport::CrashReport()
{
    mImpl = new CrashReportImpl();
    mImpl->InitCrashReport( boost::filesystem::temp_directory_path() );
}

CrashReport::~CrashReport()
{
    delete mImpl;
}

bool CrashReport::WriteDump()
{
    bool res = mImpl->mHandler->WriteMinidump();
    L1( "WriteDump %s", ( res ? "success" : "FAILURE" ) );
    return res;
}

}

