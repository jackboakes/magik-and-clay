cmake_minimum_required(VERSION 3.12...4.2)

project(game 
		VERSION 1.0.0
		DESCRIPTION "A game from scratch"
		LANGUAGES CXX)
			 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)


if(WIN32)
	add_executable(${PROJECT_NAME} WIN32 src/win32/win_main.cpp)
endif()


if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
	/permissive-)
    # Sets the Visual Studio startup project to our poject otherwise it will default to ALL_BUILD
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()