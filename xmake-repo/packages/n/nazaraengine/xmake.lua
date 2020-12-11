package("nazaraengine")
    set_homepage("https://github.com/DigitalPulseSoftware/NazaraEngine")
    set_description("A fast, complete, cross-platform and object-oriented engine which can help you in your daily developper life")

    set_urls("https://github.com/DigitalPulseSoftware/NazaraEngine.git")

    add_versions("20201119", "e0b05a7e7a2779e20a593b4083b4a881cc57ce14")

    add_configs("audio",         {description = "Includes the audio module", default = true, type = "boolean"})
    add_configs("graphics",      {description = "Includes the graphics module", default = true, type = "boolean"})
    add_configs("lua",           {description = "Includes the lua module", default = true, type = "boolean"})
    add_configs("network",       {description = "Includes the network module", default = true, type = "boolean"})
    add_configs("physics2d",     {description = "Includes the 2D physics module", default = true, type = "boolean"})
    add_configs("physics3d",     {description = "Includes the 3D physics module", default = true, type = "boolean"})
    add_configs("platform",      {description = "Includes the platform module", default = true, type = "boolean"})
    add_configs("renderer",      {description = "Includes the renderer module", default = true, type = "boolean"})
    add_configs("noise",         {description = "Includes the noise module", default = true, type = "boolean"})
    add_configs("sdk",           {description = "Includes the SDK", default = true, type = "boolean"})
    add_configs("utility",       {description = "Includes the utility module", default = true, type = "boolean"})
    add_configs("server",        {description = "Only includes server modules (and use SDK in server mode)", default = false, type = "boolean"})
    add_configs("plugin-assimp", {description = "Includes the assimp plugin", default = false, type = "boolean"})

    if is_plat("linux") then
        add_syslinks("pthread")
    end

    local function has_sdk(package)
        return package:config("sdk")
    end

    local function has_audio(package)
        return not package:config("server") and (package:config("audio") or has_sdk(package))
    end

    local function has_graphics(package)
        return not package:config("server") and (package:config("graphics") or has_sdk(package))
    end

    local function has_lua(package)
        return package:config("lua") or has_sdk(package)
    end

    local function has_network(package)
        return package:config("network")
    end

    local function has_noise(package)
        return package:config("noise")
    end

    local function has_renderer(package)
        return not package:config("server") and (package:config("renderer") or has_graphics(package))
    end

    local function has_platform(package)
        return not package:config("server") and (package:config("platform") or has_renderer(package))
    end

    local function has_physics2d(package)
        return package:config("physics2d") or has_sdk(package)
    end

    local function has_physics3d(package)
        return package:config("physics3d") or has_sdk(package)
    end

    local function has_utility(package)
        return package:config("utility") or has_platform(package)
    end

    local function has_assimp_plugin(package)
        return package:config("plugin-assimp")
    end

    on_load("windows", "linux", "macosx", "mingw", function (package)
        local prefix = "Nazara"
        local suffix = package:config("shared") and "" or "-s"

        if package:debug() then
            suffix = suffix .. "-d"
        end

        if not package:config("shared") then
            package:add("defines", "NAZARA_STATIC")
        end

        if has_sdk(package) then
            if package:config("server") then
                package:add("links", prefix .. "SDKServer" .. suffix)
            else
                package:add("links", prefix .. "SDK" .. suffix)
            end
        end

        if has_audio(package) then
            package:add("links", prefix .. "Audio" .. suffix)
        end

        if has_lua(package) then
            --package:add("deps", "lua")
            package:add("links", prefix .. "Lua" .. suffix)
        end

        if has_network(package) then
            package:add("links", prefix .. "Network" .. suffix)
        end

        if has_noise(package) then
            package:add("links", prefix .. "Noise" .. suffix)
        end

        if has_physics2d(package) then
            package:add("links", prefix .. "Physics2D" .. suffix)
        end

        if has_physics3d(package) then
            package:add("links", prefix .. "Physics3D" .. suffix)
        end

        if has_graphics(package) then
            package:add("links", prefix .. "Graphics" .. suffix)
        end

        if has_renderer(package) then
            package:add("links", prefix .. "Renderer" .. suffix)
            if package:is_plat("windows", "mingw") then
                package:add("syslinks", "gdi32", "user32", "advapi32")
            end
        end

        if has_platform(package) then
            --if package:is_plat("linux") then
            --    package:add("deps", "libsdl")
            --end

            package:add("links", prefix .. "Platform" .. suffix)
        end

        if has_utility(package) then
            --if package:is_plat("linux") then
            --    package:add("deps", "freetype")
            --end

            package:add("links", prefix .. "Utility" .. suffix)
        end

        if has_assimp_plugin(package) then
            if package:is_plat("linux") then
                package:add("deps", "assimp")
            end
        end

        package:add("links", prefix .. "Core" .. suffix)
    end)

    on_install("windows", "linux", function (package)
        -- Remove potential leftovers from previous build
        os.rm("lib")

        local premakeOptions = {"--verbose", "--excludes-examples"}
        if not has_audio(package) then
            table.insert(premakeOptions, "--excludes-module-audio")
        end

        if not has_graphics(package) then
            table.insert(premakeOptions, "--excludes-module-graphics")
        end

        if not has_lua(package) then
            table.insert(premakeOptions, "--excludes-externlib-lua")
            table.insert(premakeOptions, "--excludes-module-lua")
        end

        if not has_network(package) then
            table.insert(premakeOptions, "--excludes-module-network")
        end

        if not has_noise(package) then
            table.insert(premakeOptions, "--excludes-module-noise")
        end

        if not has_platform(package) then
            table.insert(premakeOptions, "--excludes-module-platform")
        end

        if not has_physics2d(package) then
            table.insert(premakeOptions, "--excludes-externlib-chipmunk")
            table.insert(premakeOptions, "--excludes-module-physics2d")
        end

        if not has_physics3d(package) then
            table.insert(premakeOptions, "--excludes-externlib-newton")
            table.insert(premakeOptions, "--excludes-module-physics3d")
        end

        if not has_renderer(package) then
            table.insert(premakeOptions, "--excludes-module-renderer")
        end

        if not has_utility(package) then
            table.insert(premakeOptions, "--excludes-externlib-stb_image")
            table.insert(premakeOptions, "--excludes-module-utility")
        end

        if not has_assimp_plugin(package) then
            table.insert(premakeOptions, "--excludes-tool-assimp")
        end

        if not has_sdk(package) or package:config("server") then
            table.insert(premakeOptions, "--excludes-tool-sdk")
        end

        if not has_sdk(package) or not package:config("server") then
            table.insert(premakeOptions, "--excludes-tool-sdkserver")
        end

        local archName = {
            x86 = "x86",
            x86_64 = "x64",
            x64 = "x64"
        }

        local premakeArch = assert(archName[package:arch()])
        local libDir

        os.cd("build")
        if package:is_plat("windows") then
            os.vrun("./premake5.exe " .. table.concat(premakeOptions, " ") .. " vs2019")
            os.cd("vs2019")

            local configs = {}
            local arch = package:is_arch("x86") and "Win32" or "x64"
            local mode = (package:debug() and "Debug" or "Release") .. (package:config("shared") and "Dynamic" or "Static")

            table.insert(configs, "/property:Configuration=" .. mode)
            table.insert(configs, "/property:Platform=" .. arch)

            import("package.tools.msbuild").build(package, configs)

            libDir = "msvc"
        elseif package:is_plat("linux") then
            os.vrun("./premake5-linux64 " .. table.concat(premakeOptions, " ") .. " gmake2")
            os.cd("gmake2")
            
            local configName = (package:debug() and "debug" or "release") .. (package:config("shared") and "dynamic" or "static") .. "_" .. premakeArch

            os.vrun("make config=" .. configName .. " -j4")

            libDir = "gmake"
        end

        os.cd("../../")
        os.cp("include/Nazara", package:installdir("include"))
        os.cp("SDK/include/NDK", package:installdir("include"))
        os.cp("lib/" .. libDir .. "/" .. premakeArch .. "/*", package:installdir("lib"))

        if package:is_plat("windows") then
            if has_audio(package) then
                os.cp("thirdparty/lib/common/" .. premakeArch .. "/soft_oal.dll", package:installdir("lib"))
            end

            if has_platform(package) then
                os.cp("thirdparty/lib/common/" .. premakeArch .. "/SDL2.dll", package:installdir("lib"))
            end

            if has_utility(package) then
                os.cp("thirdparty/lib/common/" .. premakeArch .. "/libsndfile-1.dll", package:installdir("lib"))
            end

            if has_assimp_plugin(package) then
                os.cp("thirdparty/lib/common/" .. premakeArch .. "/assimp.dll", package:installdir("lib"))
            end
        end
    end)

    on_test(function (package)
        print(package:deps())
        assert(package:check_cxxsnippets({test = [[
            void test(int args, char** argv) {
                Nz::Clock c;
                c.Restart();
            }
        ]]}, {includes = "Nazara/Core.hpp"}))
    end)
