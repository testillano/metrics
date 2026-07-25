# C++ metrics library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Documentation](https://codedocs.xyz/testillano/metrics.svg)](https://codedocs.xyz/testillano/metrics/index.html)
[![Ask Me Anything !](https://img.shields.io/badge/Ask%20me-anything-1abc9c.svg)](https://github.com/testillano)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/testillano/metrics/graphs/commit-activity)
[![Main project workflow](https://github.com/testillano/metrics/actions/workflows/ci.yml/badge.svg)](https://github.com/testillano/metrics/actions/workflows/ci.yml)
[![Container](https://img.shields.io/badge/Container-ghcr.io-blue.svg)](https://github.com/testillano/metrics/pkgs/container/metrics)

This library is based on @jupp0r prometheus-cpp library (https://github.com/jupp0r/prometheus-cpp).
It offers a quick way to instantiate Prometheus metrics (counters, gauges, histograms) with a unified interface. Includes a `ProcessCollector` for automatic process-level metrics (CPU, memory, FDs, threads).

Used by [h2agent](https://github.com/testillano/h2agent) and [http2comm](https://github.com/testillano/http2comm) for metrics exposition.

## Build with Docker

Single multi-stage `Dockerfile` with all dependencies from `ubuntu:24.04`.

```bash
$ ./build.sh                              # build everything
$ ./build.sh --builder                    # deps stage only
$ DBUILD_XTRA_OPTS=--no-cache ./build.sh  # force rebuild
```

Or directly:

```bash
$ docker build -t metrics .
```

### Pulling pre-built images

```bash
$ docker pull ghcr.io/testillano/metrics:<tag>
```

### Overriding dependency versions

```bash
$ jupp0r_prometheuscpp_ver=v1.4.0 ./build.sh
```

## Build natively

```bash
$ cmake . && make -j$(nproc)
```

### Requirements

All dependencies are documented in the `Dockerfile` (ARG declarations + RUN steps).

### Install

```bash
$ sudo make install
```

### Documentation

```bash
$ make doc
```

## Integration

### CMake

#### FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(ert_metrics
  GIT_REPOSITORY https://github.com/testillano/metrics.git
  GIT_TAG vx.y.z)

FetchContent_GetProperties(ert_metrics)
if(NOT ert_metrics_POPULATED)
  FetchContent_Populate(ert_metrics)
  add_subdirectory(${ert_metrics_SOURCE_DIR} ${ert_metrics_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(foo PRIVATE ert_metrics::ert_metrics)
```

## Contributing

```bash
$ sources=$(find . -name "*.hpp" -o -name "*.cpp")
$ docker run -i --rm -v $PWD:/data frankwolf/astyle ${sources}
```
