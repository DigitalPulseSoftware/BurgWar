package("nazaraengine")
    set_homepage("https://github.com/NazaraEngine/NazaraEngine")
    set_description("Nazara Engine is a cross-platform framework aimed at (but not limited to) real-time applications requiring audio, 2D and 3D rendering, network and more (such as video games).")
    set_license("MIT")
    set_policy("package.librarydeps.strict_compatibility", true)

    set_urls("https://github.com/NazaraEngine/NazaraEngine.git")

    add_versions("2022.11.05", "45d4195527d6f2a79d3230a08bf1269cf254f9b9")

    add_deps("nazarautils")
    add_deps("chipmunk2d", "dr_wav", "efsw", "fmt", "frozen", "kiwisolver", "libflac", "libsdl", "minimp3", "ordered_map", "stb", { private = true })
    add_deps("libvorbis", { private = true, configs = { with_vorbisenc = false } })
    add_deps("openal-soft", { private = true, configs = { shared = true }})

    -- static compilation is not supported for now
    add_configs("shared", {description = "Build shared library.", default = true, type = "boolean", readonly = true})

    -- all modules have their own config
    add_configs("plugin_assimp", {description = "Includes the assimp plugin", default = false, type = "boolean"})
    add_configs("plugin_ffmpeg", {description = "Includes the ffmpeg plugin", default = false, type = "boolean"})
    add_configs("entt",          {description = "Includes EnTT to use components and systems", default = true, type = "boolean"})
    add_configs("with_symbols",  {description = "Enable debug symbols in release", default = false, type = "boolean"})

    local components = {
        { 
            name = "Audio",
            deps = { "core" }
        },
        {
            name = "Core",
            custom = function (package, component)
                if package:is_plat("linux") then
                    component:add("syslinks", "pthread", "dl")
                end
            end
        },
        { 
            name = "Graphics",
            deps = { "renderer" }
        },
        { 
            name = "Network",
            deps = { "core" }
        },
        { 
            name = "Physics2D",
            deps = { "core" }
        },
        { 
            name = "Physics3D",
            deps = { "core" }
        },
        { 
            name = "Platform",
            deps = { "utility" }
        },
        { 
            name = "Renderer",
            deps = { "platform", "utility" },
            custom = function (package, component)
                if package:is_plat("windows", "mingw") then
                    component:add("syslinks", "gdi32", "user32", "advapi32")
                end
            end
        },
        { 
            name = "Utility",
            deps = { "core" }
        },
        { 
            name = "Widgets",
            deps = { "graphics" }
        },
    }

    for _, comp in ipairs(components) do
        local componentName = comp.name:lower()
        add_configs(componentName, { description = "Includes the " .. comp.name .. " module", default = true, type = "boolean" })

        on_component(componentName, function (package, component)
            local prefix = "Nazara"
            local suffix = package:config("shared") and "" or "-s"
            if package:debug() then
                suffix = suffix .. "-d"
            end

            component:add("deps", table.unwrap(comp.deps))
            component:add("links", prefix .. comp.name .. suffix)
            if comp.custom then
                comp.custom(package, component)
            end
        end)
    end

    on_load(function (package)
        for _, comp in ipairs(components) do
            local componentName = comp.name:lower()
            if package:config(componentName) then
                package:add("components", componentName)
            end
        end

        if not package:config("shared") then
            package:add("defines", "NAZARA_STATIC")
        end

        package:add("deps", "nzsl", { debug = package:debug(), configs = { with_symbols = package:config("with_symbols") or package:debug(), shared = true } })
        package:add("deps", "freetype", { private = true, configs = { bzip2 = true, png = true, woff2 = true, zlib = true, debug = package:debug() } })
        package:add("deps", "newtondynamics3", { private = true, debug = is_plat("windows") and package:debug() })
        if package:config("entt") then
            package:add("deps", "entt 3.10.1")
        end
        if package:config("plugin_assimp") then
            package:add("deps", "assimp v5.2.3", { private = true })
        end
        if package:config("plugin_ffmpeg") then
            package:add("deps", "ffmpeg", { private = true, configs = { shared = true }})
        end
    end)

    on_install("windows", "mingw", "linux", "macosx", function (package)
        local configs = {}
        configs.assimp = package:config("plugin_assimp")
        configs.ffmpeg = package:config("plugin_ffmpeg")
        configs.examples = false
        configs.override_runtime = false

        if not package:config("shared") then
            configs.embed_rendererbackends = true
        end

        if package:is_debug() then
            configs.mode = "debug"
        elseif package:config("with_symbols") then
            configs.mode = "releasedbg"
        else
            configs.mode = "release"
        end
        import("package.tools.xmake").install(package, configs)
    end)

    on_test(function (package)
        for _, comp in ipairs(components) do
            if package:config(comp.name:lower()) then
                assert(package:check_cxxsnippets({test = [[
                    void test() {
                        Nz::Modules<Nz::]] .. comp.name .. [[> nazara;
                    }
                ]]}, {configs = {languages = "c++17"}, includes = "Nazara/" .. comp.name .. ".hpp"}))
            end
        end
    end)
