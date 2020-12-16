add_repositories("burgwar-repo xmake-repo")

set_project("BurgWar")
set_version("0.1.0")

add_requires("concurrentqueue", "nlohmann_json")
add_requires("fmt", { debug = is_mode("debug"), config = { header_only = false, vs_runtime = "MD" } })
add_requires("libcurl", { debug = is_mode("debug"), config = { shared = true, vs_runtime = "MD" } })
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
	add_cxflags("/w44062") -- Switch case not handled warning
elseif is_plat("linux") then
	add_syslinks("pthread")
end

target("lua")
	set_kind("static")

	add_includedirs("contrib/lua/include", { public = true })
	add_headerfiles("contrib/lua/include/**.h")
	add_files("contrib/lua/src/**.c")

target("CoreLib")
	set_kind("static")

	add_deps("lua")
	add_headerfiles("include/CoreLib/**.hpp", "include/CoreLib/**.inl")
	add_headerfiles("src/CoreLib/**.hpp", "src/CoreLib/**.inl")
	add_files("src/CoreLib/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json")
	add_packages("nazaraserver", {links = {}})

if (is_plat("windows")) then 
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
					commitHash = os.iorunv(git, {"rev-parse", "--short", "HEAD"}):trim()
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

		io.writefile("src/CoreLib/VersionData.hpp", string.format([[
const char* BuildSystem = "%s";
const char* BuildBranch = "%s";
const char* BuildCommit = "%s";
const char* BuildDate = "%s";
]], system, branch, commitHash, os.date("%Y-%m-%d %H:%M:%S")))
	end)

target("ClientLib")
	set_kind("static")

	add_deps("CoreLib")
	add_headerfiles("include/ClientLib/**.hpp", "include/ClientLib/**.inl")
	add_headerfiles("src/ClientLib/**.hpp", "src/ClientLib/**.inl")
	add_files("src/ClientLib/**.cpp")
	add_packages("libcurl")
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
	add_packages("concurrentqueue", "fmt", "libcurl", "nlohmann_json", "nazara")

	after_install(function (target)
		os.vcp("clientconfig.lua", path.join(target:installdir(), "bin"))
	end)

target("BurgWarServer")
	set_kind("binary")

	add_defines("NDK_SERVER")

	add_deps("Main", "CoreLib")
	add_headerfiles("src/Server/**.hpp", "src/Server/**.inl")
	add_files("src/Server/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json", "nazaraserver")

	after_install(function (target)
		os.vcp("serverconfig.lua", path.join(target:installdir(), "bin"))
	end)

target("BurgWarMapEditor")
	set_kind("binary")
	add_rules("qt.console", "qt.moc")

	add_frameworks("QtCore", "QtGui", "QtWidgets")
	add_deps("Main", "ClientLib", "CoreLib")
	add_headerfiles("src/MapEditor/**.hpp", "src/MapEditor/**.inl")
	add_files("src/MapEditor/Widgets/**.hpp", "src/MapEditor/**.cpp")
	add_packages("concurrentqueue", "fmt", "nlohmann_json", "nazara")

	after_install(function (target)
		os.vcp("editorconfig.lua", path.join(target:installdir(), "bin"))
	end)

	after_install("windows", function (target, opt)
		import("core.base.option")
		import("core.project.config")
		import("core.project.depend")
		import("detect.sdks.find_vstudio")

		local installfile = path.join(target:installdir(), "bin", target:basename() .. ".exe")

		-- need re-generate this app?
		local targetfile = target:targetfile()
		local dependfile = target:dependfile(targetfile)
		local dependinfo = option.get("rebuild") and {} or (depend.load(dependfile) or {})
		if not depend.is_changed(dependinfo, {lastmtime = os.mtime(dependfile)}) then
			return
		end

		-- get qt sdk
		local qt = target:data("qt")

		-- get windeployqt
		local windeployqt = path.join(qt.bindir, "windeployqt.exe")
		assert(os.isexec(windeployqt), "windeployqt.exe not found!")

		-- find qml directory
		local qmldir = nil
		for _, sourcebatch in pairs(target:sourcebatches()) do
			if sourcebatch.rulename == "qt.qrc" then
				for _, sourcefile in ipairs(sourcebatch.sourcefiles) do
					qmldir = find_path("*.qml", path.directory(sourcefile))
					if qmldir then
						break
					end
				end
			end
		end

		-- do deploy
		local vs_studio = find_vstudio()

		local installDir = path.join(vs_studio["2019"].vcvarsall[target:arch()].VSInstallDir, "VC")
		os.addenv("VCINSTALLDIR", installDir)

		local argv = {"--force"}
		if option.get("diagnosis") then
			table.insert(argv, "--verbose=2")
		elseif option.get("verbose") then
			table.insert(argv, "--verbose=1")
		else
			table.insert(argv, "--verbose=0")
		end
		if qmldir then
			table.insert(argv, "--qmldir=" .. qmldir)
		end
		table.insert(argv, installfile)

		os.vrunv(windeployqt, argv)

		-- update files and values to the dependent file
		dependinfo.files = {targetfile}
		depend.save(dependinfo, dependfile)
	end)

target_end()

after_install(function (target)
	if (target:targetkind() == "binary") then
		os.vcp("scripts", path.join(target:installdir(), "bin"))
	end
end)
