include_directories(.)
file (GLOB_RECURSE SOURCES *.cpp)
add_executable (xor_deploy  ${SOURCES})

if (WIN32)
    # Windows does not support implicitly importing data members from DLL.
    target_link_libraries(xor_deploy megbrain megdnn ${MGE_CUDA_LIBS})
else()
    target_link_libraries (xor_deploy megengine)
endif()
install (TARGETS xor_deploy EXPORT ${MGE_EXPORT_TARGETS} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
