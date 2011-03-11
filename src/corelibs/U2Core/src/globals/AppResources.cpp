#include "AppResources.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/AppSettings.h>

#include <QtCore/QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#include <Psapi.h>
#include <Winbase.h> //for IsProcessorFeaturePresent
#endif

namespace U2 {

#define SETTINGS_ROOT QString("app_resource/")

AppResourcePool::AppResourcePool() {
    Settings* s = AppContext::getSettings();
    idealThreadCount = s->getValue(SETTINGS_ROOT + "idealThreadCount", QThread::idealThreadCount()).toInt();
    
    int maxThreadCount = s->getValue(SETTINGS_ROOT + "maxThreadCount", 1000).toInt();
    threadResource = new AppResource(RESOURCE_THREAD, maxThreadCount, tr("Threads"));
    registerResource(threadResource);

    int maxMem = s->getValue(SETTINGS_ROOT + "maxMem", 3 * 512).toInt(); //TODO: increase default value for 64bit ?
    memResource = new AppResource(RESOURCE_MEMORY, maxMem, tr("Memory"), tr("Mb"));
    registerResource(memResource);

    projectResouce = new AppResource(RESOURCE_PROJECT, 1, tr("Project"));
    registerResource(projectResouce);

    phyTreeResource = new AppResource(RESOURCE_PHYTREE, 1, tr("Phytree"));
        registerResource(phyTreeResource);
}

AppResourcePool::~AppResourcePool() {
    qDeleteAll(resources.values());
}

void AppResourcePool::setIdealThreadCount(int n) {
    assert(n > 0 && n <= threadResource->maxUse);
    n = qBound(1, n, threadResource->maxUse);
    idealThreadCount = n;
    AppContext::getSettings()->setValue(SETTINGS_ROOT + "idealThreadCount", idealThreadCount);
}

void AppResourcePool::setMaxThreadCount(int n) {
    assert(n >= 1);
    threadResource->maxUse = qMax(idealThreadCount, n);
    AppContext::getSettings()->setValue(SETTINGS_ROOT + "maxThreadCount", threadResource->maxUse );
}

void AppResourcePool::setMaxMemorySizeInMB(int n) {
    assert(n >= MIN_MEMORY_SIZE);
    memResource->maxUse = qMax(n, MIN_MEMORY_SIZE);
    AppContext::getSettings()->setValue(SETTINGS_ROOT + "maxMem", memResource->maxUse);
}

bool AppResourcePool::getCurrentAppMemory(int& mb) {
#ifdef Q_OS_WIN
    HANDLE procHandle = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    BOOL ok = GetProcessMemoryInfo(procHandle, &pmc, sizeof(procHandle));
    if (!ok) {
        return false;
    }
    mb = (int)(pmc.WorkingSetSize/(1024*1024));
    return true;    
#endif
    mb = MIN_MEMORY_SIZE;
    return false;
}

bool AppResourcePool::isSSE2Enabled() {
    bool answer = false;
#if defined( Q_OS_WIN )
    //Using WinAPI call on Windows.

    //Return Value
    //If the feature is supported, the return value is a nonzero value.
    //If the feature is not supported, the return value is zero.
    // 
    //If the HAL does not support detection of the feature, 
    //whether or not the hardware supports the feature, the return value is also zero. 
    //
    //Windows 2000: This feature is not supported.
    //
    //Header:  Winbase.h (include Windows.h)
    //Library: Kernel32.lib
    //DLL:     Kernel32.dll

    answer = (bool)IsProcessorFeaturePresent( PF_XMMI64_INSTRUCTIONS_AVAILABLE );
#elif defined( __amd64__ ) || defined( __AMD64__ ) || defined( __x86_64__ ) || defined( _M_X64 )
    answer = true;
#elif defined( __i386__ ) || defined( __X86__ ) || defined( _M_IX86 )
    //cpuid instruction: 
    //- takes 0x1 on eax,
    //- returns standard features flags in edx, bit 26 is SSE2 flag
    //- clobbers ebx, ecx
    unsigned int fflags = 0;
    unsigned int stub = 0;
    __asm__ __volatile__ (
        "push %%ebx; cpuid; mov %%ebx, %%edi; pop %%ebx" :
        "=a" (stub),
        "=D" (stub),
        "=c" (stub),
        "=d" (fflags) : "a" (0x1));
    answer = ((fflags & (1<<26))!=0);
#endif 
    return answer;
}

void AppResourcePool::registerResource(AppResource* r) {
    assert(!resources.contains(r->resourceId));
    resources[r->resourceId] = r;
}

AppResource* AppResourcePool::getResource(int id) const {
    return resources.value(id, NULL);
}


AppResourcePool* AppResourcePool::instance() {
    return AppContext::getAppSettings()->getAppResourcePool();
}

}//namespace
