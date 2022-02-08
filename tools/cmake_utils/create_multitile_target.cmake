
## merge_binaries combines multiple xcore applications into one by extracting
## a tile elf and recombining it into another binary.
## This macro takes an output target name, a base target, a target
## containing a tile to merge, and the tile number to merge.
## The resulting output will be a target named _OUTPUT_TARGET_NAME, which
## contains the _BASE_TARGET application with tile _TILE_TO_MERGE replaced with
## the respective tile from _OTHER_TARGET.
macro(merge_binaries _OUTPUT_TARGET_NAME _BASE_TARGET _OTHER_TARGET _TILE_NUM_TO_MERGE)
    get_target_property(BASE_TILE_DIR ${_BASE_TARGET} BINARY_DIR)
    get_target_property(BASE_TILE_NAME ${_BASE_TARGET} NAME)
    get_target_property(OTHER_TILE_DIR ${_OTHER_TARGET} BINARY_DIR)
    get_target_property(OTHER_TILE_NAME ${_OTHER_TARGET} NAME)

    file(MAKE_DIRECTORY ${OTHER_TILE_DIR}/${OTHER_TILE_NAME}_split)
    add_custom_target(${_OUTPUT_TARGET_NAME}
        ALL
        COMMAND xobjdump --split --split-dir ${OTHER_TILE_DIR}/${OTHER_TILE_NAME}_split ${OTHER_TILE_DIR}/${OTHER_TILE_NAME}
        COMMAND xobjdump ${BASE_TILE_DIR}/${BASE_TILE_NAME} -r 0,1,${OTHER_TILE_DIR}/${OTHER_TILE_NAME}_split/image_n0c${_TILE_NUM_TO_MERGE}_2.elf
        COMMAND cp ${BASE_TILE_DIR}/${BASE_TILE_NAME} ${BASE_TILE_DIR}/${_OUTPUT_TARGET_NAME}
        DEPENDS
            ${_BASE_TARGET}
            ${_OTHER_TARGET}
        BYPRODUCTS
            ${OTHER_TILE_DIR}/${OTHER_TILE_NAME}_split
        WORKING_DIRECTORY
        COMMENT
            "Merging tile ${_TILE_NUM_TO_MERGE} of ${_OTHER_TARGET} into ${_BASE_TARGET} to create ${_OUTPUT_TARGET_NAME}"
        VERBATIM
    )
endmacro()
