add_rules("mode.debug", "mode.release")

add_repositories("xcoder-repo repo")

add_requires("tl_expected")
add_requires("qt6widgets")
add_requires("opencv")

add_requires("supercell_flash")

set_languages("cxx17")

target("XCoder")
    set_kind("binary")
    add_rules("qt.console")

    add_packages("tl_expected")
    add_packages("qt6widgets")
    add_packages("opencv")
    add_packages("supercell_flash")

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")

    set_runargs("C:/Users/admin/Downloads/CR/assets/sc/chr_axe_man.sc")