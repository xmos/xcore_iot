cmake_minimum_required(VERSION 3.14)

macro(create_multitile_target)
    # set(TILE_LIST 0 1 2 3)
    set(TILE_LIST 0 1)
    ## TODO: replace with generator expression to only build targets we need
    foreach(TILE IN ITEMS ${TILE_LIST})
    	message(STATUS "Create target for tile ${TILE}")

        message(STATUS "${CMAKE_BINARY_DIR}")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/intermediates")

        set(THIS_TARGET ${TILE}.xe)
        add_executable(${THIS_TARGET})

        target_sources(${THIS_TARGET} PRIVATE ${APP_SOURCES})
        target_include_directories(${THIS_TARGET} PRIVATE ${APP_INCLUDES})

        target_compile_options(${THIS_TARGET} PRIVATE ${APP_COMPILER_FLAGS})
        target_compile_definitions(${THIS_TARGET} PRIVATE THIS_XCORE_TILE=${TILE})
        target_link_options(${THIS_TARGET} PRIVATE ${APP_COMPILER_FLAGS})

        add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/intermediates/${TILE}/image_n0c${TILE}.elf"
               COMMAND mkdir ${CMAKE_BINARY_DIR}/intermediates/${TILE}
           	   COMMAND xobjdump --split --split-dir ${CMAKE_BINARY_DIR}/intermediates/${TILE}  ${CMAKE_BINARY_DIR}/intermediates/${THIS_TARGET}

               BYPRODUCTS
                   "${CMAKE_BINARY_DIR}/intermediates/${TILE}/config.xml"
                   "${CMAKE_BINARY_DIR}/intermediates/${TILE}/image_n0c0.elf"
                   "${CMAKE_BINARY_DIR}/intermediates/${TILE}/image_n0c0_2.elf"
                   "${CMAKE_BINARY_DIR}/intermediates/${TILE}/image_n0c1.elf"
                   "${CMAKE_BINARY_DIR}/intermediates/${TILE}/image_n0c1_2.elf"
                   "${CMAKE_BINARY_DIR}/intermediates/${TILE}/platform_def.xn"
                   "${CMAKE_BINARY_DIR}/intermediates/${TILE}/program_info.txt"
                   "${CMAKE_BINARY_DIR}/intermediates/${TILE}/xscope.xscope"
               DEPENDS
                   "${CMAKE_BINARY_DIR}/intermediates/${THIS_TARGET}"
               VERBATIM)

    endforeach()

    add_custom_target(${PROJECT_NAME}
                      ALL
                        xobjdump ${CMAKE_BINARY_DIR}/intermediates/0.xe -r 0,1,${CMAKE_BINARY_DIR}/intermediates/1/image_n0c1_2.elf
                      COMMAND
                        mv ${CMAKE_BINARY_DIR}/intermediates/0.xe ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xe
                      DEPENDS
                         "intermediates/0.xe"
                         "intermediates/1/image_n0c1.elf"

                      COMMENT "Merge tile binaries"
                  VERBATIM)
endmacro()

# add_custom_target

# this should use another cmakelists.txt to create the tile executables
# add_custom_command(OUTPUT ${TILE0_TEMP_DIR/${EXECUTABLE}

# add_custom_command(OUTPUT something.txt
#     COMMAND touch something.txt
#
#     # Display the given message before the commands are executed at build time
#     COMMENT "Creating something.txt"
# 	# deleteme debug
# 	COMMAND echo end of add_custom_command output something.txt
# )

# add_custom_command()
#
#
# OUTPUT: MAIN_DEPENDENCY DEPENDS
#         COMMAND

# TODO: this should be one iteration and able to loop through n tiles
# add_custom_target(ctgt_split ALL
# 	# Run this entire custom target when the file below doesn't yet exist:
# 	# DEPENDS ${TILE0_TEMP_DIR}/${EXECUTABLE}
#
# 	# deleteme debug:
# 	COMMAND echo start of add_custom_target ctgt_split
# 	# deleteme debug:
# 	DEPENDS something.txt
#
# 	COMMENT "Executing xobjdump on tile executables"
#
# 	# TODO: add directories to a list, then iterate through the list for the commands
# 	# in the loop
#
# 	foreach(X IN ITEMS ${TILE_LIST})
# 		test()
# 	endforeach()
#
# 	test()
#
# 	message(STATUS "Help! Help! I'm being repressed!")
#
#
# 	# split tile0
# 	COMMAND mkdir -p ${TILE0_TEMP_DIR}
# 	COMMAND echo call xobjdump on tile0 executable, dump files in temp dir
# 	# syntax: xobjdump --split --split-dir <dir_to_dump_files> <EXECUTABLE>
# 	COMMAND echo TILE0_SRC_DIR is ${TILE0_SRC_DIR}
# 	COMMAND xobjdump --split --split-dir ${TILE0_TEMP_DIR} ${TILE0_SRC_DIR}/${EXECUTABLE}
#
# 	# do the same for tile1 TODO: iterate
#     COMMAND mkdir -p ${TILE1_TEMP_DIR}
# 	# call xobjdump on tile1 executable, dump files in temp dir
# 	COMMAND xobjdump --split --split-dir ${TILE1_TEMP_DIR} ${TILE1_SRC_DIR}/${EXECUTABLE}
#
# 	# TODO: iterate
# 	# identifies all files to be cleaned
# 	BYPRODUCTS
# 		# tile0
# 		${TILE0_TEMP_DIR}/config.xml
#         ${TILE0_TEMP_DIR}/image_n0c0.elf
#         ${TILE0_TEMP_DIR}/image_n0c0_2.elf
#         ${TILE0_TEMP_DIR}/image_n0c1.elf
#         ${TILE0_TEMP_DIR}/image_n0c1_2.elf
#         ${TILE0_TEMP_DIR}/platform_def.xn
#         ${TILE0_TEMP_DIR}/program_info.txt
#         ${TILE0_TEMP_DIR}/xscope.xscope
# 		# tile1
# 		${TILE1_TEMP_DIR}/config.xml
#         ${TILE1_TEMP_DIR}/image_n0c0.elf
#         ${TILE1_TEMP_DIR}/image_n0c0_2.elf
#         ${TILE1_TEMP_DIR}/image_n0c1.elf
#         ${TILE1_TEMP_DIR}/image_n0c1_2.elf
#         ${TILE1_TEMP_DIR}/platform_def.xn
#         ${TILE1_TEMP_DIR}/program_info.txt
#         ${TILE1_TEMP_DIR}/xscope.xscope
# 		## need to add generator expression probably to support 3+ tiles
# )
#
# add_custom_target(ctgt_merge
# 	# replace ELF files TODO: iterate
# 	COMMAND echo replacing ELFs
# 	COMMAND xobjdump ${CMAKE_SOURCE_DIR}/../bin/${EXECUTABLE} -r 0,0,${TILE0_TEMP_DIR}/image_n0c0_2.elf;
# 	COMMAND xobjdump ${CMAKE_SOURCE_DIR}/../bin/${EXECUTABLE} -r 0,1,${TILE1_TEMP_DIR}/image_n0c1_2.elf;
#
# 	# This gives access to CMAKE commands, which are different from cmd line cmds
# 	# COMMAND
#     #     ${CMAKE_COMMAND} -E echo this might work for something later
# 	# VERBATIM
# )
#
# function(test)
# 	message(STATUS "Hello from inside test()")
# endfunction()
#
# test()
#
# # # pseudocode . . . pseu pseu pseudio
# # add_custom_command(OUTPUT ${ a generator expression for this tile number}/image_n0c0.elf
#    # X DEPENDS ## a built target executable
#    # X COMMAND ${CMAKE_COMMAND} -E split FOO2.out
#    # X BYPRODUCTS
#
#    # COMMENT "Splitting executable file")
# # )
#
# # add_custom_target(#real targetfile
#     # ALL
#         # ${CMAKE_COMMAND} -E xobjdump #real targetfile -r 0, #the real binary executable ,tile1/image_n0c1.elf;
#         # DEPENDS tile1/image_n0c1.elf
#    # COMMENT "Merging tile 1 prog into tile 0")
#
#
#
# # ## deleteme Makefile code to translate:
# # # Splits up a tile's XE file into its individual ELF files.
# # SPLIT_XE = mkdir -p $(BUILD_DIR)/tmp/tile$(1); xobjdump --split --split-dir $(BUILD_DIR)/tmp/tile$(1) $(call TILE_EXECUTABLE,$(1));
#
# # # Replaces the specified tile's ELF file with the correct one from the tile specific build.
# # REPLACE_ELF = xobjdump $(EXECUTABLE) -r 0,$(1),$(BUILD_DIR)/tmp/tile$(1)/image_n0c$(1)_2.elf;
#
# # MERGE_XE = $(call SPLIT_XE,$(1)) $(call REPLACE_ELF,$(1))
#
# # $(EXECUTABLE): $(TILE_DEPS)
#     # cp $(call TILE_EXECUTABLE,$(XE_BASE_TILE)) $(EXECUTABLE)
#     # $(call PER_MERGE_TILE,MERGE_XE)
#
#
# # # ## deleteme example code
# # set(TEST_FILE "log.txt")
#
# # # add_custom_command does not create a new target. You have to define targets explicitly
# # # by add_executable, add_library or add_custom_target in order to make them visible to make
# # add_custom_command(OUTPUT ${TEST_FILE}
#     # COMMAND touch ${TEST_FILE}
#
#     # # Display the given message before the commands are executed at build time
#     # COMMENT "Creating ${TEST_FILE}"
# # )
#
# # # target zoo is always built
# # add_custom_target(zoo ALL
#     # COMMAND echo "This is ALL target 'zoo', and it depends on ${TEST_FILE}"
#     # # If the file exists, then commands related to that file won't be executed
#     # # DONOT let other target depends on the same OUTPUT as current target,
#     # #   or it may be bad when doing parallel make
#     # DEPENDS ${TEST_FILE}
#
#     # # to make quotes printable,for example
#     # VERBATIM
# # )
#
# # # target bar is only build when `make bar` is issued
# # add_custom_target(bar
#     # # cmake -E support copy/env/echo and so on. use cmake -E to see
#     # # COMMAND/COMMENT must be upper case
#     # COMMAND ${CMAKE_COMMAND} -E echo bar:hello
#     # #COMMAND ${CMAKE_COMMAND} -E environment
#
#     # COMMENT "testing add_custom_target 'bar'..."
#     # WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#
#     # #DEPENDS zoo
# # )
# # # It seems to be same as `DEPENDS zoo` above
# # add_dependencies(bar zoo)
#
# # # This is the second signature of add_custom_command, which adds a custom command to a target such as a library or executable. This is useful for performing an operation before or after building the target. The command becomes part of the target and will only execute when the target itself is built. If the target is already built, the command will not execute
# # add_custom_command(TARGET bar
#     # # On Visual Studio Generators, run before any other rules are executed within the target. On other generators, run just before PRE_LINK commands
#     # PRE_BUILD
#     # COMMAND echo -e "\texecuting a PRE_BUILD command"
#     # COMMENT "This command will be executed before building bar"
#     # VERBATIM # to support \t for example
# # )
#
# # add_custom_command(TARGET bar
#     # # Run after sources have been compiled but before linking the binary or running the librarian or archiver tool of a static library. This is not defined for targets created by the add_custom_target() command
#     # PRE_LINK
#     # COMMAND echo -e "\texecuting a PRE_LINK command"
#     # COMMENT "This command will be executed after building bar"
#     # VERBATIM
# # )
#
# # add_custom_command(TARGET bar
#     # # Run after all other rules within the target have been executed
#     # POST_BUILD
#     # COMMAND echo -e "\texecuting a POST_BUILD command"
#     # COMMENT "This command will be executed after building bar"
#     # VERBATIM
# # )
