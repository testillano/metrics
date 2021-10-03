add_executable (measure main.cpp)
add_library(ert_logger STATIC IMPORTED)
set_property(TARGET ert_logger PROPERTY IMPORTED_LOCATION /usr/local/lib/ert/libert_logger.a)
target_link_libraries(measure ${ERT_METRICS_TARGET_NAME} ert_logger)
