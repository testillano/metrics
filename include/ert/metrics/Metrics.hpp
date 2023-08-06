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

/**
 * Labels: key-value pairs map to enable Prometheus's dimensional data model.
 * Any combination of labels for the same metric identifies a particular dimensional instance of the metric.
 * For example: all HTTP requests that used the method POST to the uri '/the/uri': {"method", "POST"}, {"uri", "/the/uri"}
 */
typedef std::map<std::string, std::string> labels_t;

/** Prometheus counter type: cumulative metric that represents a single monotonically increasing counter (i.e.: requests sent, errors, etc.). */
typedef prometheus::Counter counter_t;

/** Prometheus gauge type: single numerical value that can go up and down arbitrarily (i.e.: temperatures, memory usage, concurrent requests, etc.). */
typedef prometheus::Gauge gauge_t;

/**
 * Prometheus histogram type: samples observations within buckets of specific ranges (i.e.: requests durations, message sizes, etc.).
 * Scrape exposes multiple time series:
 * > cumulative counters (*_bucket{le="<upper inclusive bound>"})
 * > total sum of observed values (*_sum)
 * > count of events observed (*_count). Equals to *_bucket{le="+Inf"}.
 */
typedef prometheus::Histogram histogram_t;

/** Bucket boundaries for histogram */
typedef std::vector<double> bucket_boundaries_t;

/** Counters family */
typedef prometheus::Family<prometheus::Counter> counter_family_t;

/** Gauges family */
typedef prometheus::Family<prometheus::Gauge> gauge_family_t;

/** Histograms family */
typedef prometheus::Family<prometheus::Histogram> histogram_family_t;

/** Map of counters family (indexed by counter name) */
typedef std::unordered_map<std::string, counter_family_t&> counter_family_map_t;

/** Map of gauges family (indexed by gauge name) */
typedef std::unordered_map<std::string, gauge_family_t&> gauge_family_map_t;

/** Map of histograms family (indexed by histogram name) */
typedef std::unordered_map<std::string, histogram_family_t&> histogram_family_map_t;

class Metrics {

    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Exposer *exposer_;

    counter_family_map_t counter_families_;
    gauge_family_map_t gauge_families_;
    histogram_family_map_t histogram_families_;

public:

    /** Default constructor */
    Metrics() {
        registry_ = std::make_shared<prometheus::Registry>();
        exposer_ = nullptr;
    }

    /** Default destructor */
    ~Metrics() {
        delete(exposer_);
    }

    /**
     * Serves metrics exposer
     *
     * @param endpoint Scrape endpoint, '0.0.0.0:8080' by default.
     */
    bool serve(const std::string & endpoint = "0.0.0.0:8080");

    /**
     * Add counter family
     *
     * Example:
     *
     * <pre>
     * // Counter family with initial labels:
     * ert::metrics::counter_family_t& observed_requests_counter_family_ref_ = metrics->addCounterFamily("admin_server_observed_requests_total", "Http2 total requests observed in admin server", {"type", "request"});
     * </pre>
     *
     * Or you may prefer to use pointers (this allows to build the family on constructor, where you could have more data available like the family name):
     *
     * <pre>
     * // class members
     * ert::metrics::counter_family_t* observed_requests_counter_family_ptr_{};
     * ...
     * // constructor
     * observed_requests_counter_family_ptr_ = &(metrics->addCounterFamily("admin_server_observed_requests_total", "Http2 total requests observed in admin server", {"type", "request"}));
     * </pre>
     *
     * Now you can create a counter (again, using pointers):
     *
     * <pre>
     * // class members
     * counter_t* observed_requests_post_counter_ptr_{};
     * ...
     * // constructor
     * observed_requests_post_counter_ptr_ = &(observed_requests_counter_family_ptr_->Add({{"method", "POST"}}));
     * </pre>
     *
     * And use it:
     *
     * <pre>
     * // source code
     * observed_requests_post_counter_ptr_->Increment(); // no cost for already added labels ('Family<T>.Add()' is not called anymore).
     * </pre>
     *
     * So, counters should be created only once (i.e.: constructors), and pointers should be used to allow accessing them.
     * Take this source as example: https://github.com/testillano/http2comm/blob/6ff2a1706d584db057ab22c55da72775919e9320/src/Http2Server.cpp#L65
     *
     * To create dynamic counters (with specific labels), you will need to call Add():
     *
     * <pre>
     * auto& counter = observed_requests_counter_family_ptr_->Add({{"status_code", std::to_string(status_code)}}); // dynamic labels are added to family ones ("type" in the example)
     * counter.Increment();
     * </pre>
     *
     * Here you can find a smart counter class, which optimizes the combination of static and dynamic labels:
     *
     * <pre>
     * class MyCounter {
     * public:
     *     MyCounter(counter_family_t& family, const std::map<std::string, std::string>& staticLabels)
     *         : family_(family), all_labels_(staticLabels) {}
     *
     *     void Increment(const std::map<std::string, std::string>& dynamicLabels) {
     *         all_labels_.insert(dynamicLabels.begin(), dynamicLabels.end()); // join labels
     *         auto& counter = family_.Add(all_labels_);
     *         counter.Increment();
     *     }
     *
     * private:
     *     counter_family_t& family_;
     *     std::map<std::string, std::string> all_labels_;
     * };
     * </pre>
     *
     * Note that dynamic labels should override all the possible labels added in the past with corresponding values,
     * in order to keep coherence. It is good to add a fixed set of dynamic labels (i.e. the status code).
     * So, you can instantiate a counter with initial labels, and then increment it with additional 'dynamic' ones:
     *
     * <pre>
     * std::map<std::string, std::string> staticLabels = {{"method", "post"}};
     * MyCounter myCounter(observed_requests_counter_family_ref_, staticLabels);
     *
     * // source code:
     * std::map<std::string, std::string> dynamicLabels = {{"status_code", std::to_string(status_code)}};
     * myCounter.Increment(dynamicLabels);
     * </pre>
     *
     * @param name Family name
     * @param help Family help description
     * @param labels Family definition labels. Empty by default (a counter generated within this family could add additional labels).
     */
    counter_family_t& addCounterFamily(const std::string &name, const std::string &help, const labels_t &labels = {});

    /**
     * Add gauge family
     *
     * Same than commented above for counter family applies here: gauge pointers over pre-created family is the best approach
     * from the performance point of view.
     *
     * @param name Family name
     * @param help Family help description
     * @param labels Family definition labels (a gauge generated within this family could even add additional labels)
     *
     * @see addCounterFamily()
     */
    gauge_family_t& addGaugeFamily(const std::string &name, const std::string &help, const labels_t &labels = {});

    /**
     * Add histogram family
     *
     * Same than commented above for counter family applies here: histogram pointers over pre-created family is the best approach
     * from the performance point of view.
     *
     * @param name Family name
     * @param help Family help description
     * @param labels Family definition labels (an histogram generated within this family could even add additional labels)
     *
     * @see addCounterFamily()
     */
    histogram_family_t& addHistogramFamily(const std::string &name, const std::string &help, const labels_t &labels = {});

    /**
     * Increase counter
     *
     * Dynamic labels could be added, but take into account the performance impact doing this. It is better to create all the combinations at the beginning.
     * This must be used only with unpredictable labels, which will be appended to the labels that were used to create the family (static initial labels).
     *
     * @param name Family name
     * @param labels Additional labels
     * @param value Increase amount
     */
    void increaseCounter(const std::string &familyName, const labels_t &labels, double value = 1.0);

    /**
     * Update gauge
     *
     * Dynamic labels could be added, but take into account the performance impact doing this. It is better to create all the combinations at the beginning.
     * This must be used only with unpredictable labels, which will be appended to the labels that were used to create the family (static initial labels).
     *
     * @param name Family name
     * @param labels Additional labels
     * @param value Instant current value
     */
    void setGauge(const std::string &familyName, const labels_t &labels, double value);

    /**
     * Observe histogram
     *
     * Dynamic labels could be added, but take into account the performance impact doing this. It is better to create all the combinations at the beginning.
     * This must be used only with unpredictable labels, which will be appended to the labels that were used to create the family (static initial labels).
     *
     * @param name Family name
     * @param labels Additional labels
     * @param value Observed value
     * @param bucketBoundaries Reference to the bucket boundaries used
     */
    void observeHistogram(const std::string &familyName, const labels_t &labels, double value, const bucket_boundaries_t & bucketBoundaries);
};

}
}

