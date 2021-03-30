add_repositories("burgwar-repo xmake-repo")

set_project("BurgWar")
set_version("0.2.0")

add_requires("cxxopts", "concurrentqueue", "libcurl", "nlohmann_json")
add_requires("fmt", { configs = { header_only = false } })
add_requires("nazaraengine 2021.03.30", { alias = "nazara", configs = { server = false } })
add_requires("nazaraengine~server 2021.03.30", { alias = "nazaraserver", configs = { server = true } })

if (is_plat("windows") and not is_arch("x86")) then
	add_requires("stackwalker")
end

add_requireconfs("fmt", "stackwalker", { debug = is_mode("debug") })
add_requireconfs("libcurl", "nazaraengine", "nazaraengine~server", { configs = { debug = is_mode("debug"), shared = true } })

add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

add_includedirs("include", "src")
add_includedirs("thirdparty/include")

set_languages("c89", "cxx17")
set_rundir("./bin/$(os)_$(arch)_$(mode)")
set_runtimes(is_mode("releasedbg") and "MD" or "MDd")
set_symbols("debug", "hidden")
set_targetdir("./bin/$(os)_$(arch)_$(mode)")
set_warnings("allextra")

if (is_mode("releasedbg")) then
	set_fpmodels("fast")
	add_vectorexts("sse", "sse2", "sse3", "ssse3")
end

if (is_plat("windows")) then
	add_cxxflags("/bigobj", "/Zc:__cplusplus", "/Zc:referenceBinding", "/Zc:throwingNew")
	add_cxxflags("/FC")
	add_cxflags("/w44062") -- Switch case not handled warning
elseif is_plat("linux") then
	add_syslinks("pthread")
end


rule("copy_symbolfile")
	after_install(function(target)
		local symbolfile = target:symbolfile()
		if os.isfile(symbolfile) then
			os.vcp(symbolfile, path.join(target:installdir(), "bin"))
		end
	end)
rule_end()

rule("install_metadata")
	local metadataInstalled = false
	after_install(function(target)
		if (not scriptsInstalled) then
			os.vcp("maps", path.join(target:installdir(), "bin"))
			os.vcp("scripts", path.join(target:installdir(), "bin"))
			metadataInstalled = true
		end
	end)
rule_end()

rule("install_nazara")
	-- This is already handled by xmake on Windows
	after_install("linux", function (target)
		local outputdir = path.join(target:installdir(), "bin")
		local nazara = target:pkg("nazara") or target:pkg("nazaraserver")
		if (nazara) then
			local libfiles = nazara:get("libfiles")
			if (libfiles) then
				for _, libpath in ipairs(table.wrap(libfiles)) do
					if (libpath:endswith(".so")) then
						os.vcp(libpath, outputdir)
					end
				end
			end
		end
	end)
rule_end()


target("lua")
	set_kind("static")
	set_group("3rdparties")

	add_includedirs("contrib/lua/include", { public = true })
	add_headerfiles("contrib/lua/include/**.h")
	add_files("contrib/lua/src/**.c")

target("CoreLib")
	set_kind("static")
	set_group("Common")

	add_deps("lua")
	add_headerfiles("include/CoreLib/**.hpp", "include/CoreLib/**.inl")
	add_headerfiles("src/CoreLib/**.hpp", "src/CoreLib/**.inl")
	add_files("src/CoreLib/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json")
	add_packages("nazaraserver", {links = {}})

if (is_plat("windows") and not is_arch("x86")) then
	add_packages("stackwalker")
end

	before_build(function (target)
		local host = os.host()
		local subhost = os.subhost()

		local system
		if (host ~= subhost) then
			system = host .. "/" .. subhost
		else
			system = host
		end

		local branch = "unknown-branch"
		local commitHash = "unknown-commit"
		try
		{
			function ()
				import("detect.tools.find_git")
				local git = find_git()
				if (git) then
					branch = os.iorunv(git, {"rev-parse", "--abbrev-ref", "HEAD"}):trim()
					commitHash = os.iorunv(git, {"describe", "--tags", "--long"}):trim()
				else
					error("git not found")
				end
			end,

			catch
			{
				function (err)
					print(string.format("Failed to retrieve git data: %s", err))
				end
			}
		}

		import("core.project.depend")
		import("core.project.project")
		local tmpfile = path.join(os.projectdir(), "project.autoversion")
		local dependfile = tmpfile .. ".d"
		depend.on_changed(function ()
			print("regenerating version data info...")
			io.writefile("src/CoreLib/VersionData.hpp", string.format([[
const char* BuildSystem = "%s";
const char* BuildBranch = "%s";
const char* BuildCommit = "%s";
const char* BuildDate = "%s";
]], system, branch, commitHash, os.date("%Y-%m-%d %H:%M:%S")))
		end, 
		{
			dependfile = dependfile, 
			files = project.allfiles(), 
			values = {system, branch, commitHash}
		})
	end)

target("ClientLib")
	set_kind("static")
	set_group("Common")

	add_deps("CoreLib")
	add_headerfiles("include/ClientLib/**.hpp", "include/ClientLib/**.inl")
	add_headerfiles("src/ClientLib/**.hpp", "src/ClientLib/**.inl")
	add_files("src/ClientLib/**.cpp")
	add_packages("libcurl")
	add_packages("concurrentqueue", "fmt", "nlohmann_json", "nazara")

target("Main")
	set_kind("static")
	set_group("Common")

	add_deps("CoreLib")
	add_headerfiles("include/Main/**.hpp", "include/Main/**.inl")
	add_headerfiles("src/Main/**.hpp", "src/Main/**.inl")
	add_files("src/Main/**.cpp")

target("BurgWar")
	set_kind("binary")
	set_group("Executable")
	add_rules("copy_symbolfile", "install_metadata", "install_nazara")

	add_deps("Main", "ClientLib", "CoreLib")
	add_headerfiles("src/Client/**.hpp", "src/Client/**.inl")
	add_files("src/Client/**.cpp")
	add_packages("concurrentqueue", "fmt", "libcurl", "nlohmann_json", "nazara")
	add_rpathdirs("@executable_path")

	after_install(function (target)
		os.vcp("clientconfig.lua", path.join(target:installdir(), "bin"))
	end)

target("BurgWarServer")
	set_kind("binary")
	set_group("Executable")
	add_rules("copy_symbolfile", "install_metadata", "install_nazara")

	add_defines("NDK_SERVER")

	add_deps("Main", "CoreLib")
	add_headerfiles("src/Server/**.hpp", "src/Server/**.inl")
	add_files("src/Server/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json", "nazaraserver")
	add_rpathdirs("@executable_path")

	after_install(function (target)
		os.vcp("serverconfig.lua", path.join(target:installdir(), "bin"))
	end)

target("BurgWarMapTool")
	set_basename("maptool")
	set_kind("binary")
	set_group("Executable")
	add_rules("copy_symbolfile", "install_nazara")

	add_deps("Main", "CoreLib")
	add_headerfiles("src/MapTool/**.hpp", "src/MapTool/**.inl")
	add_files("src/MapTool/**.cpp")
	add_packages("cxxopts", "concurrentqueue", "fmt", "nlohmann_json", "nazaraserver")
	add_rpathdirs("@executable_path")

target("BurgWarMapEditor")
	set_kind("binary")
	set_group("Executable")
	add_rules("qt.console", "qt.moc")
	add_rules("copy_symbolfile", "install_metadata", "install_nazara")

	-- Prevents symbol finding issues between Qt5 compiled with C++ >= 14 and Qt5 compiled with C++11
	-- see https://stackoverflow.com/questions/53022608/application-crashes-with-symbol-zdlpvm-version-qt-5-not-defined-in-file-libqt
	if (not is_plat("windows")) then
		add_cxxflags("-fno-sized-deallocation")
	end

	add_frameworks("QtCore", "QtGui", "QtWidgets")
	add_deps("Main", "ClientLib", "CoreLib")
	add_headerfiles("src/MapEditor/**.hpp", "src/MapEditor/**.inl")
	add_files("src/MapEditor/Widgets/**.hpp", "src/MapEditor/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json", "nazara")
	add_rpathdirs("@executable_path")

	after_install(function (target)
		os.vcp("editorconfig.lua", path.join(target:installdir(), "bin"))
	end)

target_end()
