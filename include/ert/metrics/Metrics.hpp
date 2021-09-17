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
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <array>
#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>

#include <iostream>

namespace ert
{
namespace metrics
{

typedef prometheus::Family<prometheus::Counter> counter_family_t;
typedef counter_family_t& counter_family_ref_t;
typedef prometheus::Family<prometheus::Gauge> gauge_family_t;
typedef gauge_family_t& gauge_family_ref_t;
//typedef prometheus::Family<prometheus::Histogram> histogram_family_t;
//typedef histogram_family_t& histogram_family_ref_t;

typedef std::unordered_map<std::string, counter_family_ref_t> counter_family_map_t;


class Metrics {

    std::shared_ptr<prometheus::Registry> registry_;
    counter_family_map_t counter_families_;

public:

Metrics(std::shared_ptr<prometheus::Registry> registry) {
    registry_ = registry;


    auto cf = prometheus::BuildCounter().Name("http2_traffic_messages_total")
            .Help("Number of processed traffic messages");

    counter_families_.emplace("http2_traffic_messages_total", cf.Register(*registry_));


    //http2_admin_requests_total.Register(*registry_);
}

counter_family_ref_t getCounterFamily(const std::string &name) const {
    auto it = counter_families_.find(name);
    if (it != counter_families_.end())
        return it->second;
}
/*
    counter_family_ref_t &http2_traffic_messages_total = prometheus::BuildCounter()
            .Name("http2_traffic_messages_total")
            .Help("Number of processed traffic messages")
            .Register(*registry_);
    gauge_family_ref_t &http2_traffic_requests_processing_seconds = prometheus::BuildGauge()
            .Name("http2_traffic_requests_processing_seconds")
            .Help("Traffic requests processing time")
            .Register(*registry_);
    gauge_family_ref_t &http2_traffic_messages_size_bytes = prometheus::BuildGauge()
            .Name("http2_messahttp2_traffic_messages_size_bytesges_size_bytes")
            .Help("Size of observed traffic messages")
            .Register(*registry_);
            */
    //counter_family_ref_t &http2_admin_requests_total = prometheus::BuildCounter()
      //      .Name("http2_admin_requests_total")
        //    .Help("Number of Administrative Interface HTTP/2 requests");

    // // pseudo metrics for running binary:
    // counter_family_ref_t &process_build_info = prometheus::BuildCounter()
    //         .Name("process_build_info")
    //         .Help("Process build information")
    //         .Register(*registry_);
    // counter_family_ref_t &process_execution_info = prometheus::BuildCounter()
    //         .Name("process_execution_info")
    //         .Help("rocess execution information")
    //         .Register(*registry_);

/*
    // add a counter whose dimensional data is not known at compile time
    //prometheus::Counter& http2_admin_requests_total_provision = http2_admin_requests_total.Add({{"operation", "server-provision"}});
    //prometheus::Counter& http2_admin_requests_total_matching = http2_admin_requests_total.Add({{"operation", "server-matching"}});
    //prometheus::Counter& http2_admin_requests_total_data = http2_admin_requests_total.Add({{"operation", "server-data"}});
    //prometheus::Counter& http2_admin_requests_total_logging = http2_admin_requests_total.Add({{"operation", "logging"}});
    //prometheus::Counter& http2_admin_requests_total_unsupported = http2_admin_requests_total.Add({{"operation", "unsupported"}});

    // Add and remember dimensional data, incrementing those is very cheap
    prometheus::Counter& http2_traffic_messages_total_rx_post = http2_traffic_messages_total.Add({{"direction", "rx"}, {"method", "POST"}});
    prometheus::Counter& http2_traffic_messages_total_rx_get = http2_traffic_messages_total.Add({{"direction", "rx"}, {"method", "GET"}});
    prometheus::Counter& http2_traffic_messages_total_rx_put = http2_traffic_messages_total.Add({{"direction", "rx"}, {"method", "PUT"}});
    prometheus::Counter& http2_traffic_messages_total_rx_delete = http2_traffic_messages_total.Add({{"direction", "rx"}, {"method", "DELETE"}});
    prometheus::Counter& http2_traffic_messages_total_rx_head = http2_traffic_messages_total.Add({{"direction", "rx"}, {"method", "HEAD"}});

    prometheus::Counter& http2_traffic_messages_total_tx_post = http2_traffic_messages_total.Add({{"direction", "tx"}, {"method", "POST"}});
    prometheus::Counter& http2_traffic_messages_total_tx_get = http2_traffic_messages_total.Add({{"direction", "tx"}, {"method", "GET"}});
    prometheus::Counter& http2_traffic_messages_total_tx_put = http2_traffic_messages_total.Add({{"direction", "tx"}, {"method", "PUT"}});
    prometheus::Counter& http2_traffic_messages_total_tx_delete = http2_traffic_messages_total.Add({{"direction", "tx"}, {"method", "DELETE"}});
    prometheus::Counter& http2_traffic_messages_total_tx_head = http2_traffic_messages_total.Add({{"direction", "tx"}, {"method", "HEAD"}});

    prometheus::Gauge& http2_traffic_requests_processing_seconds_rx_post = http2_traffic_requests_processing_seconds.Add({{"direction", "rx"}, {"method", "POST"}});
    prometheus::Gauge& http2_traffic_requests_processing_seconds_rx_get = http2_traffic_requests_processing_seconds.Add({{"direction", "rx"}, {"method", "GET"}});
    prometheus::Gauge& http2_traffic_requests_processing_seconds_rx_put = http2_traffic_requests_processing_seconds.Add({{"direction", "rx"}, {"method", "PUT"}});
    prometheus::Gauge& http2_traffic_requests_processing_seconds_rx_delete = http2_traffic_requests_processing_seconds.Add({{"direction", "rx"}, {"method", "DELETE"}});
    prometheus::Gauge& http2_traffic_requests_processing_seconds_rx_head = http2_traffic_requests_processing_seconds.Add({{"direction", "rx"}, {"method", "HEAD"}});

    prometheus::Gauge& http2_traffic_requests_processing_seconds_tx_post = http2_traffic_requests_processing_seconds.Add({{"direction", "tx"}, {"method", "POST"}});
    prometheus::Gauge& http2_traffic_requests_processing_seconds_tx_get = http2_traffic_requests_processing_seconds.Add({{"direction", "tx"}, {"method", "GET"}});
    prometheus::Gauge& http2_traffic_requests_processing_seconds_tx_put = http2_traffic_requests_processing_seconds.Add({{"direction", "tx"}, {"method", "PUT"}});
    prometheus::Gauge& http2_traffic_requests_processing_seconds_tx_delete = http2_traffic_requests_processing_seconds.Add({{"direction", "tx"}, {"method", "DELETE"}});
    prometheus::Gauge& http2_traffic_requests_processing_seconds_tx_head = http2_traffic_requests_processing_seconds.Add({{"direction", "tx"}, {"method", "HEAD"}});

    prometheus::Gauge& http2_traffic_messages_size_bytes_rx_post = http2_traffic_messages_size_bytes.Add({{"direction", "rx"}, {"method", "POST"}});
    prometheus::Gauge& http2_traffic_messages_size_bytes_rx_get = http2_traffic_messages_size_bytes.Add({{"direction", "rx"}, {"method", "GET"}});
    prometheus::Gauge& http2_traffic_messages_size_bytes_rx_put = http2_traffic_messages_size_bytes.Add({{"direction", "rx"}, {"method", "PUT"}});
    prometheus::Gauge& http2_traffic_messages_size_bytes_rx_delete = http2_traffic_messages_size_bytes.Add({{"direction", "rx"}, {"method", "DELETE"}});
    prometheus::Gauge& http2_traffic_messages_size_bytes_rx_head = http2_traffic_messages_size_bytes.Add({{"direction", "rx"}, {"method", "HEAD"}});

    prometheus::Gauge& http2_traffic_messages_size_bytes_tx_post = http2_traffic_messages_size_bytes.Add({{"direction", "tx"}, {"method", "POST"}});
    prometheus::Gauge& http2_traffic_messages_size_bytes_tx_get = http2_traffic_messages_size_bytes.Add({{"direction", "tx"}, {"method", "GET"}});
    prometheus::Gauge& http2_traffic_messages_size_bytes_tx_put = http2_traffic_messages_size_bytes.Add({{"direction", "tx"}, {"method", "PUT"}});
    prometheus::Gauge& http2_traffic_messages_size_bytes_tx_delete = http2_traffic_messages_size_bytes.Add({{"direction", "tx"}, {"method", "DELETE"}});
    prometheus::Gauge& http2_traffic_messages_size_bytes_tx_head = http2_traffic_messages_size_bytes.Add({{"direction", "tx"}, {"method", "HEAD"}});
    */
};

}
}

