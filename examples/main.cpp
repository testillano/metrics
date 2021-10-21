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

// C
#include <libgen.h> // basename
#include <signal.h>

// Standard
#include <iostream>
#include <string>
#include <unistd.h>
#include <thread>
#include <vector>

#include <ert/tracing/Logger.hpp>

#include <ert/metrics/Metrics.hpp>

const char* progname;

class Observer;
Observer *G_observer;


class Observer {
    ert::metrics::counter_t *count_sigusr1_;
    ert::metrics::counter_t *count_sigusr2_;
    ert::metrics::gauge_t *gauge_;
    ert::metrics::histogram_t *histogram_;
    ert::metrics::bucket_boundaries_t histogram_boundaries_;

public:

    Observer(ert::metrics::Metrics *metrics) {
        try {
            ert::metrics::counter_family_ref_t cf = metrics->addCounterFamily("observed_user_signals_total", "Number of observed user signals");
            ert::metrics::gauge_family_ref_t gf = metrics->addGaugeFamily("current_seconds", "Number of observed user signals");
            ert::metrics::histogram_family_ref_t hf = metrics->addHistogramFamily("random_time_responses", "Random response time");
            /*
                        histogram_boundaries_.push_back(15.0);
                        histogram_boundaries_.push_back(20.0);
                        histogram_boundaries_.push_back(25.0);
                        histogram_boundaries_.push_back(30.0);
            */

            //ert::metrics::counter_ref_t c = cf.Add({{"signal", "SIGUSR1"}});
            count_sigusr1_ = &(cf.Add({{"signal", "SIGUSR1"}}));
            count_sigusr2_ = &(cf.Add({{"signal", "SIGUSR2"}}));
            gauge_ = &(gf.Add({{"unit", "seconds"}}));
            histogram_ = &(hf.Add({{"unit", "seconds"}}, histogram_boundaries_));
        }
        catch(std::exception &e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    void rcvSigusr1() {
        count_sigusr1_->Increment();
    }
    void rcvSigusr2() {
        count_sigusr2_->Increment();
    }
    void setGauge(double value) {
        gauge_->Set(value);
    }
    void updateHistogram(int value) {
        histogram_->Observe(value);
    }
};

void sighndl(int signal)
{
    LOGWARNING(ert::tracing::Logger::warning(ert::tracing::Logger::asString("Signal received: %d", signal), ERT_FILE_LOCATION));
    switch (signal) {
    case SIGTERM:
    case SIGINT:
        exit(1);
        break;
    case SIGUSR1:
        G_observer->rcvSigusr1();
        break;
    case SIGUSR2:
        G_observer->rcvSigusr2();
        break;
    }
}

int main(int argc, char* argv[]) {

    progname = basename(argv[0]);
    ert::tracing::Logger::initialize(progname);
    ert::tracing::Logger::verbose();

    // Capture TERM/INT signals for graceful exit:
    signal(SIGTERM, sighndl);
    signal(SIGINT, sighndl);

    // Capture USR1/USR2 signals to update metrics:
    signal(SIGUSR1, sighndl);
    signal(SIGUSR2, sighndl);

    ert::metrics::Metrics metrics;

    if (!metrics.serve()) exit(1);

    Observer observer(&metrics);
    G_observer = &observer;

    try {
        ert::metrics::counter_family_ref_t cf = metrics.addCounterFamily("main_example_total", "Main example counter total");
        ert::metrics::counter_ref_t c = cf.Add({});

        c.Increment(5);
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    int count = 0;
    const std::array<int, 4> timeResponses = {15, 20, 25, 30}; // range = 30-15 = 15; classes = 4; wide = 15/4 ~ 4;
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        const auto random_value = std::rand();
        auto timeResponse = timeResponses.at(random_value % timeResponses.size());

        observer.setGauge(count);
        observer.updateHistogram(timeResponse);
        std::cout << "." << std::flush;
        count++;
        if (count == 60) count = 0;
    }
}

