package("nazaraengine")
    set_homepage("https://github.com/DigitalPulseSoftware/NazaraEngine")
    set_description("A fast, complete, cross-platform and object-oriented engine which can help you in your daily developper life")

    set_urls("https://github.com/DigitalPulseSoftware/NazaraEngine.git")

    add_versions("2021.08.28", "8d7e99456ce5e76993cd2037a017018f982e2206")
    add_versions("2021.06.10", "4989fbe2d28aa37dc11d24bdcc395431932a2f11")
    add_versions("2021.04.01", "aef225d8784396035e1f92a029a685e654320e73")

    add_configs("audio",         {description = "Includes the audio module", default = true, type = "boolean"})
    add_configs("graphics",      {description = "Includes the graphics module", default = true, type = "boolean"})
    add_configs("network",       {description = "Includes the network module", default = true, type = "boolean"})
    add_configs("physics2d",     {description = "Includes the 2D physics module", default = true, type = "boolean"})
    add_configs("physics3d",     {description = "Includes the 3D physics module", default = true, type = "boolean"})
    add_configs("platform",      {description = "Includes the platform module", default = true, type = "boolean"})
    add_configs("renderer",      {description = "Includes the renderer module", default = true, type = "boolean"})
    add_configs("utility",       {description = "Includes the utility module", default = true, type = "boolean"})
    add_configs("plugin-assimp", {description = "Includes the assimp plugin", default = false, type = "boolean"})

    if is_plat("linux") then
        add_syslinks("pthread")
    end

    local function has_audio(package)
        return not package:config("server") and (package:config("audio"))
    end

    local function has_graphics(package)
        return not package:config("server") and (package:config("graphics"))
    end

    local function has_network(package)
        return package:config("network")
    end

    local function has_renderer(package)
        return not package:config("server") and (package:config("renderer") or has_graphics(package))
    end

    local function has_platform(package)
        return not package:config("server") and (package:config("platform") or has_renderer(package))
    end

    local function has_physics2d(package)
        return package:config("physics2d")
    end

    local function has_physics3d(package)
        return package:config("physics3d")
    end

    local function has_utility(package)
        return package:config("utility") or has_platform(package)
    end

    local function has_assimp_plugin(package)
        return package:config("plugin-assimp")
    end

    on_load("windows", "linux", "macosx", "mingw", function (package)
        local nazaradir = os.getenv("NAZARA_ENGINE_PATH") or "C:/Projets/Perso/NazaraNext/NazaraEngine"
        if not nazaradir then 
            raise("missing NAZARA_ENGINE_PATH")
        end
        package:set("installdir", nazaradir)
    end)

    on_fetch(function (package)
        local defines = {}
        local includedirs = package:installdir("include")
        local links = {}
        local libprefix = package:debug() and "debug" or "releasedbg"
        local linkdirs = package:installdir("bin/" .. package:plat() .. "_" .. package:arch() .. "_" .. libprefix)
        local syslinks = {}

        local prefix = "Nazara"
        local suffix = package:config("shared") and "" or "-s"

        if package:debug() then
            suffix = suffix .. "-d"
        end

        if not package:config("shared") then
            table.insert(defines, "NAZARA_STATIC")
        end

        if has_audio(package) then
            table.insert(links, prefix .. "Audio" .. suffix)
        end

        if has_network(package) then
            table.insert(links, prefix .. "Network" .. suffix)
        end

        if has_physics2d(package) then
            table.insert(links, prefix .. "Physics2D" .. suffix)
        end

        if has_physics3d(package) then
            table.insert(links, prefix .. "Physics3D" .. suffix)
        end

        if has_graphics(package) then
            table.insert(links, prefix .. "Graphics" .. suffix)
        end

        if has_renderer(package) then
            table.insert(links, prefix .. "Renderer" .. suffix)
            if package:is_plat("windows", "mingw") then
                table.insert(syslinks, "gdi32")
                table.insert(syslinks, "user32")
                table.insert(syslinks, "advapi32")
            end
        end

        if has_platform(package) then
            table.insert(links, prefix .. "Platform" .. suffix)
        end

        if has_utility(package) then
            table.insert(links, prefix .. "Utility" .. suffix)
        end

        table.insert(links, prefix .. "Core" .. suffix)

        return {
            defines = defines,
            includedirs = includedirs,
            links = links,
            linkdirs = linkdirs,
            syslinks = syslinks
        }
    end)
