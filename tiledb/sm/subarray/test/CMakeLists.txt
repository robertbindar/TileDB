find_package(Catch_EP REQUIRED)

add_executable(unit_range_manager EXCLUDE_FROM_ALL)
target_link_libraries(unit_range_manager
    PRIVATE range_manager
    PUBLIC Catch2::Catch2
)
target_sources(unit_range_manager PUBLIC main.cc unit_range_manager.cc)

add_test(
    NAME "unit_range_manager"
    COMMAND $<TARGET_FILE:unit_range_manager>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
