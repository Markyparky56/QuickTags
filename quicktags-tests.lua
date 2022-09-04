project "quicktags-tests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    includedirs
    {
        "include"
    }
    files
    {
        "include/QuickTags.hpp",
        "src/quicktags-tests.cpp",
        "quicktags.natvis"
    }
    links { "quicktags-loader" }
