package("supercell_flash")
    set_kind("static")

    set_urls("https://github.com/scwmake/SupercellFlash.git")

    add_versions("1", "fc5c9ae27c5a5a409771623ea761acf0dbd7ca79")

    add_deps("premake5")

    on_install("windows", function (package)
        if is_mode("MT") then
            io.replace("premake5.lua", [[kind "StaticLib"]], [[kind "StaticLib" staticruntime "on"]])
        elseif is_mode("MD") then
            io.replace("premake5.lua", [[kind "StaticLib"]], [[kind "StaticLib" staticruntime "off"]])
        elseif is_mode("MTd") then 
            io.replace("premake5.lua", [[kind "StaticLib"]], [[kind "StaticLib" staticruntime "off" debug "on"]])
        elseif is_mode("MDd") then 
            io.replace("premake5.lua", [[kind "StaticLib"]], [[kind "StaticLib" staticruntime "off" debug "on"]])
        end
        os.runv("premake5", {"vs2022", "--file=Workspace.lua"})
        local mode = package:debug() and "Debug" or "Release"
        local configs = {}
        table.insert(configs, "/p:Configuration=" .. mode)
        table.insert(configs, "/p:Platform=" .. (package:is_arch("x64") and "Win64" or "Win32"))
        table.insert(configs, "ScFlash.sln")
        import("package.tools.msbuild").build(package, configs, {upgrade={"ScFlash.sln", "SupercellFlash.vcxproj"}})

        os.cp("dependencies/Bytestream/*.h", package:installdir("include"))
        os.cp("dependencies/Bytestream/SupercellBytestream", package:installdir("include"))

        os.cp("dependencies/Compression/include/*.h", package:installdir("include"))
        os.cp("dependencies/Compression/include/SupercellCompression", package:installdir("include"))
        
        os.cp("dependencies/TextureLoader/include/*.h", package:installdir("include"))
        os.cp("dependencies/TextureLoader/include/textures", package:installdir("include"))

        os.cp("include/*.h", package:installdir("include"))
        os.cp("include/SupercellFlash", package:installdir("include"))
        
        local outputdir = path.join("build", "bin", mode, "windows", "x86_64")
        os.cp(outputdir .. "/**.lib", package:installdir("lib"))
    end)


    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            void test() {
                SupercellSWF swf;
            }
        ]]}, {configs = {languages = "c++17"}, includes = {"SupercellFlash.h"}}))
    end)

    -- on_install("macosx", "linux", function (package)
    --     import("package.tools.autoconf").install(package)
    -- end)
package_end()
