cmake_minimum_required(VERSION 3.11)

execute_process(COMMAND uname OUTPUT_VARIABLE uname)
if (uname MATCHES "^MSYS" OR uname MATCHES "^MINGW")
    message("This is MSYS")
    # Configurer les chemins pour MSYS2
    set(MSYS2_ROOT "C:/msys64/mingw64")  # Chemin d'installation de MSYS2
    set(CMAKE_PREFIX_PATH ${MSYS2_ROOT})
    SET(CMAKE_C_COMPILER gcc)
    SET(CMAKE_CXX_COMPILER g++)
    find_program(CMAKE_C_COMPILER NAMES gcc)
    SET(CMAKE_C_COMPILER gcc)

else()
    message("This is not MSYS")
endif()

find_program(C_COMPILER NAMES gcc)
SET(C_COMPILER gcc)
find_program(CXX_COMPILER NAMES g++)
SET(CXX_COMPILER g++)
message("CMAKE_C_COMPILER => " ${CMAKE_C_COMPILER})

################################
# Options
################################

option(Tests "Tests" OFF)

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
# Ajouter les bibliothèques nécessaires (ws2_32 pour les fonctions réseau sur Windows)
if(uname MATCHES "^MSYS" OR uname MATCHES "^MINGW")
    target_link_libraries(NoMASS2Server PRIVATE NoMASSstatic  nlohmann_json::nlohmann_json ws2_32)
else ()
    target_link_libraries(NoMASS2Server PRIVATE NoMASSstatic  nlohmann_json::nlohmann_json)
endif()

add_executable(NoMASS2Client Source/simulator_client.cpp)

# Ajouter les bibliothèques nécessaires (ws2_32 pour les fonctions réseau sur Windows)
if(uname MATCHES "^MSYS" OR uname MATCHES "^MINGW")
    target_link_libraries(NoMASS2Client PRIVATE nlohmann_json::nlohmann_json ws2_32)
else ()
    target_link_libraries(NoMASS2Client PRIVATE nlohmann_json::nlohmann_json)
endif()


## Spécifiez les standards de C++
set_target_properties(NoMASS2Server PROPERTIES CXX_STANDARD 17)


if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)  # Forcer la compilation pour ARM64
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)

endif ()

if (UNIX OR APPLE)
    set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS "-Werror -Wall -Wextra")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++17 -march=native")
elseif (uname MATCHES "^MSYS" OR uname MATCHES "^MINGW")
    SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY} -static -fPIC")
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
