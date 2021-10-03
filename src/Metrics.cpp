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

#include <ert/tracing/Logger.hpp>

#include <ert/metrics/Metrics.hpp>
#include <exception>
#include <iostream>

namespace ert
{
namespace metrics
{

bool Metrics::serve(const std::string & endpoint)
{

    try {
        exposer_ = new prometheus::Exposer({endpoint});
    }
    catch(...)
    {
        ert::tracing::Logger::error("Initialization error (prometheus interface)", ERT_FILE_LOCATION);
        return false;
    }

    exposer_->RegisterCollectable(registry_);

    return true;
}

counter_family_ref_t Metrics::addCounterFamily(const std::string &name, const std::string &help, const labels_t &labels)
{
    auto fit = counter_families_.find(name);
    if (fit != counter_families_.end())
    {
        ert::tracing::Logger::error("counter family already registered", ERT_FILE_LOCATION);
    }

    auto& cf = prometheus::BuildCounter().Name(name).Help(help).Labels(labels).Register(*registry_);
    counter_families_.emplace(name, cf);

    return cf;
}

gauge_family_ref_t Metrics::addGaugeFamily(const std::string &name, const std::string &help, const labels_t &labels)
{
    auto it = gauge_families_.find(name);
    if (it != gauge_families_.end())
    {
        ert::tracing::Logger::error("gauge family already registered", ERT_FILE_LOCATION);
    }

    auto& gf = prometheus::BuildGauge().Name(name).Help(help).Labels(labels).Register(*registry_);
    gauge_families_.emplace(name, gf);

    return gf;
}

histogram_family_ref_t Metrics::addHistogramFamily(const std::string &name, const std::string &help, const labels_t &labels)
{
    auto it = histogram_families_.find(name);
    if (it != histogram_families_.end())
    {
        ert::tracing::Logger::error("histogram family already registered", ERT_FILE_LOCATION);
    }

    auto& hf = prometheus::BuildHistogram().Name(name).Help(help).Labels(labels).Register(*registry_);
    histogram_families_.emplace(name, hf);

    return hf;
}

void Metrics::increaseCounter(const std::string &familyName, const labels_t &labels, double value)
{
    auto fit = counter_families_.find(familyName);
    if (fit == counter_families_.end())
    {
        ert::tracing::Logger::error("counter family not found", ERT_FILE_LOCATION);
    }

    try {
        (fit -> second).Add(labels).Increment(value); // negative values are ignored by prometheus-cpp
    }
    catch(std::exception &e) {
        ert::tracing::Logger::error(e.what(), ERT_FILE_LOCATION);
    }
}

void Metrics::setGauge(const std::string &familyName, const labels_t &labels, double value)
{
    auto it = gauge_families_.find(familyName);
    if (it == gauge_families_.end())
    {
        ert::tracing::Logger::error("gauge family not found", ERT_FILE_LOCATION);
    }

    try {
        (it -> second).Add(labels).Set(value);
    }
    catch(std::exception &e) {
        ert::tracing::Logger::error(e.what(), ERT_FILE_LOCATION);
    }
}

void Metrics::observeHistogram(const std::string &familyName, const labels_t &labels, double value, const bucket_boundaries_t & bucketBoundaries)
{
    auto it = histogram_families_.find(familyName);
    if (it == histogram_families_.end())
    {
        ert::tracing::Logger::error("histogram family not found", ERT_FILE_LOCATION);
    }

    try {
        (it -> second).Add(labels, bucketBoundaries).Observe(value);
    }
    catch(std::exception &e) {
        ert::tracing::Logger::error(e.what(), ERT_FILE_LOCATION);
    }
}


}
}

