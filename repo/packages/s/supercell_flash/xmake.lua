package("supercell_flash")
    set_kind("static")

    set_urls("https://github.com/scwmake/SupercellFlash.git")

    add_versions("2023.08.05", "fc5c9ae27c5a5a409771623ea761acf0dbd7ca79")

    on_install(function (package)
        io.writefile("xmake.lua", [[
            add_requires("zstd")
            target("supercell_flash")
                set_kind("static")
                set_languages("cxx17")
                add_headerfiles("include/**.h")
                add_files("src/**.cpp")
                add_includedirs("src", "include")
                add_deps("supercell_compression", "supercell_texture_loader", "lzma", "lzham")
                add_packages("zstd")
            target_end()
            
            local supercell_bytestream_dir = "dependencies/Bytestream"
            target("supercell_bytestream")
                set_kind("phony")
                add_headerfiles(supercell_bytestream_dir .. "/**.h")
                add_includedirs(supercell_bytestream_dir, {public = true})
            target_end()
            
            do -- supercell_compression
                local supercell_compression_dir = "dependencies/Compression"
                target("supercell_compression")
                    set_kind("static")
                    set_languages("cxx17")
                    add_files(supercell_compression_dir .. "/src/**.cpp")
                    add_headerfiles(supercell_compression_dir .. "/include/**.h", supercell_compression_dir .. "/src/**.h")
                    add_includedirs(supercell_compression_dir .. "/include", {public = true}) 
                    add_includedirs(supercell_compression_dir .. "/src")
                    add_deps("supercell_bytestream", "lzma", "lzham")
                    add_packages("zstd")
                target_end()
            
                local lzham_dir = supercell_compression_dir .. "/dependencies/lzham"
                target("lzham")
                    set_kind("static")
                    set_languages("cxx14")
                    add_files(
                        lzham_dir .. "/src/lzham_lib.cpp",
                        -- decompress
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
            
                        -- compress
                        lzham_dir .. "/src/lzhamcomp/lzham_lzbase.cpp", 
                        lzham_dir .. "/src/lzhamcomp/lzham_lzcomp.cpp",
                        lzham_dir .. "/src/lzhamcomp/lzham_lzcomp_internal.cpp",
                        lzham_dir .. "/src/lzhamcomp/lzham_lzcomp_state.cpp", 
                        lzham_dir .. "/src/lzhamcomp/lzham_match_accel.cpp"
                    )
                    add_headerfiles(
                        -- decompress
                        lzham_dir .. "/src/lzhamdecomp/lzham_assert.h",
                        lzham_dir .. "/src/lzhamdecomp/lzham_checksum.h", 
                        lzham_dir .. "/src/lzhamdecomp/lzham_config.h", 
                        lzham_dir .. "/src/lzhamdecomp/lzham_core.h",
                        lzham_dir .. "/src/lzhamdecomp/lzham_decomp.h", 
                        lzham_dir .. "/src/lzhamdecomp/lzham_helpers.h",
                        lzham_dir .. "/src/lzhamdecomp/lzham_huffman_codes.h",
                        lzham_dir .. "/src/lzhamdecomp/lzham_lzdecompbase.h", 
                        lzham_dir .. "/src/lzhamdecomp/lzham_math.h",
                        lzham_dir .. "/src/lzhamdecomp/lzham_mem.h",
                        lzham_dir .. "/src/lzhamdecomp/lzham_platform.h", 
                        lzham_dir .. "/src/lzhamdecomp/lzham_prefix_coding.h",
                        lzham_dir .. "/src/lzhamdecomp/lzham_symbol_codec.h",
                        lzham_dir .. "/src/lzhamdecomp/lzham_timer.h", 
                        lzham_dir .. "/src/lzhamdecomp/lzham_traits.h", 
                        lzham_dir .. "/src/lzhamdecomp/lzham_types.h",
                        lzham_dir .. "/src/lzhamdecomp/lzham_utils.h", 
                        lzham_dir .. "/src/lzhamdecomp/lzham_vector.h",
            
                        -- compress
                        lzham_dir .. "/src/lzhamcomp/lzham_lzbase.h",
                        lzham_dir .. "/src/lzhamcomp/lzham_lzcomp_internal.h",
                        lzham_dir .. "/src/lzhamcomp/lzham_match_accel.h",
                        lzham_dir .. "/src/lzhamcomp/lzham_null_threading.h", 
                        lzham_dir .. "/src/lzhamcomp/lzham_pthreads_threading.h", 
                        lzham_dir .. "/src/lzhamcomp/lzham_threading.h"
                    )
                    add_includedirs(lzham_dir .. "/include", lzham_dir .. "/src/lzhamcomp", lzham_dir .. "/src/lzhamdecomp", {public = true})
                    add_defines("_LIB")
            
                    if is_plat("windows") then
                        add_defines("WIN32")
                        add_headerfiles(lzham_dir .. "/src/lzhamcomp/lzham_win32_threading.h")
                        add_files(lzham_dir .. "/src/lzhamcomp/lzham_win32_threading.cpp")
                    else
                        add_links("pthread")
                        add_files(lzham_dir .. "/src/lzhamcomp/lzham_pthreads_threading.cpp")
                        add_cxflags("-fno-strict-aliasing", "-D_LARGEFILE64_SOURCE=1", "-D_FILE_OFFSET_BITS=64")
                    end
                target_end()
            
                local lzma_dir = supercell_compression_dir .. "/dependencies/lzma"
                target("lzma")
                    set_kind("static")
            
                    set_languages("c99")
            
                    add_headerfiles(lzma_dir .. "/include/**.h")
                    add_files(lzma_dir .. "/src/**.c")
            
                    add_includedirs(lzma_dir .. "/include", {public = true})
                target_end()
            end
            
            do -- supercell_texture_loader
                local supercell_texture_loader_dir = "dependencies/TextureLoader"
                target("supercell_texture_loader")
                    set_kind("static")
                    set_languages("cxx17")
                    add_headerfiles(supercell_texture_loader_dir .. "/include/**.h")
                    add_files(supercell_texture_loader_dir .. "/src/**.cpp")
                    add_includedirs(supercell_texture_loader_dir .. "/include", {public = true})
                    add_deps("supercell_bytestream", "etcpack", "astc", "basisu", "dfdutils", "libktx")
                target_end()
            
                local astc_dir = supercell_texture_loader_dir .. "/ThirdParty/astc-encoder"
                target("astc")
                    set_kind("static")
                    set_languages("cxx17")
                    add_headerfiles(astc_dir .. "/include/**.h", astc_dir .. "/src/**.h")
                    add_files(astc_dir .. "/src/**.cpp")
                    add_includedirs(astc_dir .. "/src")
                    add_includedirs(astc_dir .. "/include", {public = true})
                target_end()
            
                local basisu_dir = supercell_texture_loader_dir .. "/ThirdParty/basisu"
                target("basisu")
                    set_kind("static")
                    set_languages("cxx11")
                    add_files(basisu_dir .. "/src/**.cpp")
                    add_headerfiles(basisu_dir .. "/include/**.h", basisu_dir .. "/include/encoder/**.h", basisu_dir .. "/include/transcoder/**.h")
                    
                    add_includedirs(basisu_dir .. "/include", {public = true})
                    add_includedirs(basisu_dir .. "/include/encoder", {public = true})
                    add_includedirs(basisu_dir .. "/include/transcoder", {public = true})
                    add_packages("zstd")
                    
                    add_defines("BASISD_SUPPORT_KTX2_ZSTD")
                    if is_plat("windows") then
                        if is_arch("x86") then
                            add_defines("BASISU_SUPPORT_OPENCL", "BASISU_SUPPORT_SSE")
                            add_includedirs("OpenCL/CL")
                            add_cxflags("/d2archSSE42")
                            add_links(basisu_dir .. "/OpenCL/lib/OpenCL")
                        elseif is_arch("x86_64") then
                            add_defines("BASISU_SUPPORT_OPENCL", "BASISU_SUPPORT_SSE")
                            add_includedirs(basisu_dir .. "/OpenCL/CL")
                            add_links(basisu_dir .. "/OpenCL/lib/OpenCL64")
                        end
                    end
                target_end()
            
                local dfdutils_dir = supercell_texture_loader_dir .. "/ThirdParty/dfdutils"
                target("dfdutils")
                    set_kind("static")
                    set_languages("cxx11")
            
                    add_headerfiles(dfdutils_dir .. "/include/**.h")
                    add_files(dfdutils_dir .. "/src/**.c")
            
                    add_includedirs(dfdutils_dir .. "/include", {public = true})
                    
                    -- temp
                    add_includedirs(dfdutils_dir .. "/include/KHR", {public = true})
                    add_includedirs(dfdutils_dir .. "/include/vulkan", {public = true})
                target_end()
            
                local etcpack_dir = supercell_texture_loader_dir .. "/ThirdParty/ETCPACK"
                target("etcpack")
                    set_kind("static")
                    set_languages("cxx17")
                    add_files(etcpack_dir .. "/src/*.cxx")
                    add_headerfiles(etcpack_dir .. "/src/*.h")
                    add_includedirs(etcpack_dir .. "/src", {public = true})
                target_end()    
            
                local libktx_dir = supercell_texture_loader_dir .. "/ThirdParty/libktx"
                target("libktx")
                    set_kind("static")
                    set_languages("cxx11")
                    
                    add_files(libktx_dir .. "/src/*.cpp")
                    add_headerfiles(libktx_dir .. "/include/*.h")
            
                    add_includedirs(libktx_dir .. "/src")
                    add_includedirs(libktx_dir .. "/include", {public = true})
            
                    add_deps("astc", "basisu", "dfdutils")
                    add_packages("zstd")
            
                    add_defines("KHRONOS_STATIC", "LIBKTX")
                    add_defines("BASISU_SUPPORT_OPENCL=0")
                target_end()
            end
        ]])
        import("package.tools.xmake").install(package, configs)
        os.trycp("include/*", package:installdir("include"))
        os.trycp("dependencies/Bytestream/*", package:installdir("include"))
        os.trycp("dependencies/Compression/include/*", package:installdir("include"))
        os.trycp("dependencies/Compression/dependencies/lzham/include/*", package:installdir("include"))
        os.trycp("dependencies/Compression/dependencies/lzma/include/*", package:installdir("include"))
        os.trycp("dependencies/TextureLoader/include/*", package:installdir("include"))
        os.trycp("dependencies/TextureLoader/ThirdParty/astc-encoder/include/*", package:installdir("include"))
        os.trycp("dependencies/TextureLoader/ThirdParty/basisu/include/*", package:installdir("include"))
        os.trycp("dependencies/TextureLoader/ThirdParty/dfdutils/include/*", package:installdir("include"))
        os.trycp("dependencies/TextureLoader/ThirdParty/ETCPACK/src/*", package:installdir("include"))
        os.trycp("dependencies/TextureLoader/ThirdParty/libktx/include/*", package:installdir("include"))
    end)

    -- on_test(function (package)
    --     assert(package:check_cxxsnippets({test = [[
    --         void test() {
    --             SupercellSWF swf;
    --         }
    --     ]]}, {configs = {languages = "c++17"}, includes = {"SupercellFlash/SupercellSWF.h"}}))
    -- end)
package_end()
