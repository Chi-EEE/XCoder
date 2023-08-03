add_rules("mode.debug", "mode.release")
add_requires("tl_expected")
add_requires("qt6widgets")
add_requires("opencv")

set_languages("cxx17")

target("XCoder")
    set_kind("binary")
    add_rules("qt.console")

    add_packages("tl_expected")
    add_packages("qt6widgets")
    add_packages("opencv")

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    
    after_build(function(target)
        for _, dll in ipairs(os.files("$(scriptdir)/lib/dll/*.dll")) do
            os.cp(dll, path.join(target:targetdir(), path.filename(dll)))
        end
    end)

    -- To make it possible to access the SupercellFlash files
    add_includedirs("include")

    -- SupercellFlash files
    add_linkdirs("lib/$(mode)")
    add_links(
        "ASTC",
        "ETCPACK",
        "LZHAM", 
        "LZMA", 
        "SupercellFlash", 
        "SupercellCompression", 
        "SupercellTextureLoader", 
        "Zstandard"
    )
    set_runargs("C:/Users/admin/Downloads/CRA/assets/sc/chr_axe_man.sc")