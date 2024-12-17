cc_library(
    name = "hashmap",
    hdrs = ["hashmap.h"],
)

cc_test(
    name = "hashmap_test",
    srcs = ["hashmap_test.cc"],
    deps = [
        ":hashmap",
        "@googletest//:gtest_main",
    ]
)

cc_binary(
    name = "hashmap_bench",
    srcs = ["hashmap_bench.cc"],
    testonly = True,
    deps = [
        ":hashmap",
        "@google_benchmark//:benchmark_main",
        "@abseil-cpp//absl/hash",
    ]
)