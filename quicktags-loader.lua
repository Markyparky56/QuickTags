project "quicktags-loader"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    includedirs
    {
        "include"
    }
    files
    {
        "include/QuickTags.hpp",
        "include/QuickTags-Loader.hpp",
        "src/QuickTags-Loader.cpp",
        "quicktags.natvis"
    }