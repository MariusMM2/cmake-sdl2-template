cmake_minimum_required(VERSION 3.10)
project(SDLFirstTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(SDL2_DIR ${PROJECT_SOURCE_DIR}/SDL2-2.0.8)

set(SOURCE_FILES
        #02_getting_an_image_on_the_screen/02_getting_an_image_on_the_screen.cpp
        #04_key_presses/04_key_presses.cpp
        #31_scrolling_backgrounds/31_scrolling_backgrounds.cpp
        32_text_input_and_clipboard_handling/32_text_input_and_clipboard_handling.cpp
        #main.cpp
        )

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
)
