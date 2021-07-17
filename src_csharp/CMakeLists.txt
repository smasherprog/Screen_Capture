project(screen_capture_lite_csharp)
    
set(OUTPUT ${PROJECT_NAME}.dll)
set(CSPROJ ${PROJECT_NAME}.csproj)

execute_process(COMMAND dotnet --version
                  RESULT_VARIABLE result
                  OUTPUT_QUIET
                  ERROR_QUIET)
if(result) 
    message(WARNING "CMake failed: dotnet executable not found but by this build. You must install dotnet to generate csharp bindings")
else()
    message(STATUS "Found dotnet executable CSHARP bindings will be generated!")
    add_custom_command(OUTPUT ${OUTPUT} 
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:screen_capture_lite_shared> ${CMAKE_CURRENT_LIST_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:screen_capture_lite_shared> ${CMAKE_BINARY_DIR}/src_csharp
        COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_CURRENT_LIST_DIR}/${CSPROJ}
          ${CMAKE_BINARY_DIR}/src_csharp
        COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_CURRENT_LIST_DIR}/Screen_Capture_Lite.cs
          ${CMAKE_BINARY_DIR}/src_csharp
        COMMAND dotnet build --configuration ${CMAKE_BUILD_TYPE} ${CMAKE_BINARY_DIR}/src_csharp/${CSPROJ} -o ${CMAKE_BINARY_DIR}
        COMMENT "Building DOTNET assembly ${OUTPUT}"
      )

    add_custom_target(${PROJECT_NAME} ALL DEPENDS ${OUTPUT})
    add_dependencies(${PROJECT_NAME} screen_capture_lite_shared)

install (FILES ${CMAKE_BINARY_DIR}/${OUTPUT}
	DESTINATION bin
)
install (FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pdb
	DESTINATION bin
    OPTIONAL
)
endif()


 