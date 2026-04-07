cmake_minimum_required(VERSION 3.16)

project(hardwareConfig LANGUAGES CXX)


# =======================
# 基础配置
# =======================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# =======================
# 第三方库路径配置
# =======================
set(OpenCV_DIR "D:/OpenCV/opencv/build")
set(ONNXRUNTIME_DIR "D:/YOLOv8/onnxruntime-win-x64-1.18.1")

find_package(OpenCV REQUIRED)
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")

# =======================
# 公共 include（项目自身）
# =======================
set(PROJECT_COMMON_INCLUDES
    ${CMAKE_SOURCE_DIR}/src/motor/include
    ${CMAKE_SOURCE_DIR}/src/camera/include
    ${CMAKE_SOURCE_DIR}/src/relay/include
)



# =======================
# motor
# =======================
aux_source_directory(${CMAKE_SOURCE_DIR}/src/motor/src MOTOR_SRC)
add_executable(motortest ${MOTOR_SRC})
target_include_directories(motortest PRIVATE ${PROJECT_COMMON_INCLUDES})

# =======================
# relay
# =======================
aux_source_directory(${CMAKE_SOURCE_DIR}/src/relay/src RELAY_SRC)
add_executable(relaytest ${RELAY_SRC} )
target_include_directories(relaytest PRIVATE ${PROJECT_COMMON_INCLUDES})
target_link_libraries(relaytest PRIVATE ws2_32)

# =======================
# camera
# =======================
aux_source_directory(${CMAKE_SOURCE_DIR}/src/camera/src CAMERA_SRC)
add_executable(cameratest ${CAMERA_SRC} )
target_include_directories(cameratest PRIVATE ${PROJECT_COMMON_INCLUDES} ${ONNXRUNTIME_DIR}/include)
target_link_directories(cameratest PRIVATE D:/Users/LeyanWang/Desktop/hardwareConfig/src/camera/lib ${ONNXRUNTIME_DIR}/lib)
target_link_libraries(cameratest PRIVATE ${OpenCV_LIBS} GxIAPICPPEx onnxruntime)


# =======================
# OpenCV DLL 拷贝
# =======================
add_custom_command(TARGET cameratest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${OpenCV_DIR}/x64/vc16/bin"
        "${EXECUTABLE_OUTPUT_PATH}/$<CONFIG>"
)
# =======================
# ONNXruntime DLL 拷贝
# =======================
add_custom_command(TARGET cameratest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ONNXRUNTIME_DIR}/lib/onnxruntime.dll"
        "${EXECUTABLE_OUTPUT_PATH}/$<CONFIG>"
)


if (MSVC)
    target_compile_options(cameratest PRIVATE /utf-8)
endif()
