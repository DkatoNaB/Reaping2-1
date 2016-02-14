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
namespace {
namespace buffers {
#if ( BOOST_OS_WINDOWS )
typedef wchar_t char_type;
#define string_length wstrlen
#else
typedef char char_type;
#define string_length strlen
#endif
size_t const MaxNameSize = 1024;
char_type NameBuffer[ MaxNameSize ] = { 0 };
size_t DirSize = 0;
size_t NameSize = 0;
}
}

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
    std::string mCustomData;
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
#if ( BOOST_OS_LINUX )
    char const* _minidump_id = md.path();
    buffers::NameSize = string_length( _minidump_id ) * sizeof( buffers::char_type );
    memcpy( buffers::NameBuffer, _minidump_id, buffers::NameSize );
#else
    buffers::DirSize = string_length( _dump_dir ) * sizeof( buffers::char_type );
    buffers::NameSize = string_length( _minidump_id ) * sizeof( buffers::char_type );
    memcpy( buffers::NameBuffer, _dump_dir, buffers::DirSize);
    memcpy( buffers::NameBuffer + buffers::DirSize, _minidump_id, buffers::NameSize );
#endif
    std::cerr << "Dumped " << buffers::NameBuffer << "\n";
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

void CrashReport::SetCustomData( std::string const& customData )
{
    mImpl->mCustomData = customData;
}

}

