set(TARGET_NAME chip8cpp)

# add source files
file(GLOB_RECURSE SOURCES "src/**.cpp")
file(GLOB_RECURSE HEADERS "include/**.hpp")

# add library target
add_library(${TARGET_NAME} STATIC ${SOURCES} ${HEADERS})

target_set_common_properties(${TARGET_NAME})

target_include_directories(
        ${TARGET_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${TARGET_NAME}-${PROJECT_VERSION}>
)