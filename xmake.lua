add_repositories("burgwar-repo xmake-repo")

set_project("BurgWar")
set_version("0.1.0")

add_requires("concurrentqueue", "nlohmann_json")
add_requires("fmt", { debug = is_mode("debug"), config = { header_only = false, vs_runtime = "MD" } })
add_requires("libcurl", { debug = is_mode("debug"), config = { vs_runtime = "MD" } })

if (is_plat("windows")) then
	add_requires("stackwalker")
end

add_rules("mode.debug", "mode.release")

--FIXME: Use packages
add_includedirs("../NazaraEngine/package/include")
add_linkdirs("../NazaraEngine/package/lib/x64/")

add_includedirs("include", "src")
add_includedirs("thirdparty/include")
set_languages("c89", "cxx17")

add_packages("concurrentqueue", "fmt", "nlohmann_json")
set_symbols("debug", "hidden")
set_warnings("allextra")

if (is_mode("release")) then
	set_fpmodels("fast")
    set_optimize("fastest")
	add_vectorexts("sse", "sse2", "sse3", "ssse3")
end

if (is_plat("windows")) then
	add_cxflags(is_mode("debug") and "/MDd" or "/MD")
	add_cxxflags("/bigobj", "/ZI", "/Zc:__cplusplus", "/Zc:referenceBinding", "/Zc:throwingNew")
	add_cxxflags("/FC")
end

target("lua")
	set_kind("static")

	add_includedirs("contrib/lua/include", { public = true })
	add_headerfiles("contrib/lua/include/**.h")
	add_files("contrib/lua/src/**.c")


target("CoreLib")
	set_kind("static")

	add_deps("lua")
	add_links("NazaraAudio", "NazaraCore", "NazaraLua", "NazaraGraphics", "NazaraNetwork", "NazaraNoise", "NazaraRenderer", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraPlatform", "NazaraSDK", "NazaraUtility")
	add_headerfiles("include/CoreLib/**.hpp", "include/CoreLib/**.inl")
	add_files("src/CoreLib/**.cpp")

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

target("BurgWarServer")
	set_kind("binary")

	add_deps("Main", "CoreLib")
	add_headerfiles("src/Server/**.hpp", "src/Server/**.inl")
	add_files("src/Server/**.cpp")

target("BurgWarMapEditor")
	set_kind("binary")
	add_rules("qt.console", "qt.moc")

	add_frameworks("QtCore", "QtGui", "QtWidgets")
	add_deps("Main", "ClientLib", "CoreLib")
	add_headerfiles("src/MapEditor/**.hpp", "src/MapEditor/**.inl")
	add_files("src/MapEditor/Widgets/**.hpp", "src/MapEditor/**.cpp")

target_end()
