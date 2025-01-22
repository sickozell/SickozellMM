cmake_minimum_required(VERSION 3.24)

########################################################################################
# this line is used to build hardware plugin (uncomment the next one)
#include(${CMAKE_CURRENT_LIST_DIR}/../../metamodule/metamodule-plugin-sdk/plugin.cmake)

# this line below (INSTEAD OF PREVIOUS) is used to build plugin in the simulator
include(${METAMODULE_SDK_DIR}/plugin.cmake)

########################################################################################

project(Sickozell VERSION 0.3 DESCRIPTION "Sickozell for MetaModule" LANGUAGES C CXX ASM)

add_library(Sickozell STATIC)

# Add sources from the Rack source repo
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/Sickozell)

target_sources(Sickozell PRIVATE
    ${SOURCE_DIR}/src/plugin.cpp 
    ${SOURCE_DIR}/src/Adder8.cpp
    ${SOURCE_DIR}/src/Bgates.cpp
    ${SOURCE_DIR}/src/Blender.cpp
    ${SOURCE_DIR}/src/Blender8.cpp
    ${SOURCE_DIR}/src/Btoggler.cpp
    ${SOURCE_DIR}/src/BtogglerSt.cpp
    ${SOURCE_DIR}/src/BtogglerStCompact.cpp
    ${SOURCE_DIR}/src/Calcs.cpp
    ${SOURCE_DIR}/src/Clocker2.cpp
    ${SOURCE_DIR}/src/CVrouter.cpp
    ${SOURCE_DIR}/src/CVswitcher.cpp
    ${SOURCE_DIR}/src/Drummer.cpp
    ${SOURCE_DIR}/src/Drummer4.cpp
    ${SOURCE_DIR}/src/Drummer4Plus.cpp
    ${SOURCE_DIR}/src/EnverL.cpp
    ${SOURCE_DIR}/src/Holder8L.cpp
    ${SOURCE_DIR}/src/HolderCompactL.cpp
    ${SOURCE_DIR}/src/HolderL.cpp
    ${SOURCE_DIR}/src/Modulator.cpp
    ${SOURCE_DIR}/src/Modulator7.cpp
    ${SOURCE_DIR}/src/Modulator7Compact.cpp
    ${SOURCE_DIR}/src/MultiRouterL.cpp
    ${SOURCE_DIR}/src/MultiRouterML.cpp
    ${SOURCE_DIR}/src/MultiSwitcherL.cpp
    ${SOURCE_DIR}/src/MultiSwitcherML.cpp
    ${SOURCE_DIR}/src/RandLoops.cpp
    ${SOURCE_DIR}/src/Shifter.cpp
    ${SOURCE_DIR}/src/SickoAmp.cpp
    ${SOURCE_DIR}/src/SickoCrosser4L.cpp
    ${SOURCE_DIR}/src/SickoCrosserL.cpp
    ${SOURCE_DIR}/src/SickoLooper1.cpp
    ${SOURCE_DIR}/src/SickoLooper1Exp.cpp
    ${SOURCE_DIR}/src/SickoLooper3.cpp
    ${SOURCE_DIR}/src/SickoQuant4L.cpp
    ${SOURCE_DIR}/src/SickoQuantL.cpp
    ${SOURCE_DIR}/src/SickoSampler2.cpp
    ${SOURCE_DIR}/src/SimpleSeq4.cpp
    ${SOURCE_DIR}/src/SlewerL.cpp
    ${SOURCE_DIR}/src/Switcher.cpp
    ${SOURCE_DIR}/src/Switcher8.cpp
    ${SOURCE_DIR}/src/SwitcherSt.cpp
    ${SOURCE_DIR}/src/TogglerCompact.cpp
    ${SOURCE_DIR}/src/TogglerMM.cpp
)

# Add includes and compile options for source repo
target_include_directories(Sickozell PRIVATE ${SOURCE_DIR}/src)

# Call this to link and create the plugin file
create_plugin(
    SOURCE_LIB      Sickozell                                          # The cmake target name (defined in add_target)
    PLUGIN_NAME     Sickozell                                          # This must match the brand "slug" used in VCV Rack
    PLUGIN_JSON     ${SOURCE_DIR}/plugin.json                         # Path to the plugin.json file used by VCV Rack
    SOURCE_ASSETS   ${CMAKE_CURRENT_LIST_DIR}/assets                  # Path to the assets/ dir containing the PNGs
    DESTINATION     ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugin   # Path to where you want the plugin file output
)
