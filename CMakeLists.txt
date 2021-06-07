cmake_minimum_required(VERSION 3.16)
project(vitanet)
if (VITA)
    include("${VITASDK}/share/vita.cmake" REQUIRED)
    message(INFO "${VITASDK}")
endif ()

find_package(SDL REQUIRED)

set(VITA_APP_NAME "VitaNet")
set(VITA_TITLEID "VNET02121")
set(VITA_VERSION "01.00")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Ofast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

include_directories("${SDL_INCLUDE_DIR}")

add_executable(${PROJECT_NAME}.elf
        main.cpp
        request_tools.cpp server_conn.cpp graph.cpp video.cpp
        input.cpp)

target_link_libraries(${PROJECT_NAME}.elf
        curlpp
        curl
        z
        ssl
        crypto
        SDL
        SDL_image
        SDL_gfx
        )

if (VITA)
    target_link_libraries(${PROJECT_NAME}.elf
            SceSysmodule_stub
            SceDisplay_stub
            SceNet_stub
            SceNetCtl_stub
            SceHttp_stub
            SceCtrl_stub
            SceAudio_stub
            SceHid_stub
            SceCommonDialog_stub
            SceIme_stub
            SceAppUtil_stub
            jpeg
            png
            webp
            pthread
            )
    vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME}.elf)
    vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
            VERSION ${VITA_VERSION}
            NAME ${VITA_APP_NAME}
            FILE kyuhen.png sce_sys/pic0.png
            FILE icon.png sce_sys/icon0.png
            FILE icon.png sce_sys/livearea/contents/logo0.png
            FILE bg.png sce_sys/livearea/contents/bg.png
            FILE template.xml sce_sys/livearea/contents/template.xml
            FILE kyuhen.png sce_sys/livearea/contents/startup.png
            )
endif ()