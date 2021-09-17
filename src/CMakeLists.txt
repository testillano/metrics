add_library (${ERT_METRICS_TARGET_NAME} STATIC
        ${CMAKE_CURRENT_LIST_DIR}/Metrics.cpp
)

target_include_directories(${ERT_METRICS_TARGET_NAME}
  PUBLIC ${ERT_METRICS_INCLUDE_BUILD_DIR}
)

target_link_libraries(${ERT_METRICS_TARGET_NAME}
PRIVATE
-static
        prometheus-cpp-pull
        prometheus-cpp-core
        z
)

install(TARGETS ${ERT_METRICS_TARGET_NAME}
        ARCHIVE DESTINATION lib/ert)
