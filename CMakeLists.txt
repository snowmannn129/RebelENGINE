add_executable(core_tests
    EventBusTest.cpp
)

target_link_libraries(core_tests
    PRIVATE
    RebelCADCore
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME core_tests COMMAND core_tests)
