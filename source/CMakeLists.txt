add_subdirectory(core)

if (NOT CHIP8_CPP_CORE_ONLY)
    add_subdirectory(app)
endif ()