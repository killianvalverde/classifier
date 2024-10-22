project(classifier_test)

include_directories(${PROJECT_SOURCE_DIR}/../../src)

set(GTEST_LIBRARIES gtest gtest_main)

set(CLASSIFIER_TEST_SOURCE_FILES
        program_test.cpp
)

add_executable(classifier_test
        main.cpp
        ${CLASSIFIER_TEST_SOURCE_FILES}
)

target_link_libraries(classifier_test classifier ${GTEST_LIBRARIES})
