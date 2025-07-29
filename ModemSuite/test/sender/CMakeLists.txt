#[[
  CMakeLists
   Created on: 2024_11_24
       Author: Rev_RoastDuck
       Github: https://github.com/Rev-RoastedDuck
  
  :copyright: (c) 2023 by Rev-RoastedDuck.
]]

#[[----------------- 基础配置信息 --------------------
=====================================================]]
cmake_minimum_required(VERSION 3.0)
project(Embedded-Lib-Test)

set(CMAKE_CXX_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH ../)

#[[------------------ 搜集头文件 ---------------------
=====================================================]]
# 递归搜索hpp文件地址
set(INC_LIST)
file(GLOB_RECURSE INC_LIST_RAW ${CMAKE_CURRENT_SOURCE_DIR}/../../*h)
foreach(inc_path ${INC_LIST_RAW})
    string(FIND ${inc_path} "CMakeFiles" result)
    if(result GREATER -1)
        continue()
    endif()
    
    string(FIND ${inc_path} "xmodem" result)
    if(result GREATER -1)
        continue()
    endif()

    get_filename_component(inc_no_filename ${inc_path} DIRECTORY)
    list(APPEND INC_LIST ${inc_no_filename})
endforeach()
# 去重
list(REMOVE_DUPLICATES INC_LIST)
# 输出过滤后的hpp文件地址
message(STATUS "[Message] Filtered header dir:")
foreach(file ${INC_LIST})
    message("           ${file}")
endforeach()

#[[------------------ 添加头文件-- -------------------
=====================================================]]
include_directories(${INC_LIST})


#[[------------------ 搜集源文件 ---------------------
=====================================================]]
# 递归搜索cpp文件地址
set(SRC_LIST)
file(GLOB_RECURSE SRC_LIST_RAW ${CMAKE_CURRENT_SOURCE_DIR}/../../*.c)
# file(GLOB_RECURSE SRC_LIST_RAW ${CMAKE_CURRENT_SOURCE_DIR}/../algorithm/data_structure/*.c)
# file(GLOB_RECURSE SRC_LIST_RAW_CURR ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
# list(APPEND SRC_LIST_RAW ${SRC_LIST_RAW_CURR})
foreach(src_path ${SRC_LIST_RAW})
    string(FIND ${src_path} "CMakeFiles" result)
    if(result GREATER -1)
        continue()
    endif()
    
    string(FIND ${src_path} "test/receiver" result)
    if(result GREATER -1)
        continue()
    endif()

    string(FIND ${src_path} "xmodem" result)
    if(result GREATER -1)
        continue()
    endif()

    list(APPEND SRC_LIST ${src_path})
endforeach()
# 输出过滤后的cpp文件地址
message(STATUS "[Message] Filtered source files:")
foreach(file ${SRC_LIST})
    message("           ${file}")
endforeach()

#[[---------------- 生成可执行文件 -------------------
=====================================================]]
add_executable(start ${SRC_LIST})

#[[------------------ 添加依赖库-- -------------------
=====================================================]]
target_link_libraries(start m)
