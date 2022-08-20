-- Append solution type to build dir
local BUILD_DIR = path.join("build/", _ACTION)
-- If specific compiler specified, append that too
if _OPTIONS["cc"] ~= nil then
    BUILD_DIR = BUILD_DIR .. "_" .. _OPTIONS["cc"]
end

workspace "QuickTags"
    location (BUILD_DIR)
    startproject "QuickTags-tests"
    configurations {
        "Debug",
        "Release"
    }
    -- Let 32 bit die already
    if os.is64bit() then
        platforms "x86_64"
    else
        platforms "x86"
    end

    targetdir (BUILD_DIR .. "/bin/" .. "%{cfg.shortname}")
    debugdir "%{cfg.targetdir}"
    objdir (BUILD_DIR .. "/bin/obj/" .. "%{cfg.shortname}")
    filter "configurations:Debug"
        defines
        {
            "_DEBUG"
        }
        optimize "Debug"
        symbols "On"
    filter "configurations:Release"
        defines
        {
            "NDEBUG"
        }
        optimize "Full"
    filter "platforms:x86"
        architecture "x86"
    filter "platforms:x86_64"
        architecture "x86_64"
    filter "action:vs*"
        buildoptions { "/Zc:__cplusplus" }
        flags { "MultiProcessorCompile" }

include "quicktags-tests.lua"
include "quicktags-loader.lua"
include "quicktags-analyser.lua"
