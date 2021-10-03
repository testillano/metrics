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

#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <memory>
#include <string>
#include <unordered_map>

//#include <exception>


namespace ert
{
namespace metrics
{

typedef std::map<std::string, std::string> labels_t;

typedef prometheus::Counter counter_t;
typedef counter_t& counter_ref_t;
typedef prometheus::Gauge gauge_t;
typedef gauge_t& gauge_ref_t;
typedef prometheus::Histogram histogram_t;
typedef histogram_t& histogram_ref_t;
typedef std::vector<double> bucket_boundaries_t;

typedef prometheus::Family<prometheus::Counter> counter_family_t;
typedef counter_family_t& counter_family_ref_t;
typedef prometheus::Family<prometheus::Gauge> gauge_family_t;
typedef gauge_family_t& gauge_family_ref_t;
typedef prometheus::Family<prometheus::Histogram> histogram_family_t;
typedef histogram_family_t& histogram_family_ref_t;

typedef std::unordered_map<std::string, counter_family_ref_t> counter_family_map_t;
typedef std::unordered_map<std::string, gauge_family_ref_t> gauge_family_map_t;
typedef std::unordered_map<std::string, histogram_family_ref_t> histogram_family_map_t;

class Metrics {

    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Exposer *exposer_;

    counter_family_map_t counter_families_;
    gauge_family_map_t gauge_families_;
    histogram_family_map_t histogram_families_;

public:

    Metrics() {
        registry_ = std::make_shared<prometheus::Registry>();
        exposer_ = nullptr;
    }

    bool serve(const std::string & endpoint = "0.0.0.0:8080");

    // Statically counter creation:
    // counter_family_ref_t cf = addCounterFamily(...)
    // counter_ref_t counter = cf.Add(<labels>);
    counter_family_ref_t addCounterFamily(const std::string &name, const std::string &help, const labels_t &labels = {});
    gauge_family_ref_t addGaugeFamily(const std::string &name, const std::string &help, const labels_t &labels = {});
    histogram_family_ref_t addHistogramFamily(const std::string &name, const std::string &help, const labels_t &labels = {});


    // dynamically calling Family<T>.Add() works but is slow and should be avoided:
    void increaseCounter(const std::string &familyName, const labels_t &labels, double value = 1.0);
    void setGauge(const std::string &familyName, const labels_t &labels, double value);
    void observeHistogram(const std::string &familyName, const labels_t &labels, double value, const bucket_boundaries_t & bucketBoundaries);
};

}
}

