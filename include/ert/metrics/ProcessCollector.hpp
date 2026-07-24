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

#pragma once

#include <prometheus/collectable.h>
#include <prometheus/metric_family.h>

#include <string>
#include <vector>

namespace ert
{
namespace metrics
{

/**
 * Collects process-level metrics from /proc/self on Linux.
 *
 * Exposes the following metrics (compatible with Go client_golang conventions):
 * - process_cpu_seconds_total: Total user and system CPU time spent in seconds.
 * - process_resident_memory_bytes: Resident memory size in bytes (RSS).
 * - process_virtual_memory_bytes: Virtual memory size in bytes.
 * - process_open_fds: Number of open file descriptors.
 * - process_max_fds: Maximum number of open file descriptors.
 * - process_threads: Number of OS threads in the process.
 *
 * All metrics include a "source" label to identify the process instance.
 */
class ProcessCollector : public prometheus::Collectable {
public:
    /**
     * Constructs a ProcessCollector.
     *
     * @param source Label value for the "source" label on all metrics.
     *              Typically the application/process name (e.g. "h2agent").
     */
    explicit ProcessCollector(const std::string &source);

    /**
     * Collects current process metrics by reading /proc/self.
     * Called by the Exposer on each scrape.
     *
     * @return Vector of MetricFamily with current process metrics.
     */
    std::vector<prometheus::MetricFamily> Collect() const override;

private:
    std::string source_;

    // Clock ticks per second (sysconf(_SC_CLK_TCK))
    long clock_ticks_;

    // Helper methods
    double getCpuSeconds() const;
    double getResidentMemoryBytes() const;
    double getVirtualMemoryBytes() const;
    double getOpenFds() const;
    double getMaxFds() const;
    double getThreads() const;
};

}
}
