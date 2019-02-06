function(include_target target_name)
    list(SUBLIST ARGV 1 -1 directories)
    add_library(${target_name} INTERFACE IMPORTED)
    set_property(TARGET ${target_name}
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${directories}
    )
endfunction()
