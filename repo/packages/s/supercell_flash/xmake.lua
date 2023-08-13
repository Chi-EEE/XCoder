package("supercell_flash")
    set_homepage("https://github.com/scwmake/SupercellFlash")
    set_description("C++ library for loading and processing Supercell 2D (.sc) assets")
    set_license("MIT")
    
    add_urls("https://github.com/scwmake/SupercellFlash.git")

    add_versions("2023.08.05", "fc5c9ae27c5a5a409771623ea761acf0dbd7ca79")

    on_install(function (package)
        os.cp("include/*.h", package:installdir("include"))
        io.writefile("xmake.lua", [[
            add_rules("mode.debug","mode.release")
            add_requires("zstd")

            target("supercell_bytestream")
                set_kind("phony")
                add_headerfiles("dependencies/Bytestream/**.h")
                add_includedirs("dependencies/Bytestream", 
                    "dependencies/Bytestream/SupercellBytestream", 
                    "dependencies/Bytestream/SupercellBytestream/base", 
                    {public = true})

            target("supercell_flash")
                set_kind("static")
                set_languages("cxx17")
                add_files("src/**.cpp")
                add_headerfiles("include/**.h")
                add_includedirs("include", {public = true})
                add_deps("supercell_bytestream", "supercell_compression", "supercell_texture_loader")

            local compression_dir = "dependencies/Compression"
            target("supercell_compression")
                set_kind("static")
                set_languages("cxx17")
                add_headerfiles(compression_dir .. "/src/**.h")
                add_files(compression_dir .. "/src/**.cpp")
                add_includedirs(compression_dir .. "/include", {public = true})
                add_includedirs(compression_dir .. "/src")
                add_deps("supercell_bytestream", "lzham","lzma")
                add_packages("zstd")

            local lzham_dir = compression_dir .. "/dependencies/lzham"
            target("lzham")
                set_kind("static")
                set_languages("cxx14")

                add_files(
                    lzham_dir .. "/src/lzham_lib.cpp",
                    -- Decompress
                    lzham_dir .. "/src/lzhamdecomp/lzham_assert.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_checksum.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_huffman_codes.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_lzdecompbase.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_lzdecomp.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_mem.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_platform.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_prefix_coding.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_symbol_codec.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_timer.cpp",
                    lzham_dir .. "/src/lzhamdecomp/lzham_vector.cpp",
                    -- Compress
                    lzham_dir .. "/src/lzhamcomp/lzham_lzbase.cpp",lzham_dir .. "/src/lzhamcomp/lzham_lzcomp.cpp",
                    lzham_dir .. "/src/lzhamcomp/lzham_lzcomp_internal.cpp",
                    lzham_dir .. "/src/lzhamcomp/lzham_lzcomp_state.cpp",lzham_dir .. "/src/lzhamcomp/lzham_match_accel.cpp" 
                    )

                add_headerfiles(
                    -- Decompress
                    lzham_dir .. "/src/lzhamdecomp/lzham_checksum.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_config.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_core.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_mem.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_symbol_codec.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_assert.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_decomp.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_helpers.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_prefix_coding.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_huffman_codes.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_lzdecompbase.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_math.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_platform.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_timer.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_traits.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_types.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_utils.h",
                    lzham_dir .. "/src/lzhamdecomp/lzham_vector.h",
                    -- Compress
                    lzham_dir .. "/src/lzhamcomp/lzham_lzbase.h",
                    lzham_dir .. "/src/lzhamcomp/lzham_lzcomp_internal.h",
                    lzham_dir .. "/src/lzhamcomp/lzham_match_accel.h",
                    lzham_dir .. "/src/lzhamcomp/lzham_null_threading.h",
                    lzham_dir .. "/src/lzhamcomp/lzham_pthreads_threading.h",
                    lzham_dir .. "/src/lzhamcomp/lzham_threading.h"
                    )

                add_headerfiles(lzham_dir .."/include/*.h")

                add_includedirs(lzham_dir .. "/include", lzham_dir .. "/src/lzhamcomp", lzham_dir .. "/src/lzhamdecomp",{public = true})

                add_defines("_LIB")

                if is_mode("debug") then
                    set_optimize("none")
                elseif is_mode("release") then
                    set_optimize("fastest")
                end

                if is_plat("windows") then
                    add_defines("WIN32")
                    add_files(lzham_dir .. "/src/lzhamcomp/lzham_win32_threading.cpp")
                    add_headerfiles(lzham_dir .. "/src/lzhamcomp/lzham_win32_threading.h")
                else
                    add_links("pthread")
                    add_files("src/lzhamcomp/lzham_pthreads_threading.cpp")
                    add_cxxflags("-fno-strict-aliasing","-D_LARGEFILE64_SOURCE=1","-D_FILE_OFFSET_BITS=64")

                    if is_mode("debug") then
                        add_cxxflags("-g","-Wall","-Wextra")
                    elseif is_mode("release") then
                        add_cxxflags("-Wall","-Wextra","-O3","-fomit-frame-pointer","-fexpensive-optimizations")
                    end
                end


            local lzma_dir = compression_dir .. "/dependencies/lzma"
            target("lzma")
                set_kind("static")
                set_languages("c90")
                add_includedirs(lzma_dir .. "/include",{public = true})
                add_files(lzma_dir .. "/src/**.c")

            local texture_dir = "dependencies/TextureLoader"
            target("supercell_texture_loader")
                set_kind("static")
                set_languages("cxx17")
                add_files(texture_dir .. "/src/**.cpp")
                add_includedirs(texture_dir .. "/include", {public = true})
                add_deps("supercell_bytestream", "libktx","astc-encoder","etcpack")

            local astc_encoder_dir = texture_dir .. "/ThirdParty/astc-encoder"
            target("astc-encoder")
                set_kind("static")
                set_languages("cxx17")
                add_files(astc_encoder_dir .. "/src/*.cpp")
                add_files(astc_encoder_dir .. "/src/*.c")
                add_headerfiles(astc_encoder_dir .. "/src/*.h")
                add_includedirs(astc_encoder_dir .. "/include",{public = true})

            local basisu_dir = texture_dir .. "/ThirdParty/basisu"
            target("basisu")
                set_kind("static")
                set_languages("cxx11")

                add_includedirs(basisu_dir .. "/include")
                add_includedirs(basisu_dir .. "/include/encoder")
                add_includedirs(basisu_dir .. "/include/transcoder")

                add_headerfiles(basisu_dir .. "/include/**.h")

                add_files(basisu_dir .. "/src/**.cpp")
                add_defines("BASISD_SUPPORT_KTX2_ZSTD")

                if is_plat("windows") then
                    add_defines("BASISU_SUPPORT_OPENCL","BASISU_SUPPORT_SSE")
                    add_includedirs(basisu_dir .. "/OpenCL")
                    add_includedirs(basisu_dir .. "/OpenCL/CL")
                    add_cxflags("/d2archSSE42")

                    if is_arch("x86") then
                        add_links("OpenCL/lib/OpenCL")
                    elseif is_arch("x86_64") then
                        add_links("OpenCL/lib/OpenCL64")
                    end
                end
                add_packages("zstd")

            local dfdutils_dir = texture_dir .. "/ThirdParty/dfdutils"
            target("dfdutils")
                set_kind("static")
                set_languages("c90")
                add_files(dfdutils_dir .. "/src/**.c")
                add_headerfiles(dfdutils_dir .. "/include/**.h")
                add_includedirs(dfdutils_dir .. "/include",{public = true})
                add_includedirs(dfdutils_dir .. "/include/vulkan",{public = true})
                add_includedirs(dfdutils_dir .. "/include/KHR",{public = true})

            local ETCPACK_dir = texture_dir .. "/ThirdParty/ETCPACK"
            target("etcpack")
                set_kind("static")
                set_languages("cxx17")
                add_files(ETCPACK_dir .. "/src/*.cxx")
                add_headerfiles(ETCPACK_dir .. "/src/*.h")

            local libktx_dir = texture_dir .. "/ThirdParty/libktx"
            target("libktx")
                set_kind("static")
                set_languages("cxx11")
                
                add_files(libktx_dir .. "/src/*.c")
                add_headerfiles(libktx_dir .. "/src/*.h")
                
                add_includedirs(libktx_dir .. "/include",{public = true})
                add_includedirs(libktx_dir .. "/include/GL",{public = true})
                add_includedirs(libktx_dir .. "/include/KHR",{public = true})

                add_deps("basisu","astc-encoder","dfdutils")
                add_packages("zstd")

                add_defines("KHRONOS_STATIC","LIBKTX","BASISU_SUPPORT_OPENCL=0")

                if is_plat("windows") then
                    add_defines("BASISU_SUPPORT_OPENCL=1")
                    add_cxflags("/W4","/WX")
                    if is_mode("debug") then
                        add_cxflags("/Gz","/O2")
                    end
                else
                    add_cxflags("-Wall","-Wextra","-Werror")
                    if is_mode("debug") then
                        add_cxflags("-O0","-O3")
                    end
                end
        ]])
        import("package.tools.xmake").install(package)
    end)

    on_test(function (package)
        assert(package:check_csnippets({test = [[
            void test() {
                SupercellSWF swf;
            }
        ]]}, {includes = "SupercellFlash.h"}))
    end)