
## merge_binaries combines multiple xcore applications into one by extracting
## a tile elf and recombining it into another binary.
## This macro takes an output target name, a base target, a target
## containing a tile to merge, and the tile number to merge.
## The resulting output will be a target named _OUTPUT_TARGET_NAME, which
## contains the _BASE_TARGET application with tile _TILE_TO_MERGE replaced with
## the respective tile from _OTHER_TARGET.
macro(merge_binaries _OUTPUT_TARGET_NAME _BASE_TARGET _OTHER_TARGET _TILE_NUM_TO_MERGE)
    get_target_property(BASE_TILE_DIR     ${_BASE_TARGET}  BINARY_DIR)
    get_target_property(BASE_TILE_NAME    ${_BASE_TARGET}  NAME)
    get_target_property(OTHER_TILE_NAME   ${_OTHER_TARGET} NAME)

    add_custom_target(${_OUTPUT_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OTHER_TILE_NAME}_split
        COMMAND xobjdump --split --split-dir ${OTHER_TILE_NAME}_split ${OTHER_TILE_NAME}.xe
        COMMAND xobjdump ${BASE_TILE_NAME}.xe -r 0,${_TILE_NUM_TO_MERGE},${OTHER_TILE_NAME}_split/image_n0c${_TILE_NUM_TO_MERGE}_2.elf
        COMMAND cp ${BASE_TILE_NAME}.xe ${_OUTPUT_TARGET_NAME}.xe
        DEPENDS
            ${_BASE_TARGET}
            ${_OTHER_TARGET}
        BYPRODUCTS
            ${OTHER_TILE_NAME}_split
        WORKING_DIRECTORY
            ${BASE_TILE_DIR}
        COMMENT
            "Merge tile ${_TILE_NUM_TO_MERGE} of ${_OTHER_TARGET}.xe into ${_BASE_TARGET}.xe to create ${_OUTPUT_TARGET_NAME}.xe"
        VERBATIM
    )
    set_target_properties(${_OUTPUT_TARGET_NAME} PROPERTIES BINARY_DIR ${BASE_TILE_DIR})
endmacro()

## Creates a run target for a provided binary
macro(create_run_target _EXECUTABLE_TARGET_NAME)
    add_custom_target(run_${_EXECUTABLE_TARGET_NAME}
      COMMAND xrun --xscope ${_EXECUTABLE_TARGET_NAME}.xe
      DEPENDS ${_EXECUTABLE_TARGET_NAME}
      COMMENT
        "Run application"
      VERBATIM
    )
endmacro()

## Creates a debug target for a provided binary
macro(create_debug_target _EXECUTABLE_NAME)
    add_custom_target(debug_${_EXECUTABLE_NAME}
      COMMAND xgdb ${_EXECUTABLE_NAME}.xe -ex 'connect' -ex 'connect --xscope' -ex 'run'
      DEPENDS ${_EXECUTABLE_NAME}.xe
      COMMENT
        "Debug application"
    )
endmacro()

## Creates a flash app target for a provided binary
macro(create_flash_app_target _EXECUTABLE_NAME)
    add_custom_target(flash_app_${_EXECUTABLE_NAME}
      COMMAND xflash --quad-spi-clock 50MHz ${_EXECUTABLE_NAME}.xe
      DEPENDS ${_EXECUTABLE_NAME}.xe
      COMMENT
        "Flash application"
    )
endmacro()
