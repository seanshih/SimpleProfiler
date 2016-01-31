#include "windows.h"
#include "Profiler.h"
#include <vector>
#include <dbghelp.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <iomanip>
#include <algorithm>
#include <numeric>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "winmm.lib")

static HANDLE s_main_thread = 0;
static size_t s_count;
static size_t s_init_count;
static std::vector<DWORD> s_records;
static std::string s_path;

void CALLBACK ProfilerFunction(
    UINT uTimerID,
    UINT uMsg,
    DWORD_PTR dwUser,
    DWORD_PTR dw1,
    DWORD_PTR dw2)
{
    if (s_count == 0)
        ExitThread(0);

    s_count--;

    SuspendThread(s_main_thread);
    CONTEXT context = { 0 };
    context.ContextFlags = CONTEXT_i386 | CONTEXT_CONTROL;
    GetThreadContext(s_main_thread, &context);
    s_records.emplace_back(context.Eip);
    ResumeThread(s_main_thread);
}


Profiler::Profiler(unsigned sample_count, const char* report_file_name)
{
    s_count = sample_count;
    s_init_count = s_count;

    s_records.reserve(s_count);

    s_main_thread = OpenThread(
        THREAD_SUSPEND_RESUME |
        THREAD_GET_CONTEXT |
        THREAD_QUERY_INFORMATION,
        0,
        GetCurrentThreadId());

    timeSetEvent(1, 1, ProfilerFunction, 0,
        TIME_PERIODIC | TIME_CALLBACK_FUNCTION | TIME_KILL_SYNCHRONOUS);

    s_path = report_file_name;
}

Profiler::~Profiler()
{
    ////////////////////////
    // get symbol info from eip, and put them into statistics
    ////////////////////////
    const auto result0 = SymInitialize(GetCurrentProcess(), nullptr, true);

    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = { 0 };
    PSYMBOL_INFO func_info = (PSYMBOL_INFO)buffer;
    func_info->SizeOfStruct = sizeof(SYMBOL_INFO);
    func_info->MaxNameLen = MAX_SYM_NAME;
    func_info->Flags = SYMFLAG_FUNCTION;

    std::unordered_map<std::string, unsigned> statistics;
    for (auto eip : s_records)
    {
        auto result = SymFromAddr(GetCurrentProcess(), eip, nullptr, func_info);
        std::string func_name = func_info->Name;
        auto iter = statistics.find(func_name);
        if (iter == statistics.end())
            iter = statistics.emplace(func_name, 0).first;
        ++(iter->second);
    }

    ////////////////////////
    // sort the statistics
    ////////////////////////
    typedef std::pair<std::string, unsigned> pair_t;
    std::vector <pair_t> sorted_pairs(statistics.begin(), statistics.end());
    std::make_heap(sorted_pairs.begin(), sorted_pairs.end());

    std::sort(sorted_pairs.begin(), sorted_pairs.end(), [](const pair_t& pair1, const pair_t& pair2){
        return (pair1.second > pair2.second)
            || ((pair1.second == pair2.second) && (pair1.first.compare(pair2.first) > 0));

    });


    ////////////////////////
    // output statistics
    ////////////////////////
    typedef std::pair<std::string, unsigned> pair_t;
    std::fstream ost(s_path.c_str(), std::ios_base::out);

    const auto total_count = s_init_count - s_count;

    for (auto& info : sorted_pairs)
    {
        ost << std::setw(8) << std::setprecision(3) << std::fixed << std::right
            << (100.f * info.second) / total_count << " %  " /*count*/ 
            << info.first /*name*/ 
            << "\n";
    }

    ost << "\nOverall " << total_count << " samples collected.\n\n";
    ost.close();

    system("sh report_gen.sh");

    ////////////////////////
    // Reset count 
    ////////////////////////
    s_count = 0;
}

