cmake_minimum_required(VERSION 3.0.2)
project(BabelTrader)

# set compile parameter
if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wno-missing-field-initializers")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE -DUNICODE)
	# add_definitions(-DNOMINMAX) # for fix std::max conflict in windows
endif()

# set output dir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# set use folder in vs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# add custom cmake modules
INCLUDE(FindPackageHandleStandardArgs)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

# dependencies
find_package(Threads)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
# use libuv and uWebSockets with source code
# find_package(LibUV REQUIRED)
# find_package(uWS REQUIRED)

# libuv
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/libuv)
set_target_properties(uv uv_a
	PROPERTIES FOLDER "thirdparty/libuv"
)

# uWebSockets
file(GLOB UWS_H ${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/src/*.h)
file(GLOB UWS_CPP ${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/src/*.cpp)
if (WIN32)
	add_library(uWS SHARED
		${UWS_H}
		${UWS_CPP}
	)
else()
	add_library(uWS SHARED
		${UWS_CPP}
	)
endif(WIN32)
add_dependencies(uWS uv)
target_link_libraries(uWS
	${OPENSSL_LIBRARIES}
	${ZLIB_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	uv
)
target_include_directories(uWS PUBLIC
	${OPENSSL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/libuv/include
)
if (NOT WIN32)
	target_compile_definitions(uWS PRIVATE UWS_THREADSAFE)
endif()
set_target_properties(uWS
	PROPERTIES FOLDER "thirdparty/uWs"
)
add_custom_command(
	TARGET uWS POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/thirdparty/uWS
)
foreach(uWS_Header ${UWS_H})
	get_filename_component(uWS_Header_name ${uWS_Header} NAME)
	add_custom_command(
		TARGET uWS
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${uWS_Header} ${CMAKE_BINARY_DIR}/thirdparty/uWS/include/uWS/${uWS_Header_name}
	)
endforeach(uWS_Header ${UWS_H})

set_target_properties(uWS PROPERTIES PUBLIC_HEADER "${UWS_H}")
install(
    TARGETS uWS
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
    PUBLIC_HEADER DESTINATION include/uWS
)

# glog
set(WITH_GFLAGS OFF CACHE BOOL "glog with gflags") 
set(BUILD_TESTING OFF CACHE BOOL "glog build testting") 
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/glog)
set_target_properties(glog
	PROPERTIES FOLDER "thirdparty/google"
)

# muggleCC
set(MUGGLE_BUILD_ONLY_C OFF)
set(MUGGLE_BUILD_EXAMPLE OFF)
set(MUGGLE_BUILD_TEST OFF)
set(MUGGLE_BUILD_BENCHMARK OFF)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/muggleCC)
set_target_properties(muggleCC
	PROPERTIES
	FOLDER "thirdparty/muggle"
)

# dependencies
set(dependencies_inc
	${OPENSSL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/libuv/include
	${CMAKE_BINARY_DIR}/thirdparty/uWS/include/
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/rapidjson/include
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/muggleCC
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/glog/src
	${CMAKE_BINARY_DIR}/thirdparty/glog/
	${CMAKE_CURRENT_LIST_DIR}/src
)

# common
file(GLOB common_h ${CMAKE_CURRENT_LIST_DIR}/src/common/*.h)
file(GLOB common_cpp ${CMAKE_CURRENT_LIST_DIR}/src/common/*.cpp)
if (WIN32)
	add_library(BabelTrader-Common STATIC
		${common_h}
		${common_cpp}
	)
else()
	add_library(BabelTrader-Common STATIC
		${common_cpp}
	)
endif()
target_include_directories(BabelTrader-Common PRIVATE ${dependencies_inc})
target_link_libraries(BabelTrader-Common uWS glog)


# utils function
function(add_serv name folder)
	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_cpp})
	else()
		add_executable(${name} ${tmp_cpp})
	endif(WIN32)
	add_dependencies(${name} uWS glog)
	target_include_directories(${name} PRIVATE ${dependencies_inc})
	target_link_libraries(${name} uWS glog BabelTrader-Common)
endfunction(add_serv)

# include and link directories
if (WIN32)
	link_directories(
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/ctp/lib/win64
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/xtp/lib/win64
	)
else()
	link_directories(
		/usr/local/lib
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/ctp/lib/linux
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/xtp/lib/linux
	)
endif()

option(BABELTRADER_ENABLE_PERFORMANCE "enable performance" OFF)
if (${BABELTRADER_ENABLE_PERFORMANCE})
	add_compile_definitions(ENABLE_PERFORMANCE_TEST=1)
else()
	add_compile_definitions(ENABLE_PERFORMANCE_TEST=0)
endif()

# BabelTrader-CTP-Quote
add_serv(BabelTrader-CTP-Quote ${CMAKE_CURRENT_LIST_DIR}/src/ctp-quotes)
target_include_directories(BabelTrader-CTP-Quote PRIVATE ${CMAKE_CURRENT_LIST_DIR}/thirdparty/ctp/include)
target_link_libraries(BabelTrader-CTP-Quote thostmduserapi)
if (WIN32)
	add_custom_command(
		TARGET BabelTrader-CTP-Quote
		PRE_LINK
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/thirdparty/ctp/lib/win64/thostmduserapi.dll $(OutDir)
	)
	set_target_properties(BabelTrader-CTP-Quote PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
endif()

# BabelTrader-CTP-Trade
add_serv(BabelTrader-CTP-Trade ${CMAKE_CURRENT_LIST_DIR}/src/ctp-trade)
target_include_directories(BabelTrader-CTP-Trade PRIVATE ${CMAKE_CURRENT_LIST_DIR}/thirdparty/ctp/include)
target_link_libraries(BabelTrader-CTP-Trade thosttraderapi)
if (WIN32)
	add_custom_command(
		TARGET BabelTrader-CTP-Trade
		PRE_LINK
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/thirdparty/ctp/lib/win64/thosttraderapi.dll $(OutDir)
	)
	set_target_properties(BabelTrader-CTP-Trade PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
endif()

# BabelTrader-XTP-Trade
add_serv(BabelTrader-XTP-Trade ${CMAKE_CURRENT_LIST_DIR}/src/xtp-trade)
target_include_directories(BabelTrader-XTP-Trade PRIVATE ${CMAKE_CURRENT_LIST_DIR}/thirdparty/xtp/include)
target_link_libraries(BabelTrader-XTP-Trade xtptraderapi)
if (WIN32)
	add_custom_command(
		TARGET BabelTrader-XTP-Trade
		PRE_LINK
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/thirdparty/xtp/lib/win64/xtptraderapi.dll $(OutDir)
	)
	set_target_properties(BabelTrader-XTP-Trade PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
	target_compile_definitions(BabelTrader-XTP-Trade PRIVATE ISLIB)
endif()

# BabelTrader-XTP-Quote
add_serv(BabelTrader-XTP-Quote ${CMAKE_CURRENT_LIST_DIR}/src/xtp-quotes)
target_include_directories(BabelTrader-XTP-Quote PRIVATE ${CMAKE_CURRENT_LIST_DIR}/thirdparty/xtp/include)
target_link_libraries(BabelTrader-XTP-Quote xtpquoteapi)
if (WIN32)
	add_custom_command(
		TARGET BabelTrader-XTP-Quote
		PRE_LINK
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/thirdparty/xtp/lib/win64/xtpquoteapi.dll $(OutDir)
	)
	set_target_properties(BabelTrader-XTP-Quote PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
	target_compile_definitions(BabelTrader-XTP-Quote PRIVATE ISLIB)
endif()

# fake target
if (WIN32)
	add_custom_target(
		FAKE_COPY_CONFIG ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/config ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(Configuration)/config
	)
else()
	add_custom_target(
		FAKE_COPY_CONFIG ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/config ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config
	)
endif()

# cpp demo
add_serv(test_quote ${CMAKE_CURRENT_LIST_DIR}/demo/cpp/test_quote)

if (WIN32)
	set_target_properties(test_quote
		PROPERTIES
		FOLDER "demo"
		VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
	)
endif()