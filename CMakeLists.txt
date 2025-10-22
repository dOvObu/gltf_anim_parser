cmake_minimum_required(VERSION 3.30)
project(opengl_tutorials)

set(CMAKE_CXX_STANDARD 20)

# Пути к Mesa
set(MESA_INCLUDE_DIR "/usr/local/include/mesa")
set(MESA_LIB_DIR "/usr/local/lib")  # или путь Homebrew, например /opt/homebrew/opt/mesa/lib

add_executable(opengl_tutorials
        main.cpp glad.c stb_image.cpp
        Window.h Window.cpp
        WindowsFactory.h WindowsFactory.cpp
        Shader.h Shader.cpp
        PrimitiveMesh.h PrimitiveMesh.cpp
        Tex.h Tex.cpp
        Tr.h Tr.cpp
        MeshRenderer.cpp MeshRenderer.h
        Camera.h
        json.hpp
        AssetsUtility.h
        SkinnedRenderer.h SkinnedRenderer.cpp
        Scene.h
        tiny_gltf.hpp
        GLTF.h
        GPUUtility.h
        base64.h base64.cpp
)

# Подключаем заголовки
target_include_directories(opengl_tutorials PUBLIC
        /usr/local/include          # здесь лежит glad.h
        ${MESA_INCLUDE_DIR}         # Mesa
        /opt/homebrew/Cellar/glfw/3.4/include
)

# Линки
target_link_directories(opengl_tutorials PUBLIC ${MESA_LIB_DIR})

target_link_libraries(opengl_tutorials "-lglfw -L/opt/homebrew/Cellar/glfw/3.4/lib")
target_link_libraries(opengl_tutorials "-lGL -L/opt/homebrew/Cellar/mesa/25.2.2/lib") # Mesa libGL

# Линкуем Mesa OpenGL вместо системного
target_link_libraries(opengl_tutorials
        pthread  # Mesa может требовать pthread
        dl       # Mesa может требовать dl
)

target_link_libraries(opengl_tutorials "-framework Cocoa")
target_link_libraries(opengl_tutorials "-framework IOKit")
target_link_libraries(opengl_tutorials "-framework CoreVideo")
