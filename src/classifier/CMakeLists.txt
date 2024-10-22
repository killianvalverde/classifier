set(CLASSIFIER_SOURCE_FILES
        exception.hpp
        json.hpp
        program.cpp
        program.hpp
        program_args.hpp
)

add_library(classifier STATIC ${CLASSIFIER_SOURCE_FILES})

set(suffix "$<IF:$<CONFIG:Debug>,d,>")
target_link_libraries(classifier speed${suffix})
