/*
 _____________________________________________________________
|             _                         _        _            |
|            | |                       | |      (_)           |
|    ___ _ __| |_   __   _ __ ___   ___| |_ _ __ _  ___ ___   |  Metrics wrapper library C++
|   / _ \ '__| __| |__| | '_ ` _ \ / _ \ __| '__| |/ __/ __|  |  Version 1.0.z
|  |  __/ |  | |_       | | | | | |  __/ |_| |  | | (__\__ \  |  https://github.com/testillano/metrics
|   \___|_|   \__|      |_| |_| |_|\___|\__|_|  |_|\___|___/  |
|_____________________________________________________________|

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2021 Eduardo Ramos

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <ert/metrics/ProcessCollector.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <dirent.h>
#include <unistd.h>

namespace ert
{
namespace metrics
{

ProcessCollector::ProcessCollector(const std::string &source)
    : source_(source)
    , clock_ticks_(sysconf(_SC_CLK_TCK))
{
}

double ProcessCollector::getCpuSeconds() const
{
    std::ifstream stat("/proc/self/stat");
    if (!stat.is_open()) return 0.0;

    std::string line;
    std::getline(stat, line);

    // Skip past the comm field (which may contain spaces/parens)
    // Format: pid (comm) state ... field14=utime field15=stime ...
    auto pos = line.rfind(')');
    if (pos == std::string::npos) return 0.0;

    std::istringstream iss(line.substr(pos + 2)); // skip ") "
    std::string token;

    // Fields after comm: state(1) ppid(2) pgrp(3) session(4) tty_nr(5)
    // tpgid(6) flags(7) minflt(8) cminflt(9) majflt(10) cmajflt(11)
    // utime(12) stime(13)
    for (int i = 1; i <= 11; ++i) {
        if (!(iss >> token)) return 0.0;
    }

    unsigned long long utime = 0, stime = 0;
    if (!(iss >> utime >> stime)) return 0.0;

    return static_cast<double>(utime + stime) / static_cast<double>(clock_ticks_);
}

double ProcessCollector::getResidentMemoryBytes() const
{
    std::ifstream stat("/proc/self/stat");
    if (!stat.is_open()) return 0.0;

    std::string line;
    std::getline(stat, line);

    auto pos = line.rfind(')');
    if (pos == std::string::npos) return 0.0;

    std::istringstream iss(line.substr(pos + 2));
    std::string token;

    // Fields after comm: skip to rss which is field 22 (index 21 from after comm)
    // state(1)..vsize(21) rss(22)
    for (int i = 1; i <= 21; ++i) {
        if (!(iss >> token)) return 0.0;
    }

    long rss_pages = 0;
    if (!(iss >> rss_pages)) return 0.0;

    long page_size = sysconf(_SC_PAGESIZE);
    return static_cast<double>(rss_pages) * static_cast<double>(page_size);
}

double ProcessCollector::getVirtualMemoryBytes() const
{
    std::ifstream stat("/proc/self/stat");
    if (!stat.is_open()) return 0.0;

    std::string line;
    std::getline(stat, line);

    auto pos = line.rfind(')');
    if (pos == std::string::npos) return 0.0;

    std::istringstream iss(line.substr(pos + 2));
    std::string token;

    // Fields after comm: vsize is field 21 (index 20 from after comm)
    for (int i = 1; i <= 20; ++i) {
        if (!(iss >> token)) return 0.0;
    }

    unsigned long long vsize = 0;
    if (!(iss >> vsize)) return 0.0;

    return static_cast<double>(vsize);
}

double ProcessCollector::getOpenFds() const
{
    int count = 0;
    DIR *dir = opendir("/proc/self/fd");
    if (!dir) return 0.0;

    while (readdir(dir) != nullptr) {
        ++count;
    }
    closedir(dir);

    // Subtract 2 for "." and "..", and 1 for the dir fd opened by opendir
    return static_cast<double>(count > 3 ? count - 3 : 0);
}

double ProcessCollector::getMaxFds() const
{
    std::ifstream limits("/proc/self/limits");
    if (!limits.is_open()) return 0.0;

    std::string line;
    while (std::getline(limits, line)) {
        if (line.find("Max open files") != std::string::npos) {
            std::istringstream iss(line.substr(line.find("Max open files") + 14));
            std::string soft;
            iss >> soft; // skip whitespace then read soft limit
            unsigned long long val = 0;
            try {
                val = std::stoull(soft);
            } catch (...) {
                return 0.0;
            }
            return static_cast<double>(val);
        }
    }

    return 0.0;
}

double ProcessCollector::getThreads() const
{
    std::ifstream status("/proc/self/status");
    if (!status.is_open()) return 0.0;

    std::string line;
    while (std::getline(status, line)) {
        if (line.find("Threads:") == 0) {
            std::istringstream iss(line.substr(8));
            int threads = 0;
            if (iss >> threads) {
                return static_cast<double>(threads);
            }
        }
    }

    return 0.0;
}

std::vector<prometheus::MetricFamily> ProcessCollector::Collect() const
{
    std::vector<prometheus::MetricFamily> families;
    families.reserve(6);

    prometheus::ClientMetric::Label source_label;
    source_label.name = "source";
    source_label.value = source_;

    // Helper lambda to build a gauge metric family
    auto makeGaugeFamily = [&](const std::string &name, const std::string &help, double value) {
        prometheus::MetricFamily family;
        family.name = name;
        family.help = help;
        family.type = prometheus::MetricType::Gauge;

        prometheus::ClientMetric metric;
        metric.label.push_back(source_label);
        metric.gauge.value = value;
        family.metric.push_back(std::move(metric));

        return family;
    };

    // Helper lambda to build a counter metric family
    auto makeCounterFamily = [&](const std::string &name, const std::string &help, double value) {
        prometheus::MetricFamily family;
        family.name = name;
        family.help = help;
        family.type = prometheus::MetricType::Counter;

        prometheus::ClientMetric metric;
        metric.label.push_back(source_label);
        metric.counter.value = value;
        family.metric.push_back(std::move(metric));

        return family;
    };

    families.push_back(makeCounterFamily(
        "process_cpu_seconds_total",
        "Total user and system CPU time spent in seconds.",
        getCpuSeconds()));

    families.push_back(makeGaugeFamily(
        "process_resident_memory_bytes",
        "Resident memory size in bytes.",
        getResidentMemoryBytes()));

    families.push_back(makeGaugeFamily(
        "process_virtual_memory_bytes",
        "Virtual memory size in bytes.",
        getVirtualMemoryBytes()));

    families.push_back(makeGaugeFamily(
        "process_open_fds",
        "Number of open file descriptors.",
        getOpenFds()));

    families.push_back(makeGaugeFamily(
        "process_max_fds",
        "Maximum number of open file descriptors.",
        getMaxFds()));

    families.push_back(makeGaugeFamily(
        "process_threads",
        "Number of OS threads in the process.",
        getThreads()));

    return families;
}

}
}
