cmake_minimum_required(VERSION 3.12...4.2)

project(game 
		VERSION 1.0.0
		DESCRIPTION "A game from scratch"
		LANGUAGES CXX)
			 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)


if(WIN32)

	add_executable(${PROJECT_NAME} WIN32 
	code/win32/win_main.cpp
	code/common/types.h
	code/renderer/renderer_d3d11.cpp
	code/renderer/renderer_d3d11.h
	third_party/HandmadeMath/HandmadeMath.h
	third_party/stb/stb_image.h)
	
	source_group("third_party" FILES
    third_party/HandmadeMath/HandmadeMath.h
    third_party/stb/stb_image.h
	)


	target_include_directories(${PROJECT_NAME} PRIVATE
	"${CMAKE_SOURCE_DIR}/code"
	"${CMAKE_SOURCE_DIR}/include"
	third_party/stb
	third_party/HandmadeMath
	)
	
	

	target_link_libraries(${PROJECT_NAME} PRIVATE
        d3d11.lib
        dxgi.lib
        d3dcompiler.lib
		winmm.lib
        dxguid.lib)
elseif(APPLE)
# 
elseif(UNIX)
# 
endif()




if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
	/permissive-)
    # Sets the Visual Studio startup project to our poject otherwise it will default to ALL_BUILD
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()