cmake_minimum_required (VERSION 2.6)

if(APPLE)
  cmake_policy(SET CMP0042 NEW)
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

if(CrossCompile32 OR CrossCompile64)
  SET(CMAKE_SYSTEM_NAME Windows)
  set(COMPILER_PREFIX "i686-w64-mingw32")
  if(CrossCompile64)
    set(COMPILER_PREFIX "x86_64-w64-mingw32")
  endif()
  find_program(CMAKE_RC_COMPILER NAMES ${COMPILER_PREFIX}-windres)
  SET(CMAKE_RC_COMPILER ${COMPILER_PREFIX}-windres)
  find_program(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-gcc)
  SET(CMAKE_C_COMPILER ${COMPILER_PREFIX}-gcc)
  find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-g++)
  SET(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-g++)

  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

endif()


if(CrossCompileLinux64)
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

endif()

################################
# Project
################################

project (FMI)

SET( SRC
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

ADD_LIBRARY(FMI SHARED Source/fmuTemplate.cpp)
target_link_libraries(FMI NoMASSstatic)

add_executable(NoMASS Source/main.cpp)
target_link_libraries(NoMASS NoMASSstatic)

add_executable(NoMASS2 Source/main_boris.cpp)
target_link_libraries(NoMASS2 NoMASSstatic)

if(UNIX OR APPLE)
  set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS "-Werror -Wall -Wextra")
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")
elseif(WIN32)
  SET( CMAKE_CXX_CREATE_SHARED_LIBRARY  "${CMAKE_CXX_CREATE_SHARED_LIBRARY} -static -fPIC" )
endif()

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11")



include_directories("${CMAKE_CURRENT_BINARY_DIR}")

################################
# Testing
################################
if (Tests)
 add_subdirectory(tests)
endif()
