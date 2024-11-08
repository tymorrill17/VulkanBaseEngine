find_package(Vulkan REQUIRED)

# VMA is a header-only libary, so we can simply specify the include directory
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE $ENV{VULKAN_SDK}/Include)

# GLM is a header-only libary, so we can simply specify the include directory
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE $ENV{VULKAN_SDK}/Include)

# We must include headers and link libaries with SDL2, so specify the include and lib directories, then actually link the .lib files
add_library(sdl2 INTERFACE)
target_include_directories(sdl2 INTERFACE $ENV{VULKAN_SDK}/Include)
target_link_directories(sdl2 INTERFACE $ENV{VULKAN_SDK}/Lib)
target_link_libraries(sdl2 INTERFACE SDL2 SDL2main) # These are the names of .lib files in the specified link directory above

add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)
file(GLOB_RECURSE IMGUI_SOURCES "./imgui/*.cpp")
file(GLOB_RECURSE IMGUI_HEADERS "./imgui/*.h")
target_sources(imgui PRIVATE ${IMGUI_SOURCES} ${IMGUI_HEADERS})
target_link_libraries(imgui PUBLIC Vulkan::Vulkan sdl2)


# This is only a single header-file so just specify include directory
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb_image)

