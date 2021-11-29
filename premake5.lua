workspace "Game"
    architecture "x64"

    configurations {
        "Debug",
        "Release"
    }

output_dir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

project "Game"
    location "projects"
    kind "ConsoleApp"
    language "C"

    targetdir ("builds/bin/" .. output_dir .. "/%{prj.name}")
    objdir ("builds/obj/" .. output_dir .. "/%{prj.name}")

    files {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs {
        "vendor/GLFW/include",
        "vendor/vk_mem_alloc",
        "vendor/HandmadeMath",
        "vendor/tiny_obj_loader",
        "C:/VulkanSDK/1.2.162.0/include/"
    }

    links {
        "vendor/GLFW/lib/glfw3",
        "C:/VulkanSDK/1.2.162.0/lib/vulkan-1",
    }