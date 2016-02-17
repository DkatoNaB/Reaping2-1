#pragma once
#ifndef INCLUDED_CRASH_REPORT_CPP
#define INCLUDED_CRASH_REPORT_CPP

#include "singleton.h"
#include <string>

namespace platform {
class CrashReportImpl;
class CrashReport : public Singleton<CrashReport>
{
public:
    bool WriteDump();
private:
    friend class Singleton<CrashReport>;
    CrashReport();
    ~CrashReport();
    CrashReportImpl* mImpl;
};
}

#endif // INCLUDED_CRASH_REPORT_CPP

