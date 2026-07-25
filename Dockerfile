# =============================================================================
# metrics multi-stage Dockerfile
# =============================================================================
# Stages:
#   deps  - Dependencies (logger + prometheus-cpp)
#   build - Library compilation and installation
#
# Usage:
#   docker build --target deps  -t metrics_builder .
#   docker build --target build -t metrics .
#   docker build -t metrics .  (default: build)
# =============================================================================

FROM ubuntu:24.04 AS deps
LABEL maintainer="testillano"
LABEL testillano.metrics_builder.description="Docker image with all dependencies to build ert_metrics library"

WORKDIR /code/build

# ---------------------------------------------------------------------------
# Dependency versions (single source of truth)
# ---------------------------------------------------------------------------
ARG make_procs=4
ARG build_type=Release
ARG ert_logger_ver=v1.1.1
ARG jupp0r_prometheuscpp_ver=v1.3.0
ARG civetweb_civetweb_ver=v1.16

# ---------------------------------------------------------------------------
# System packages
# ---------------------------------------------------------------------------
RUN apt-get update && apt-get install -y \
    wget tar \
    make cmake g++ \
    libssl-dev zlib1g-dev libcurl4-openssl-dev \
    doxygen graphviz \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# ===========================================================================
# ERT_LOGGER
# ===========================================================================
RUN set -x && \
    wget https://github.com/testillano/logger/archive/${ert_logger_ver}.tar.gz && \
    tar xvf ${ert_logger_ver}.tar.gz && cd logger-*/ && \
    cmake -DERT_LOGGER_BuildExamples=OFF -DCMAKE_BUILD_TYPE=${build_type} . && \
    make -j${make_procs} && make install && \
    cd .. && rm -rf * && \
    set +x

# ===========================================================================
# PROMETHEUS-CPP + CIVETWEB
# ===========================================================================
RUN set -x && \
    wget https://github.com/jupp0r/prometheus-cpp/archive/refs/tags/${jupp0r_prometheuscpp_ver}.tar.gz && \
    tar xvf ${jupp0r_prometheuscpp_ver}.tar.gz && cd prometheus-cpp*/3rdparty && \
    wget https://github.com/civetweb/civetweb/archive/refs/tags/${civetweb_civetweb_ver}.tar.gz && \
    tar xvf ${civetweb_civetweb_ver}.tar.gz && mv civetweb-*/* civetweb && cd .. && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=${build_type} -DENABLE_TESTING=OFF .. && \
    make -j${make_procs} && make install && \
    cd ../.. && rm -rf * && \
    set +x

# ---------------------------------------------------------------------------
# Builder entrypoint
# ---------------------------------------------------------------------------
COPY deps/build.sh /var/build.sh
RUN chmod a+x /var/build.sh

ENTRYPOINT ["/var/build.sh"]
CMD []

# =============================================================================
# Stage: build (compile and install ert_metrics)
# =============================================================================
FROM deps AS build

ARG make_procs=4
ARG build_type=Release

COPY . /code/build/metrics/
RUN set -x && \
    cd metrics && \
    cmake -DERT_METRICS_BuildExamples=OFF -DCMAKE_BUILD_TYPE=${build_type} . && \
    make -j${make_procs} && make install && \
    cd .. && rm -rf metrics && \
    set +x
