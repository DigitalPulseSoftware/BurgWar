package("nazaraengine")
    set_homepage("https://github.com/NazaraEngine/NazaraEngine")
    set_description("A fast, complete, cross-platform and object-oriented engine which can help you in your daily developper life")

    set_urls("https://github.com/NazaraEngine/NazaraEngine.git")

    add_versions("2022.11.23", "040e49c1cd462ca22e1673171d80db6b0f18bd68")

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
    add_configs("server",        {description = "Only includes server modules (takes priority over other options)", default = false, type = "boolean"})
    add_configs("utility",       {description = "Includes the utility module", default = true, type = "boolean"})
    add_configs("clientsdk",     {description = "Includes the Client SDK", default = true, type = "boolean"})
    add_configs("plugin-assimp", {description = "Includes the assimp plugin", default = false, type = "boolean"})

    add_deps("premake5")

    if is_plat("linux") then
        add_syslinks("pthread")
    end

    local function has_clientsdk(package)
        return not package:config("server") or package:config("sdk")
    end

    local function has_sdk(package)
        return package:config("sdk") or has_clientsdk(package)
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

        if has_clientsdk(package) then
            package:add("links", prefix .. "ClientSDK" .. suffix)
        end

        if has_sdk(package) then
            package:add("links", prefix .. "SDK" .. suffix)
        end

        if has_audio(package) then
            package:add("deps", "openal-soft", "libsndfile", { configs = { shared = true }})
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
            package:add("deps", "libsdl")

            package:add("links", prefix .. "Platform" .. suffix)
        end

        if has_utility(package) then
            package:add("deps", "freetype", "zlib")

            package:add("links", prefix .. "Utility" .. suffix)
        end

        if has_assimp_plugin(package) then
            package:add("deps", "assimp")
        end

        package:add("links", prefix .. "Core" .. suffix)
    end)

    on_install("windows", "linux", "mingw", function (package)
        -- Remove potential leftovers from previous build
        os.rm("lib")

        local premakeOptions = {"--verbose", "--excludes-examples", " --excludes-tests"}
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

        if not has_sdk(package) then
            table.insert(premakeOptions, "--excludes-tool-sdk")
        end

        if not has_clientsdk(package) then
            table.insert(premakeOptions, "--excludes-tool-clientsdk")
        end

        local archName = {
            x86 = "x86",
            x86_64 = "x64",
            x64 = "x64"
        }

        local premakeArch = assert(archName[package:arch()])
        local libDir

        local includedirs, linkdir
        for _, depname in ipairs({ "assimp", "freetype", "zlib", "libsndfile", "libsdl" }) do
            local dep = package:dep(depname)
            if dep then
                local fetchinfo = dep:fetch({external = false})
                if fetchinfo then
                    includedirs = table.join(includedirs or {}, fetchinfo.includedirs)
                    includedirs = table.join(includedirs or {}, fetchinfo.sysincludedirs)
                    linkdir = table.join(linkdir or {}, fetchinfo.linkdirs)
                end
            end
        end

        -- patch Newton MinGW defines check
        io.replace("thirdparty/src/newton/dgCore/dgTypes.cpp", [[#if (defined (_MSC_VER) || defined (_MINGW_32_VER) || defined (_MINGW_64_VER))]], [[#if (defined (_MSC_VER) || defined (__MINGW32__) || defined (__MINGW64__))]], {plain=true})

        os.cd("build")

        local conf_file = io.open("config.lua", "a")
        if includedirs then
            conf_file:print("ExtlibIncludeDirs = {[[%s]]}", table.concat(includedirs, "]],[["))
        end
        if linkdir then
            conf_file:print("ExtlibLinkDirs = {[[%s]]}", table.concat(linkdir, "]],[["))
        end
        conf_file:close()

        if package:is_plat("windows") then
            os.vrun("premake5 " .. table.concat(premakeOptions, " ") .. " vs2022")
            os.cd("vs2022")

            local configs = {}
            local arch = package:is_arch("x86") and "Win32" or "x64"
            local mode = (package:debug() and "Debug" or "Release") .. (package:config("shared") and "Dynamic" or "Static")

            table.insert(configs, "/property:Configuration=" .. mode)
            table.insert(configs, "/property:Platform=" .. arch)

            import("package.tools.msbuild").build(package, configs)

            libDir = "msvc"
        elseif package:is_plat("linux", "mingw") then
            os.vrun("premake5 " .. table.concat(premakeOptions, " ") .. " gmake2")
            os.cd("gmake2")

            local configs = {}
            table.insert(configs, "config=" .. (package:debug() and "debug" or "release") .. (package:config("shared") and "dynamic" or "static") .. "_" .. premakeArch)

             -- mingw-make doesn't seem to like parallel building (or maybe it's premake?)
            if package:is_plat("mingw") then
                local opt = package:is_plat("mingw") and {jobs=1} or nil
                import("package.tools.make").build(package, table.join(configs, {"chipmunk"}), opt)
            end

            import("package.tools.make").build(package, configs, opt)

            libDir = package:is_plat("mingw") and "mingw" or "gmake"
        else
            os.raise("unexpected platform")
        end

        os.cd("../../")
        os.cp("include/Nazara", package:installdir("include"))
        os.cp("SDK/include/NDK", package:installdir("include"))

        if package:is_plat("windows") then
            os.cp("lib/" .. libDir .. "/" .. premakeArch .. "/*.dll", package:installdir("bin"))
            os.cp("lib/" .. libDir .. "/" .. premakeArch .. "/*.lib", package:installdir("lib"))
        else
            os.cp("lib/" .. libDir .. "/" .. premakeArch .. "/*", package:installdir("lib"))
        end
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            void test(int args, char** argv) {
                Nz::Clock c;
                c.Restart();
            }
        ]]}, {includes = "Nazara/Core.hpp"}))
    end)
