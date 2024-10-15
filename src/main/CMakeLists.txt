set(MAIN_SOURCE_FILES
        main.cpp
)

add_executable(main ${MAIN_SOURCE_FILES})
target_link_libraries(main classifier)
install(TARGETS main DESTINATION bin)
set_target_properties(main PROPERTIES OUTPUT_NAME classifier)
