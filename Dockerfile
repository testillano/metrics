ARG base_tag=latest
FROM ghcr.io/testillano/metrics_builder:${base_tag}
MAINTAINER testillano

LABEL testillano.metrics.description="ert_metrics library image"

WORKDIR /code/build

ARG make_procs=4
ARG build_type=Release

# ert_metrics
COPY . /code/build/metrics/
RUN set -x && \
    cd metrics && cmake -DCMAKE_BUILD_TYPE=${build_type} . && make -j${make_procs} && make install && \
    cd .. && rm -rf metrics && \
    set +x
