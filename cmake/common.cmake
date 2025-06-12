function(target_set_common_properties target_name)
    # being a cross-platform target, we enforce standards conformance on MSVC
    target_compile_options(${target_name} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
    target_compile_options(${target_name} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

    # add a clean DEBUG preprocessor define if applicable
    target_compile_definitions(${target_name} PRIVATE $<$<CONFIG:Debug>:DEBUG>)
endfunction()