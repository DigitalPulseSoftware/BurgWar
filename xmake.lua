add_repositories("burgwar-repo xmake-repo")

set_project("BurgWar")
set_version("0.1.0")

add_requires("concurrentqueue", "libcurl", "nlohmann_json")
add_requires("fmt", { debug = is_mode("debug"), config = { header_only = false, vs_runtime = "MD" } })

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

if (is_plat("windows")) then 
	add_cxflags(is_mode("debug") and "-MDd" or "-MD")
	add_cxxflags("/EHsc", "/bigobj")
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
	add_headerfiles("include/CoreLib/**.hpp")
	add_files("src/CoreLib/**.cpp")

if (is_plat("windows")) then 
	add_packages("stackwalker")
end

target("ClientLib")
	set_kind("static")

	add_deps("CoreLib")
	add_headerfiles("include/ClientLib/**.hpp")
	add_files("src/ClientLib/**.cpp")
	add_packages("libcurl", { public = true })

target("Main")
	set_kind("static")

	add_deps("CoreLib")
	add_headerfiles("include/Main/**.hpp")
	add_files("src/Main/**.cpp")

target("BurgWar")
	set_kind("binary")

	add_deps("Main", "ClientLib", "CoreLib")
	add_headerfiles("src/Client/**.hpp")
	add_files("src/Client/**.cpp")

target("BurgWarServer")
	set_kind("binary")

	add_deps("Main", "CoreLib")
	add_headerfiles("src/Server/**.hpp")
	add_files("src/Server/**.cpp")

target_end()
