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
        "include/*.hpp",
        "src/*.cpp",
        "quicktags.natvis"
    }