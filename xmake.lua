add_repositories("burgwar-repo xmake-repo")

set_project("BurgWar")
set_version("0.1.0")

add_requires("concurrentqueue", "nlohmann_json")
add_requires("fmt", { debug = is_mode("debug"), config = { header_only = false, vs_runtime = "MD" } })
add_requires("libcurl", { debug = is_mode("debug"), config = { vs_runtime = "MD" } })
add_requires("nazaraengine", { alias = "nazara", debug = is_mode("debug"), config = { server = false, shared = true, vs_runtime = "MD" } })
add_requires("nazaraengine~server", { alias = "nazaraserver", debug = is_mode("debug"), config = { server = true, shared = true, vs_runtime = "MD" } })

if (is_plat("windows")) then
	add_requires("stackwalker")
end

add_rules("mode.debug", "mode.release")

add_includedirs("include", "src")
add_includedirs("thirdparty/include")
set_languages("c89", "cxx17")

set_symbols("debug", "hidden")
set_warnings("allextra")
set_targetdir("./bin/$(os)_$(arch)_$(mode)")
set_rundir("./bin/$(os)_$(arch)_$(mode)")

if (is_mode("release")) then
	set_fpmodels("fast")
	set_optimize("fastest")
	add_vectorexts("sse", "sse2", "sse3", "ssse3")
end

if (is_plat("windows")) then
	add_cxflags(is_mode("debug") and "/MDd" or "/MD")
	add_cxxflags("/bigobj", "/ZI", "/Zc:__cplusplus", "/Zc:referenceBinding", "/Zc:throwingNew")
	add_cxxflags("/FC")
elseif is_plat("linux") then
	add_syslinks("pthread")
end


--[[
target("autoupdate")
    set_kind("binary")
    on_build(function (target)
        if os.getenv("XMAKE_IN_VSTUDIO") then
            os.execv("xmake", {"project", "-k", "vsxmake"}, {detach = true, envs = {XMAKE_CONFIGDIR = os.tmpfile() .. ".xmake"}})
        end
    end)
]]

target("lua")
	set_kind("static")

	add_includedirs("contrib/lua/include", { public = true })
	add_headerfiles("contrib/lua/include/**.h")
	add_files("contrib/lua/src/**.c")

target("CoreLib")
	set_kind("static")

	add_deps("lua")
	add_headerfiles("include/CoreLib/**.hpp", "include/CoreLib/**.inl")
	add_files("src/CoreLib/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json")
	add_packages("nazara", {links = {}})

if (is_plat("windows")) then 
	add_packages("stackwalker")
end

target("ClientLib")
	set_kind("static")

	add_deps("CoreLib")
	add_headerfiles("include/ClientLib/**.hpp", "include/ClientLib/**.inl")
	add_headerfiles("src/ClientLib/**.hpp", "src/ClientLib/**.inl")
	add_files("src/ClientLib/**.cpp")
	add_packages("libcurl", { public = true })
	add_packages("concurrentqueue", "fmt", "nlohmann_json", "nazara")

target("Main")
	set_kind("static")

	add_deps("CoreLib")
	add_headerfiles("include/Main/**.hpp", "include/Main/**.inl")
	add_headerfiles("src/Main/**.hpp", "src/Main/**.inl")
	add_files("src/Main/**.cpp")

target("BurgWar")
	set_kind("binary")

	add_deps("Main", "ClientLib", "CoreLib")
	add_headerfiles("src/Client/**.hpp", "src/Client/**.inl")
	add_files("src/Client/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json", "nazara")

target("BurgWarServer")
	set_kind("binary")

	add_defines("NDK_SERVER")

	add_deps("Main", "CoreLib")
	add_headerfiles("src/Server/**.hpp", "src/Server/**.inl")
	add_files("src/Server/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json", "nazaraserver")

target("BurgWarMapEditor")
	set_kind("binary")
	add_rules("qt.console", "qt.moc")

	add_frameworks("QtCore", "QtGui", "QtWidgets")
	add_deps("Main", "ClientLib", "CoreLib")
	add_headerfiles("src/MapEditor/**.hpp", "src/MapEditor/**.inl")
	add_files("src/MapEditor/Widgets/**.hpp", "src/MapEditor/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json", "nazara")

target_end()
