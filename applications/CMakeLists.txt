
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    add_subdirectory(ffd)
    add_subdirectory(stlp)
else()
    #add_subdirectory(stlp/host)
endif()
