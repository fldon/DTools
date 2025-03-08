#include <unistd.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <source_location>
#include <stacktrace>

#ifndef MISCTOOLS_H
#define MISCTOOLS_H

namespace NS_dtools
{

namespace NS_misc
{

constexpr inline unsigned long long floorlog2(unsigned long long x)
{
    unsigned long long result = 0;
    while(x != 1)
    {
        ++result;
        x = x >> 1;
    }
    return result;
}

constexpr inline unsigned long long ceillog2(unsigned long long x)
{
    const unsigned long long result = x == 1 ? 0 : floorlog2(x - 1) + 1;
    return result;
}

//////////////////////////////////////////////////////////////////////////////
//
// process_mem_usage(double &, double &) - takes two doubles by reference,
// attempts to read the system-dependent data for a process' virtual memory
// size and resident set size, and return the results in KB.
//
// On failure, returns 0.0, 0.0

inline void process_mem_usage(double& vm_usage, double& resident_set)
{
    using std::ios_base;
    using std::ifstream;
    using std::string;

    vm_usage     = 0.0;
    resident_set = 0.0;

    // 'file' stat seems to give the most reliable results
    //
    ifstream stat_stream("/proc/self/stat",ios_base::in);

    // dummy vars for leading entries in stat that we don't care about
    //
    string pid, comm, state, ppid, pgrp, session, tty_nr;
    string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    string utime, stime, cutime, cstime, priority, nice;
    string O, itrealvalue, starttime;

    // the two fields we want
    //
    unsigned long vsize;
    long rss;

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
        >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
        >> utime >> stime >> cutime >> cstime >> priority >> nice
        >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

    stat_stream.close();

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage     = vsize / 1024.0;
    resident_set = rss * page_size_kb;
}


class BaseOmegaException : public std::exception
{
public:
    BaseOmegaException(std::string str, const std::source_location &loc =
                                                std::source_location::current(), std::stacktrace trace = std::stacktrace::current())
        :m_err_str{std::move(str)}, m_location(loc), m_backtrace{trace} {}

    const char* what() const noexcept override {return m_err_str.c_str();}
    const std::source_location& where() const noexcept {return m_location;}
    const std::stacktrace& stack() const noexcept {return m_backtrace;}

private:
    std::string m_err_str;
    std::source_location m_location;
    std::stacktrace m_backtrace;
};

/*!
 * \brief OmegaException class as proposed by Peter Muldoon
 * Can "carry" an arbitrary data object as well as a string. If it bubbles up, it prints a stacktrace.
 */
template<typename DATA_T>
class OmegaException : public BaseOmegaException
{
public:
    OmegaException(std::string str, DATA_T data, const std::source_location &loc =
                                                 std::source_location::current(), std::stacktrace trace = std::stacktrace::current())
        :BaseOmegaException(str, loc, trace), m_user_data{std::move(data)} {}

    DATA_T& data() {return m_user_data;}
    const DATA_T& data() const noexcept {return m_user_data;}
private:
    DATA_T m_user_data;
};

}


} //dtools NS

#endif
