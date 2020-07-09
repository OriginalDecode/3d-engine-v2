
newoption {
    trigger = "platform",
    value = "Platform",
    description = "Set platform",
    allowed = {
        { "windows", "Windows" },
        { "linux", "Linux" }
    }
}

newoption {
    trigger = "project",
    value = "Project",
    description = "Set project flag",
    allowed = {
        { "unit_test", "unit test" },
        { "engine", "engine" },
        { "thirdparty", "thirdparty" }
    }
}

newoption {
    trigger = "cflags",
    value = "cflags",
    description = "compiler flags to add things for compilation",
    allowed = {
        { "freetype", "freetype" }
    }
}

if os.isdir('../bin/') == nil then
    print('no direcotry, creating')
    os.mkdir('../bin/')
end

if os.isfile('../bin/.gitignore') == false then
    print('no')
    io.writefile('../bin/.gitignore', '*')
end
    

--beware of the scope issue https://github.com/premake/premake-core/wiki/Scopes-and-Inheritance
if _OPTIONS["project"] == nil then
    print("No project set, will not configure")
    return 
end 

if _OPTIONS["project"] == "engine" then
    print("configuring Engine")
workspace "Engine" --this is the solution name in a vs project if no filename specified
elseif _OPTIONS["project"] == "unit_test" then
    print("configuring UnitTest")
workspace "UnitTest"
else
    return
end
    -- fnc.setWorkspace("Engine")
    -- fnc.addConfig("Debug")
    -- fnc.addConfig("Release")
    -- fnc.setPlatform("Windows")
    configurations { "Debug" , "Release" }

    platforms { _OPTIONS["platform"] }
    debugdir "%{wks.location}../bin"
    
-- language "C++"
    cppdialect "c++17"
    disablewarnings { "4201" }
-- flags { "C++14" }
--filename "whiteroom"
-- platforms { "Windows" }
    architecture "x64"
    includedirs { ".\\" }
    -- libdirs { "" }
    -- flags { "StaticRuntime" }
    warnings "Extra"

    objdir "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}"
    staticruntime "on"
    filter "system:Windows"
        systemversion "latest"

    filter "platforms:Windows"
        defines { "_WIN32", "_CRT_SECURE_NO_WARNINGS" }
            
    filter "platforms:Linux"
        defines { "_GCC_" }
    
    filter "kind:StaticLib"
        targetdir "%{wks.location}/lib/%{cfg.buildcfg}"

    filter "kind:WindowedApp"
        targetdir "%{wks.location}/../bin"
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines {"NDEBUG"}
        optimize "On"

if _OPTIONS["project"] ~= nil then
    if _OPTIONS["project"] == "engine" then
        startproject "Executable"
        project "Executable" --project name
            targetname "%{wks.name}_%{cfg.buildcfg}"
            kind "WindowedApp" --type [ConsoleApp, WindowedApp, SharedLib, StaticLib, Makefile, Utility, None, AndroidProj], WindowedApp is important on Windows and Mac OS X
            location ("./executable")
            includedirs { "./external_libs/" }
            dependson { "Core", "Graphics", "Input", "Logger", "Game" }
            links { "Graphics", "Core", "Input", "Logger", "Game" } --libraries to link

            files { "executable/*.cpp" }
    elseif _OPTIONS["project"] == "unit_test" then
        startproject "UnitTest"
        project "UnitTest" --project name
            targetname "%{wks.name}_%{cfg.buildcfg}Test"
            kind "ConsoleApp" --type [ConsoleApp, WindowedApp, SharedLib, StaticLib, Makefile, Utility, None, AndroidProj], WindowedApp is important on Windows and Mac OS X
            location ("./unit_test")
            define { "UNIT_TEST" }
            dependson { "Core" }
            links { "Core", "Input", "Logger", "external_libs/googletest/lib/Debug/gtestd.lib", 
                                                            "external_libs/googletest/lib/Debug/gtest_maind.lib",
                                                            "external_libs/googletest/lib/Debug/gmockd.lib", 
                                                            "external_libs/googletest/lib/Debug/gmock_maind.lib" } --libraries to link
            files { "unit_test/*.cpp" }
    end
else
    print("No project set")
end

    project "Graphics"
        kind "StaticLib"
        location ("./graphics")    
        files { "graphics/**.cpp", "graphics/**.c", "graphics/**.cxx", "graphics/**.h", "graphics/**.hpp" }
        includedirs { "./external_libs/" }
        dependson { "Core", "ImGui" }
        links { "$(VULKAN_SDK)/lib/vulkan-1.lib", 
                "thirdparty/freetype/freetype.lib", 
                "ImGui" }

        includedirs { "$(VULKAN_SDK)/Include/",
                      "thirdparty/freetype/" }
        -- symbolspath does not seem to work as inteded
        -- symbolspath "%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}.pdb"
        

    project "Core"
        kind "StaticLib"
        location ("./core")
        files { "core/**.cpp", "core/**.h", "core/**.c", "core/**.hpp" }

    project "Input"
        kind "StaticLib"
        location("./input")
        files{"input/**.cpp", "input/**.h", "input/**.hpp", "input/**.c"}
        dependson{"Core"}
        links { "Core" }
    
    project "Game"
        kind "StaticLib"
        location("./game")
        files{"game/**.cpp", "game/**.h", "game/**.hpp", "game/**.c"}
        dependson{"Core", "Input"}
        links { "Core", "Input"}

    project "Logger"
        kind "StaticLib"
        location("./logger")
        links { "Core" }
        dependson { "Core" }
        files{"logger/**.cpp", "logger/**.h", "logger/**.hpp", "logger/**.c"}
    
    project "ImGui"
        kind "StaticLib"
        location("./external_libs/imgui")
        includedirs { "./external_libs/", "$(VULKAN_SDK)/Include/" }
        files{  "external_libs/imgui/*.h", "external_libs/imgui/*.cpp", 
                "external_libs/imgui/misc/**.cpp", "external_libs/imgui/misc/**.h",
                "external_libs/imgui/examples/imgui_impl_win32.*", 
                "external_libs/imgui/examples/imgui_impl_vulkan.*" }
        if _OPTIONS["cflags"] ~= "freetype" then
            print("excluding freetype")
            excludes { "./external_libs/imgui/misc/freetype/**" }
        end