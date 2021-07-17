project(screen_capture_example_cpp)

if(WIN32)
	set(${PROJECT_NAME}_PLATFORM_LIBS Dwmapi)
	add_definitions(-DNOMINMAX)
elseif(APPLE)
    find_package(Threads REQUIRED)
    find_library(corefoundation_lib CoreFoundation REQUIRED)
    find_library(cocoa_lib Cocoa REQUIRED)
    find_library(coremedia_lib CoreMedia REQUIRED)
    find_library(avfoundation_lib AVFoundation REQUIRED)
    find_library(coregraphics_lib CoreGraphics REQUIRED)
    find_library(corevideo_lib CoreVideo REQUIRED)
   
	set(${PROJECT_NAME}_PLATFORM_LIBS
        ${CMAKE_THREAD_LIBS_INIT}
        ${corefoundation_lib}
        ${cocoa_lib}
        ${coremedia_lib}
        ${avfoundation_lib}
        ${coregraphics_lib}  
        ${corevideo_lib}
    ) 
else()
	find_package(X11 REQUIRED)
	if(!X11_XTest_FOUND)
 		message(FATAL_ERROR "X11 extensions are required, but not found!")
	endif()
	if(!X11_Xfixes_LIB)
 		message(FATAL_ERROR "X11 fixes extension is required, but not found!")
	endif()
	find_package(Threads REQUIRED)
	set(${PROJECT_NAME}_PLATFORM_LIBS
		${X11_LIBRARIES}
		${X11_Xfixes_LIB}
		${X11_XTest_LIB}
		${X11_Xinerama_LIB}
		${CMAKE_THREAD_LIBS_INIT}
	)
endif()

include_directories(
	../include 
) 

add_executable(${PROJECT_NAME}_static
	lodepng.cpp
	Screen_Capture_Example.cpp
)
target_link_libraries(${PROJECT_NAME}_static screen_capture_lite_static ${${PROJECT_NAME}_PLATFORM_LIBS}) 
add_test (NAME ${PROJECT_NAME}_static COMMAND ${PROJECT_NAME}_static)

add_executable(${PROJECT_NAME}_shared  
	lodepng.cpp
	Screen_Capture_Example.cpp
)

install (TARGETS ${PROJECT_NAME}_static ${PROJECT_NAME}_shared screen_capture_lite_shared
	RUNTIME DESTINATION Examples
) 
  
target_link_libraries(${PROJECT_NAME}_shared screen_capture_lite_shared ${${PROJECT_NAME}_PLATFORM_LIBS}) 
add_test (NAME ${PROJECT_NAME}_shared COMMAND ${PROJECT_NAME}_shared)