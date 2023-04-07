find_package(benchmark)

if (benchmark_FOUND)
    function(add_benchmark target_name entry_point)
        add_executable(${target_name} ${entry_point})
        target_compile_options(${target_name} PRIVATE "-Wl,--no-as-needed,-lprofiler,--as-needed")
        target_link_libraries(${target_name} -pthread ${OpenCV_LIBS} benchmark::benchmark)
    endfunction(add_benchmark)

    add_benchmark(radon-cpu-bench cpu.cpp)
endif()