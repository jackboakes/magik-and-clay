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
	code/win32/win_core.cpp
	code/win32/win_core.h
	code/win32/win_input.h
	code/common/types.h
	code/common/utils.cpp
	code/common/utils.h
	code/common/ring_buffer.h
	code/game/game.cpp
	code/game/game.h
	code/renderer/renderer.cpp
	code/renderer/renderer.h
	code/renderer/backend/d3d11_backend.cpp
	code/renderer/backend/d3d11_backend.h
	code/input/input.cpp
	code/input/input.h
	code/camera/camera.cpp
	code/camera/camera.h
	third_party/HandmadeMath/HandmadeMath.h
	third_party/stb/stb_image.h
	third_party/stb/stb_truetype.h
	third_party/stb/stb_rect_pack.h
	)
	
	source_group("win32" FILES
		code/win32/win_main.cpp
		code/win32/win_core.h
		code/win32/win_core.cpp
		code/win32/win_input.h
	)
 
	source_group("common" FILES
		code/common/types.h
		code/common/utils.h
		code/common/utils.cpp
		code/common/ring_buffer.h
	)

	source_group("game" FILES
		code/game/game.cpp
		code/game/game.h
	)
 
	source_group("renderer" FILES
		code/renderer/renderer.h
		code/renderer/renderer.cpp
	)
 
	source_group("renderer/backend" FILES
		code/renderer/backend/d3d11_backend.h
		code/renderer/backend/d3d11_backend.cpp
	)

	source_group("input" FILES
		code/input/input.h
		code/input/input.cpp
		code/win32/win_input.h
	)

	source_group("camera" FILES
		code/camera/camera.h
		code/camera/camera.cpp
	)

	source_group("third_party" FILES
    third_party/HandmadeMath/HandmadeMath.h
    third_party/stb/stb_image.h
	third_party/stb/stb_truetype.h
	third_party/stb/stb_rect_pack.h
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
	# Move exe to root
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

endif()