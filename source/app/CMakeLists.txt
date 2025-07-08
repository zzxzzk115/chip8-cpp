set(TARGET_NAME chip8cpp-app)

# add CPM dependency
include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

# add imgui-ext
CPMAddPackage(
        URI "gh:zzxzzk115/imgui-ext#0.1.1"
        OPTIONS "IMGUIEXT_WITH_BACKEND_SDL2 ON" "IMGUIEXT_WITH_BACKEND_GL3 ON" "IMGUIEXT_BUILD_EXAMPLES OFF"
)

# set binary folder
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# add source files
file(GLOB_RECURSE SOURCES "src/**.cpp")
file(GLOB_RECURSE HEADERS "include/**.hpp")

# add executable target
add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${TARGET_NAME} PUBLIC chip8cpp ImGuiExt)

target_set_common_properties(${TARGET_NAME})

target_include_directories(
        ${TARGET_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${TARGET_NAME}-${PROJECT_VERSION}>
)

# copy SDL2 libraries after build
add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        $<TARGET_FILE_DIR:SDL2::SDL2>
        $<TARGET_FILE_DIR:${TARGET_NAME}>
)

# copy programs after build
add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/programs
        $<TARGET_FILE_DIR:${TARGET_NAME}>/programs
)
