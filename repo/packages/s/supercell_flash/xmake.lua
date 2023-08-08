package("supercell_flash")
    set_kind("static")

    set_urls("https://github.com/scwmake/SupercellFlash.git")

    add_versions("1", "fc5c9ae27c5a5a409771623ea761acf0dbd7ca79")

    add_deps("premake5")

    on_install("windows", function (package)
        if is_mode("MT") then
            io.replace("premake5.lua", [[kind "StaticLib"]], [[kind "StaticLib"\nstaticruntime "on"]])
        elseif is_mode("MD") then
            io.replace("premake5.lua", [[kind "StaticLib"]], [[kind "StaticLib"\nstaticruntime "off"]])
        end
        os.runv("premake5", {"vs2015", "--file=Workspace.lua"})
        for _, dir in ipairs(os.files("**")) do
            print(dir)
        end
        print("OK")
        -- os.execv("echo", {"hello", "xmake!"})
        local configs = {"ScFlash.sln", "-c Release"}
        import("package.tools.msbuild").build(package, configs, {upgrade={"ScFlash.sln", "SupercellFlash.vcxproj"}})
    end)

    -- on_install("macosx", "linux", function (package)
    --     import("package.tools.autoconf").install(package)
    -- end)
package_end()
