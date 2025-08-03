CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(Haptic_Dual_Arm)

IF(MSVC)
  ADD_DEFINITIONS(-DNOMINMAX)
  # Another MSVC quirk. http://stackoverflow.com/questions/6832666/lnk2019-when-including-asio-headers-solution-generated-with-cmake
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP") # For multi-threaded compilation on MSVC
ELSE()
  IF(APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++") # C++11 support on Apple clang
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")# C++11 support on GCC
  ENDIF()
ENDIF()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_BUILD_TYPE "Release")
set(CMAKE_GENERATOR_PLATFORM x64)


include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(include)
include_directories(include/OpenHaptic/include)
include_directories(include/OpenHaptic/utilities/include)
include_directories("D:/eigen-3.3.9")
include_directories("C:/eigen-3.4.0")
include_directories("C:/Program Files/eigen-3.4.0")


set(DEBUG_LIB_DIR 
	"${CMAKE_CURRENT_SOURCE_DIR}/lib"
	"lib"
	"lib/opencv2/"
	"lib/OpenHaptic/lib/x64/Debug"
	"lib/OpenHaptic/utilities/lib/x64/Debug"
)
set(RELEASE_LIB_DIR 
	"${CMAKE_CURRENT_SOURCE_DIR}/lib"
	"lib"
	"lib/opencv2/"
	"lib/OpenHaptic/lib/x64/Release"
	"lib/OpenHaptic/utilities/lib/x64/Release"
)

link_directories(
    $<$<CONFIG:Debug>:${DEBUG_LIB_DIR}>
    $<$<CONFIG:Release>:${RELEASE_LIB_DIR}>
)



FILE(GLOB_RECURSE SRC_FILES src/*.c*)
list(FILTER SRC_FILES EXCLUDE REGEX ".*\\.obj$")
FILE(GLOB_RECURSE HDR_FILES include/*.hpp include/*.h)

FILE(GLOB SHADER_FILES "src/shader/*")
source_group("Shader Files" FILES ${SHADER_FILES})

message(STATUS "Source files after excluding .obj: ${SRC_FILES}")

add_compile_definitions(_XKEYCHECK_H)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_SCL_SECURE_NO_WARNINGS)
add_compile_definitions(_SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING)
add_compile_definitions(_HAS_STD_BYTE=0) # for c++17

ADD_EXECUTABLE(${PROJECT_NAME} ${HDR_FILES} ${SRC_FILES} ${SHADER_FILES})

# model file content
file(GLOB_RECURSE MODEL_FILES CONFIGURE_DEPENDS src/*.obj)

#  copy .obj files to content
foreach(FILE ${MODEL_FILES})
    configure_file(${FILE} ${CMAKE_BINARY_DIR}/model COPYONLY)
endforeach()

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/src/model"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/model"
)



TARGET_LINK_LIBRARIES(${PROJECT_NAME}
			freetype.lib
            glew32s.lib
			glfw3.lib
			$<$<CONFIG:Debug>:opencv_world3416d.lib>
    			$<$<CONFIG:Release>:opencv_world3416.lib>
			opengl32.lib
			legacy_stdio_definitions.lib
			hd.lib
			hl.lib
			HapticMouse.lib
			hdu.lib
			hlu.lib
			SnapConstraints.lib
			assimp-vc140-mt.lib

)
