cmake_minimum_required(VERSION 3.11)

find_program(C_COMPILER NAMES gcc)
SET(C_COMPILER gcc)
find_program(CXX_COMPILER NAMES g++)
SET(CXX_COMPILER g++)

execute_process(COMMAND uname OUTPUT_VARIABLE uname)
if (uname MATCHES "^MSYS" OR uname MATCHES "^MINGW")
    message("This is MSYS")
    # Configurer les chemins pour MSYS2
    set(MSYS2_ROOT "C:/msys64/mingw64")  # Chemin d'installation de MSYS2
    set(CMAKE_PREFIX_PATH ${MSYS2_ROOT})
    SET(CMAKE_C_COMPILER "/mingw64/bin/gcc")
    SET(CMAKE_CXX_COMPILER "/mingw64/bin/g++")
    find_program(CMAKE_C_COMPILER NAMES gcc)
    SET(CMAKE_C_COMPILER gcc)
    message("CMAKE_C_COMPILER" ${CMAKE_C_COMPILER})

else()
    message("This is not MSYS")
endif()


################################
# Options
################################

option(Tests "Tests" OFF)
option(CrossCompile32 "CrossCompile32" OFF)
option(CrossCompile64 "CrossCompile64" OFF)
option(CrossCompileLinux64 "CrossCompileLinux64" OFF)

################################
# Cross Compiling
################################

if (CrossCompile32 OR CrossCompile64)
    SET(CMAKE_SYSTEM_NAME Windows)
    set(COMPILER_PREFIX "i686-w64-mingw32")
    if (CrossCompile64)
        set(COMPILER_PREFIX "x86_64-w64-mingw32")
    endif ()
    find_program(CMAKE_RC_COMPILER NAMES ${COMPILER_PREFIX}-windres)
    SET(CMAKE_RC_COMPILER ${COMPILER_PREFIX}-windres)
    find_program(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-gcc)
    SET(CMAKE_C_COMPILER ${COMPILER_PREFIX}-gcc)
    find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-g++)
    SET(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-g++)

    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

endif ()

if (CrossCompileLinux64)
    SET(CMAKE_SYSTEM_NAME Linux)
    #  set(COMPILER_PREFIX "x86_64-elf")
    #  set(CMAKE_FIND_ROOT_PATH /opt/homebrew/opt/x86_64-elf)
    #  set(CMAKE_ENABLE_EXPORTS OFF)
    #  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
    #  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

    #  find_program(CMAKE_RC_COMPILER NAMES ${COMPILER_PREFIX}-windres)
    #  SET(CMAKE_RC_COMPILER ${COMPILER_PREFIX}-windres)

    #  find_program(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-gcc)
    #  SET(CMAKE_C_COMPILER ${COMPILER_PREFIX}-gcc)

    #  find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-c++)
    #  SET(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-g++)

    # Spécifie le compilateur
    set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")

    # Options pour cibler Linux
    set(CMAKE_CXX_FLAGS "-target x86_64-linux-gnu")
    set(CMAKE_EXE_LINKER_FLAGS "-L/opt/homebrew/lib/x86_64-linux-gnu -L/opt/homebrew/opt/glib/lib -Wl,--rpath,/opt/homebrew/opt/glib/lib")

    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

endif ()



#find_package(ASIO REQUIRED)

if (APPLE)
    set(ASIO_INCLUDE_DIR "/opt/homebrew/Cellar/asio/1.30.2/include")
    # Rechercher Asio
    find_path(ASIO_INCLUDE_DIR NAMES asio.hpp PATH_SUFFIXES asio)
elseif (uname MATCHES "^MSYS" OR uname MATCHES "^MINGW")
    message(STATUS "je suis ici")

    set(ASIO_INCLUDE_DIR "/mingw64/include")

    find_path(
            ASIO_INCLUDE_DIR
            "asio.hpp"
            HINTS
            /mingw64/include
            REQUIRED
    )
else ()
    find_path(
        ASIO_INCLUDE_DIR
        "asio.hpp"
        HINTS
          /usr/include
          /usr/local/include
        REQUIRED
    )
endif ()

# Si Asio est trouvé, ajouter son répertoire d'en-tête
if (ASIO_INCLUDE_DIR)
    message(STATUS "Found Asio: ${ASIO_INCLUDE_DIR}")
    include_directories(${ASIO_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "Asio not found. Install via 'brew install asio'")
endif ()

find_package(nlohmann_json 3.11.3 REQUIRED)

#include(FetchContent)

#FetchContent_Declare(
#        json
#        GIT_REPOSITORY https://github.com/nlohmann/json.git
##        GIT_TAG v3.11.3 # Version à vérifier
#)

#FetchContent_MakeAvailable(json)

if (APPLE)
    set(JSON_INCLUDE_DIR "/opt/homebrew/Cellar/nlohmann-json/3.11.3/include")
    # Rechercher Json
    find_path(JSON_INCLUDE_DIR NAMES json.hpp PATH_SUFFIXES json)
elseif (uname MATCHES "^MSYS" OR uname MATCHES "^MINGW")
    set(JSON_INCLUDE_DIR "/mingw64/include/nlohmann")
    find_path(
        JSON_INCLUDE_DIR
        "json.hpp"
        HINTS
          /usr/include
          /usr/local/include
        REQUIRED
    )
endif ()

# Si Json est trouvé, ajouter son répertoire d'en-tête
if (JSON_INCLUDE_DIR)
    message(STATUS "Found nlohmann: ${JSON_INCLUDE_DIR}")
    include_directories(${message})
else ()
    message(FATAL_ERROR "Json not found. Install via 'brew install nlohmann-json'")
endif ()


################################
# Project
################################

project(FMI)

SET(SRC
        Source/Agent.cpp
        Source/Occupant.cpp
        Source/Occupant_Action.cpp
        Source/Occupant_Action_Heat_Gains.cpp
        Source/Occupant_Action_Lights.cpp
        Source/Occupant_Action_Lights_BDI.cpp
        Source/Occupant_Action_Shades.cpp
        Source/Occupant_Action_Shades_BDI.cpp
        Source/Occupant_Action_Window.cpp
        Source/Occupant_Action_Window_Stochastic.cpp
        Source/Occupant_Action_Window_Stochastic_BDI.cpp
        Source/Occupant_Action_Window_Learning.cpp
        Source/Occupant_Action_Appliance.cpp
        Source/Occupant_Action_Appliance_BDI.cpp
        Source/Occupant_Action_HeatingSetPoints_Learning.cpp
        Source/Occupant_Zone.cpp
        Source/DataStore.cpp
        Source/Log.cpp
        Source/Model_Activity.cpp
        Source/Model_Activity_Survival.cpp
        Source/Model_ExternalShading.cpp
        Source/Model_HeatGains.cpp
        Source/Model_Lights.cpp
        Source/Model_Presence.cpp
        Source/Model_RandomWeibull.cpp
        Source/Model_Windows.cpp
        Source/Model_Appliance_Ownership.cpp
        Source/Model_Appliance_Large_Usage.cpp
        Source/Model_Appliance_Large_Usage_Survival.cpp
        Source/Model_Appliance_Small_Usage.cpp
        Source/Model_Appliance_Power_CSV.cpp
        Source/Simulation.cpp
        Source/Configuration.cpp
        Source/SimulationTime.cpp
        Source/State.cpp
        Source/StateMachine.cpp
        Source/Utility.cpp
        Source/Environment.cpp
        Source/Building.cpp
        Source/Building_Zone.cpp
        Source/Building_Appliances.cpp
        Source/fmiFunctions.cpp
        Source/QLearning.cpp
        Source/Appliance.cpp
        Source/Appliance_Large.cpp
        Source/Appliance_Large_CSV.cpp
        Source/Appliance_Small.cpp
        Source/Appliance_Generic_CSV.cpp
        Source/Appliance_FMI.cpp
        Source/Appliance_Large_Learning.cpp
        Source/Appliance_Large_Learning_CSV.cpp
        Source/Appliance_Battery.cpp
        Source/Appliance_Battery_GridCost_Reward.cpp
        Source/LVN.cpp
        Source/LVN_Node.cpp
        Source/Contract_Node_Priority.cpp
        Source/Contract_Node_Supply.cpp
        Source/Contract_Negotiation.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/../rapidxml)
include_directories(${PROJECT_SOURCE_DIR}/Source)

ADD_LIBRARY(NoMASSstatic STATIC ${SRC})
target_link_libraries(NoMASSstatic nlohmann_json::nlohmann_json)

ADD_LIBRARY(FMI SHARED Source/fmuTemplate.cpp)
target_link_libraries(FMI NoMASSstatic)

add_executable(NoMASS Source/main.cpp)
target_link_libraries(NoMASS NoMASSstatic)


# Trouver Protobuf
#find_package(Protobuf REQUIRED)
#find_package(absl CONFIG REQUIRED)


# Ajouter les fichiers proto
#set(PROTO_FILES message.proto)

# Générer les fichiers C++ depuis le .proto
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})


# Ajouter les fichiers sources et d'en-tête
#set(SOURCE
#        message.pb.cc
#        ${PROTO_SRCS}  # Fichiers générés par Protobuf
#)

#set(HEADERS
#        message.pb.h
#        ${PROTO_HDRS}  # Fichiers d'en-tête générés par Protobuf
#)


#  find_package(ABSEIL EXACT MODULE)

# if (APPLE)
#     set(ABSEIL_INCLUDE_DIR "/opt/homebrew/Cellar/abseil/20240722.0/include/")
#     # Rechercher abseil
 #    find_path(ABSEIL_INCLUDE_DIR NAMES "absl/algorithm/algorithm.h" PATH_SUFFIXES abseil)
# else ()
#     find_path(
#         ABSEIL_INCLUDE_DIR
#         "absl/algorithm/algorithm.h"
#         HINTS
#          /usr/include
#           /usr/local/include
#        REQUIRED
#     )
# endif ()

# if (ABSEIL_INCLUDE_DIR)
#     message(STATUS "Found abseil: ${ABSEIL_INCLUDE_DIR}")
#     include_directories(${message})
# else ()
 #    message(FATAL_ERROR "abseil not found. Install via 'brew install abseil'")
#     message(FATAL_ERROR "abseil not found. Install via 'apt-target install abseil'")
# endif ()

#message(STATUS "Found PROTO_SRCS: ${PROTO_SRCS}")

# Ajouter l'exécutable
#add_executable(NoMASS2Server_protobuf Source/simulator_server_protobuf.cpp ${SOURCES} ${HEADERS})
## Lier Protobuf à l'exécutable
#target_link_libraries(NoMASS2Server_protobuf PRIVATE ${Protobuf_LIBRARIES} NoMASSstatic absl::base absl::strings absl::log)
#include_directories(${Protobuf_INCLUDE_DIRS})
#set_target_properties(NoMASS2Server_protobuf PROPERTIES CXX_STANDARD 17)

add_executable(NoMASS2 Source/main_boris.cpp)
target_link_libraries(NoMASS2 NoMASSstatic nlohmann_json::nlohmann_json)

add_executable(NoMASS2Server Source/simulator_server.cpp)
target_link_libraries(NoMASS2Server PRIVATE NoMASSstatic  nlohmann_json::nlohmann_json)

add_executable(NoMASS2Client Source/simulator_client.cpp)
target_link_libraries(NoMASS2Client PRIVATE nlohmann_json::nlohmann_json)

## Spécifiez les standards de C++
set_target_properties(NoMASS2Server PROPERTIES CXX_STANDARD 17)


if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)  # Forcer la compilation pour ARM64
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)

endif ()

if (UNIX OR APPLE)
    set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS "-Werror -Wall -Wextra")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++17 -march=native")
elseif (WIN32)
    SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY} -static -fPIC")
endif ()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

################################
# Testing
################################
if (Tests)
    add_subdirectory(tests)
endif ()
