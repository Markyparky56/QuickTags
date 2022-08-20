project "quicktags-analyser"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    debugargs
    {
        "-f",
        "../../../../src/Tags.txt"
    }
    includedirs
    {
        "include"
    }
    files
    {
        "include/QuickTags.hpp",
        "src/quicktags-analyser.cpp",
        "quicktags.natvis"
    }
    links { "quicktags-loader" }
