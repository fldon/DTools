#include <cmath>
#include <map>
#include <unistd.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <source_location>
#include <stacktrace>

#include "DTools/debug.h"

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

[[nodiscard]] constexpr double clamp_d(double value, double min_val, double max_val)
{
    if(value > max_val) return max_val;
    if(value < min_val) return min_val;
    return value;
}

//Generic clamping of addition/multiplication
//TODO: maybe find a more generic way to do this...
[[nodiscard]] constexpr double add_clamp_d(double lhs, double rhs, double min_val, double max_val)
{
    //For doubles this should work because there is no overrun, only INF and - INF
    const double result = lhs + rhs;
    if(result > max_val) return max_val;
    if(result < min_val) return min_val;
    DEBUG_ASSERT(result <= max_val && result >= min_val);
    return result;
}

[[nodiscard]] constexpr double mult_clamp_d(double lhs, double rhs, double min_val, double max_val)
{
    //For doubles this should work because there is no overrun, only INF and - INF
    const double result = lhs * rhs;
    if(result > max_val) return max_val;
    if(result < min_val) return min_val;
    DEBUG_ASSERT(result <= max_val && result >= min_val);
    return result;
}

//"wrapping" of added values (back to max if under min and vice versa)
[[nodiscard]] constexpr double add_wrap_d(double lhs, double rhs, double min_val, double max_val)
{
    //For doubles this should work because there is no overrun, only INF and - INF
    const double result = lhs + rhs;
    if(result > max_val) return add_wrap_d(result - max_val, min_val, min_val, max_val);
    if(result < min_val) return add_wrap_d(result - min_val, max_val, min_val, max_val);
    DEBUG_ASSERT(result <= max_val && result >= min_val);
    return result;
}

[[nodiscard]] constexpr double mult_wrap_d(double lhs, double rhs, double min_val, double max_val)
{
    //For doubles this should work because there is no overrun, only INF and - INF
    const double result = lhs * rhs;
    if(result > max_val) return mult_wrap_d(result - max_val, min_val, min_val, max_val);
    if(result < min_val) return mult_wrap_d(result - min_val, max_val, min_val, max_val);
    DEBUG_ASSERT(result <= max_val && result >= min_val);
    return result;
}


//Interpolate any function as Dependend = func(Independend) with given stepsize
//returns pairs of datapoints in the form <Dependend, Independend>
template<typename Independend, typename Dependend, typename Functor>
[[nodiscard]] constexpr std::vector<std::pair<Independend, Dependend>> interpolate(Independend t0, Independend t1,
                                                                                   Independend stepsize, Functor func)
{
    using std::swap;
    //Make sure t0 is < t1 for simplicity
    //TODO: maybe this adds too many constraints on the types
    if(std::signbit(stepsize) != std::signbit(t1 - t0))
    {
        throw BaseOmegaException("stepsize has different sign than difference of range");
    }

    std::vector<std::pair<Independend, Dependend>> result;
    for(Independend step = t0; std::abs(step - t0) <= std::abs(t1 - t0); step = step + stepsize)
    {
        const std::pair<Independend, Dependend> datapoint = std::make_pair(step, func(step));
        result.push_back(datapoint);
    }
    return result;
}

//Returns pairs of <independend, dependend> of a line from i_0,d_0 to i_1,d_1 in stepsize steps
[[nodiscard]] constexpr std::vector< std::pair<double, double> > interpolate_line_d_d(double independend_0, double independend_1,
                                                                                        double dependend_0, double dependend_1,
                                                                                        double stepsize)
{
    std::vector< std::pair<double, double> > result;

    if(std::signbit(stepsize) != std::signbit(independend_1 - independend_0))
    {
        throw BaseOmegaException("stepsize has different sign than difference of range");
    }

    if(stepsize == 0)
    {
        throw BaseOmegaException("stepsize is 0");
    }

    if(std::abs(independend_1 - independend_0) < std::abs(stepsize))
    {
        result.push_back(std::make_pair(independend_0, dependend_0));
        return result;
    }

    const double slope = (dependend_1 - dependend_0) / (independend_1 - independend_0);

    double curr_dependend = dependend_0;
    for(double step = independend_0; std::abs(step - independend_0) <= std::abs(independend_1 - independend_0); step += stepsize )
    {
        result.emplace_back(std::make_pair(step, curr_dependend));
        curr_dependend += slope * stepsize;
    }
    return result;
}

//Returns pairs of <independend, dependend> of a line from i_0,d_0 to i_1,d_1 in stepsize steps
//independend is int
[[nodiscard]] constexpr std::map<int, double>  interpolate_line_i_d(int independend_0, int independend_1,
                                                                                    double dependend_0, double dependend_1,
                                                                                    int stepsize = 1)
{
    std::map<int, double> result;

    if(std::signbit(stepsize) != std::signbit(independend_1 - independend_0))
    {
        throw BaseOmegaException("stepsize has different sign than difference of range");
    }

    if(stepsize == 0)
    {
        throw BaseOmegaException("stepsize is 0");
    }

    if(std::abs(independend_1 - independend_0) < std::abs(stepsize))
    {
        result.insert(std::make_pair(independend_0, dependend_0));
        return result;
    }

    const double slope = (dependend_1 - dependend_0) / (independend_1 - independend_0);

    double curr_dependend = dependend_0;
    for(int step = independend_0; std::abs(step - independend_0) <= std::abs(independend_1 - independend_0); step += stepsize )
    {
        result.insert(std::make_pair(step, curr_dependend));
        curr_dependend += slope * stepsize;
    }
    return result;
}

}//N_misc


} //dtools NS

#endif
