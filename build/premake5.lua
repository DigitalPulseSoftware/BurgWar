local function luaDefines()
	if (os.istarget("macosx")) then
		return {"LUA_USE_MACOSX"}
	elseif (os.istarget("linux")) then
		return {"LUA_USE_LINUX"}
	elseif (os.istarget("posix")) then
		return {"LUA_USE_POSIX"}
	else
		return {}
	end
end

local function qtDebugLib(libname)
	if (os.istarget("windows")) then
		return libname .. "d"
	else
		return libname
	end
end

WorkspaceName = "Burgwar"
Projects = {
	{
		Group = "3rdparties",
		Name = "libfmt",
		Kind = "StaticLib",
		Defines = {},
		DisableWarnings = true,
		Files = {
			"../contrib/fmt/include/fmt/**.h",
			"../contrib/fmt/src/fmt/**.cc",
		},
		Libs = {},
		LibsDebug = {},
		LibsRelease = {},
		AdditionalDependencies = {}
	},
	{
		Group = "3rdparties",
		Name = "lua",
		Kind = "StaticLib",
		Defines = luaDefines(),
		DisableWarnings = true,
		Files = {
			"../contrib/lua/include/**.h",
			"../contrib/lua/include/**.hpp",
			"../contrib/lua/src/**.h",
			"../contrib/lua/src/**.c",
		},
		Libs = {},
		LibsDebug = {},
		LibsRelease = {},
		AdditionalDependencies = {}
	},
	{
		Group = "Common",
		Name = "CoreLib",
		Kind = "StaticLib",
		Defines = { "SOL_SAFE_NUMERICS=1" },
		Files = {
			"../include/CoreLib/**.hpp",
			"../include/CoreLib/**.inl",
			"../src/CoreLib/**.hpp",
			"../src/CoreLib/**.inl",
			"../src/CoreLib/**.cpp"
		},
		Frameworks = {"Nazara"},
		LinkStatic = {},
		LinkStaticDebug = {"lua-d", "libfmt-d"},
		LinkStaticRelease = {"lua", "libfmt"},
		Libs = {},
		LibsDebug = {},
		LibsRelease = {},
		AdditionalDependencies = {}
	},
	{
		Group = "Common",
		Name = "ClientLib",
		Kind = "StaticLib",
		Defines = { "SOL_SAFE_NUMERICS=1" },
		Files = {
			"../include/ClientLib/**.hpp",
			"../include/ClientLib/**.inl",
			"../src/ClientLib/**.hpp",
			"../src/ClientLib/**.inl",
			"../src/ClientLib/**.cpp"
		},
		Frameworks = {"Curl", "Nazara"},
		LinkStatic = {},
		LinkStaticDebug = {"CoreLib-d"},
		LinkStaticRelease = {"CoreLib"},
		Libs = os.istarget("windows") and {"libcurl"} or {"curl"},
		LibsDebug = {},
		LibsRelease = {},
		AdditionalDependencies = {}
	},
	{
		Group = "Common",
		Name = "Main",
		Kind = "StaticLib",
		Files = {
			"../include/Main/**.hpp",
			"../include/Main/**.inl",
			"../src/Main/**.hpp",
			"../src/Main/**.inl",
			"../src/Main/**.cpp"
		},
		LinkStaticDebug = {"CoreLib-d"},
		LinkStaticRelease = {"CoreLib"},
	},
	{
		Group = "Executable",
		Name = "BurgWar",
		Kind = "ConsoleApp",
		Defines = { "SOL_SAFE_NUMERICS=1" },
		Files = {
			"../src/Client/**.hpp",
			"../src/Client/**.inl",
			"../src/Client/**.cpp"
		},
		Frameworks = {"Curl", "Nazara"},
		LinkStatic = {},
		LinkStaticDebug = {"Main-d", "ClientLib-d", "CoreLib-d", "lua-d", "libfmt-d"},
		LinkStaticRelease = {"Main", "ClientLib", "CoreLib", "lua", "libfmt"},
		Libs = os.istarget("windows") and {"libcurl"} or {"curl"},
		LibsDebug = {"NazaraAudio-d", "NazaraCore-d", "NazaraLua-d", "NazaraGraphics-d", "NazaraNetwork-d", "NazaraNoise-d", "NazaraRenderer-d", "NazaraPhysics2D-d", "NazaraPhysics3D-d", "NazaraPlatform-d", "NazaraSDK-d", "NazaraUtility-d"},
		LibsRelease = {"NazaraAudio", "NazaraCore", "NazaraLua", "NazaraGraphics", "NazaraNetwork", "NazaraNoise", "NazaraRenderer", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraPlatform", "NazaraSDK", "NazaraUtility"},
		AdditionalDependencies = {"Newton", "libsndfile-1", "soft_oal", "SDL2"}
	},
	{
		Group = "Executable",
		Name = "BWServer",
		Kind = "ConsoleApp",
		Defines = { "NDK_SERVER", "SOL_SAFE_NUMERICS=1" },
		Files = {
			"../src/Server/**.hpp",
			"../src/Server/**.inl",
			"../src/Server/**.cpp"
		},
		Frameworks = {"Nazara"},
		LinkStatic = {},
		LinkStaticDebug = {"Main-d", "CoreLib-d", "lua-d", "libfmt-d"},
		LinkStaticRelease = {"Main", "CoreLib", "lua", "libfmt"},
		Libs = {},
		LibsDebug = {"NazaraCore-d", "NazaraLua-d", "NazaraNetwork-d", "NazaraNoise-d", "NazaraPhysics2D-d", "NazaraPhysics3D-d", "NazaraSDKServer-d", "NazaraUtility-d"},
		LibsRelease = {"NazaraCore", "NazaraLua", "NazaraNetwork", "NazaraNoise", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraSDKServer", "NazaraUtility"},
		AdditionalDependencies = {"Newton"}
	},
	{
		Group = "Tools",
		Name = "BWMapEditor",
		Kind = "ConsoleApp",
		Defines = {},
		Files = {
			"../src/MapEditor/**.hpp",
			"../src/MapEditor/**.inl",
			"../src/MapEditor/**.cpp"
		},
		Frameworks = {"Nazara", "Qt"},
		LinkStatic = {},
		LinkStaticDebug = {"Main-d", "ClientLib-d", "CoreLib-d", "lua-d", "libfmt-d"},
		LinkStaticRelease = {"Main", "ClientLib", "CoreLib", "lua", "libfmt"},
		Libs = {},
		LibsDebug = {qtDebugLib("Qt5Core"), qtDebugLib("Qt5Gui"), qtDebugLib("Qt5Widgets"), "NazaraAudio-d", "NazaraCore-d", "NazaraLua-d", "NazaraGraphics-d", "NazaraNetwork-d", "NazaraNoise-d", "NazaraRenderer-d", "NazaraPhysics2D-d", "NazaraPhysics3D-d", "NazaraPlatform-d", "NazaraSDK-d", "NazaraUtility-d"},
		LibsRelease = {"Qt5Core", "Qt5Gui", "Qt5Widgets", "NazaraAudio", "NazaraCore", "NazaraLua", "NazaraGraphics", "NazaraNetwork", "NazaraNoise", "NazaraRenderer", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraPlatform", "NazaraSDK", "NazaraUtility"},
		AdditionalDependencies = {"Newton", "libsndfile-1", "soft_oal", "SDL2"}
	}
}

Config = {}

local configLoader, err = load(io.readfile("config.lua"), "config.lua", "t", Config)
if (not configLoader) then
	error("config.lua failed to load: " .. err)
end

local configLoaded, err = pcall(configLoader)
if (not configLoaded) then
	error("config.lua failed to load: " .. err)
end

local frameworkConfigs = {
	Curl = "cURL",
	Nazara = "Nazara",
	Qt = "Qt"
}

location(_ACTION)

workspace("Burgwar")
	configurations({"Debug", "Release", "ReleaseWithDebug"})
	platforms("x64")
	architecture("x86_64")
	language("C++")
	cppdialect("C++17")

	filter("configurations:Debug")
		symbols("On")

	filter("configurations:Release")
		optimize("On")

	filter("configurations:ReleaseWithDebug")
		symbols("On")
		optimize("Speed")

	filter("configurations:ReleaseWithDebug", "action:vs*")
		buildoptions "/Zo"

	filter {"configurations:Debug", "kind:*Lib"}
		targetsuffix("-d")

	filter "action:vs*"
		defines "_CRT_SECURE_NO_WARNINGS"
		buildoptions "/bigobj"

	filter({})

	flags { "MultiProcessorCompile", "NoMinimalRebuild" }

	debugdir("../bin/%{cfg.buildcfg}")
	targetdir("../bin/%{cfg.buildcfg}")

	includedirs({
		"../contrib/fmt/include",
		"../contrib/lua/include",
		"../include",
		"../src",
		"../thirdparty/include"
	})

	libdirs({
		"../thirdparty/lib/" .. tostring(_ACTION),
		"../bin/%{cfg.buildcfg}"
	})

	pic("On")

	if (os.ishost("windows")) then
		local commandLine = "premake5.exe " .. table.concat(_ARGV, ' ')

		prebuildcommands("cd .. && " .. commandLine)
		filter("kind:*App")
			postbuildcommands("cd .. && premake5.exe --buildarch=%{cfg.architecture} --buildmode=%{cfg.buildcfg} thirdparty_sync")
	end

	for _, projectData in pairs(Projects) do
		local skipProject = false
		local frameworkIncludes = {}
		local frameworkBins = {}
		local frameworkLibs = {}

		local usedFrameworks = {}
		if (projectData.Frameworks) then
			for _, framework in pairs(projectData.Frameworks) do
				local configKey = assert(frameworkConfigs[framework], "Unknown framework " .. framework)
				local frameworkTable = Config[configKey]
				if (not frameworkTable) then
					print("Framework config key " .. configKey .. " is not set, skipping project " .. projectData.Name)
					skipProject = true
					break
				end

				if (type(frameworkTable) ~= "table") then
					error("Unexpected value for " .. configKey .. " config (expected table)")
				end

				local frameworkPackage = frameworkTable.PackageFolder
				if (frameworkPackage) then
					if (frameworkPackage ~= ":system") then
						table.insert(frameworkBins, frameworkPackage .. "/bin")
						table.insert(frameworkIncludes, frameworkPackage .. "/include")
						table.insert(frameworkLibs, frameworkPackage .. "/lib")
					end
				else
					local binPath = frameworkTable.BinPath
					local includePath = frameworkTable.IncludePath
					local libPath = frameworkTable.LibPath

					if (includePath) then
						table.insert(frameworkIncludes, includePath)

						if (binPath) then
							table.insert(frameworkBins, binPath)
						end

						if (libPath) then
							table.insert(frameworkLibs, libPath)
						end
					else
						print("Framework config key " .. configKey .. " is or invalid, skipping project " .. projectData.Name)
						skipProject = true
						break
					end
				end

				usedFrameworks[framework] = frameworkTable
			end

			-- Stabilize projects settings
			table.sort(frameworkBins)
			table.sort(frameworkIncludes)
			table.sort(frameworkLibs)
		end

		if (not skipProject) then
			group(projectData.Group)
			project(projectData.Name)
				kind(projectData.Kind)
				defines(projectData.Defines)
				files(projectData.Files)

				if (not projectData.DisableWarnings) then
					warnings("Extra")
				end

				filter("configurations:Debug")
					defines({"DEBUG"})
					links(projectData.LinkStaticDebug)
					links(projectData.Libs)
					links(projectData.LibsDebug)
					symbols("On")

				filter("configurations:Release*")
					defines({ "NDEBUG"})
					links(projectData.LinkStaticRelease)
					links(projectData.Libs)
					links(projectData.LibsRelease)

				filter "action:gmake*"
					links "stdc++fs"
					links "dl"
					links "pthread"

				filter "action:vs*"
					files("../thirdparty/include/**.natvis")

				filter {}

			filter {}
			for _, dir in pairs(frameworkIncludes) do
				includedirs(dir)
			end

			for _, dir in pairs(frameworkLibs) do
				filter {"architecture:x86", "configurations:Debug"}
					libdirs(dir .. "/debug")
					libdirs(dir .. "/x86/debug")

				filter {"architecture:x86"}
					libdirs(dir)
					libdirs(dir .. "/x86")

				filter {"architecture:x64", "configurations:Debug"}
					libdirs(dir .. "/debug")
					libdirs(dir .. "/x64/debug")

				filter {"architecture:x64"}
					libdirs(dir)
					libdirs(dir .. "/x64")
			end

			for _, dir in pairs(frameworkBins) do
				filter {"architecture:x86", "configurations:Debug"}
					libdirs(dir .. "/debug")
					libdirs(dir .. "/x86/debug")

				filter {"architecture:x86"}
					libdirs(dir)
					libdirs(dir .. "/x86")

				filter {"architecture:x64", "configurations:Debug"}
					libdirs(dir .. "/debug")
					libdirs(dir .. "/x64/debug")

				filter {"architecture:x64"}
					libdirs(dir)
					libdirs(dir .. "/x64")
			end

			if (usedFrameworks["Qt"]) then
				local mocPath = usedFrameworks["Qt"].MocPath
				if (mocPath and os.isfile(mocPath)) then
					local absMocPath, err = os.realpath(mocPath)
					if (not absMocPath) then
						error(string.format("Failed to retrieve absolute moc path: %s", err))
					end
					mocPath = absMocPath

					local headerFiles = {}
					for _, filter in pairs(projectData.Files) do
						if (filter:endswith(".hpp")) then
							headerFiles = table.join(headerFiles, os.matchfiles(filter))
						end
					end

					for k,filePath in pairs(headerFiles) do
						local input, err = io.open(filePath, "r")
						if (not input) then
							error(string.format("Failed to open input file %s: %s", filePath, err))
						end

						local found = false
						for line in input:lines() do
							if (line:match("Q_OBJECT")) then
								found = true
								break
							end
						end
						input:close()

						if (found) then
							local sourceStats = os.stat(filePath)
							if (not sourceStats) then
								error(string.format("Failed to get stats from input file %s", filePath))
							end

							local targetName = filePath:sub(1, -5) .. ".moc"

							local targetStats = os.stat(targetName)
							if (not targetStats or targetStats.size == 0 or targetStats.mtime < sourceStats.mtime) then
								local file, err = io.open(targetName, "w+")
								if (not file) then
									error(string.format("Failed to open output file %s: %s", targetName, err))
								end

								local command = string.format("%s \"%s\" -i", mocPath, path.getabsolute(filePath))

								print("> " .. command)
								local content, errCode = os.outputof(command)
								if (not content) then
									error("Failed to write moc output: " .. tostring(errCode))
								end

								file:write(content)
								file:close()
							end
						end
					end
				else
					print("Warning: moc executable not set/found, is your MocPath Qt config correctly set up?")
				end
			end
		end
	end

	newoption({
		trigger     = "buildarch",
		description = "Set the directory for the thirdparty_update",
		value       = "VALUE",
		allowed = {
			{ "x86", "/x86" },
			{ "x86_64", "/x64" }
		}
	})

	newoption({
		trigger     = "buildmode",
		description = "Set the directory for the thirdparty_update",
		value       = "VALUE",
		allowed = {
			{ "Debug", "/Debug" },
			{ "Release", "/Release" },
			{ "ReleaseWithDebug", "/ReleaseWithDebug" }
		}
	})

	newaction {
		trigger = "thirdparty_sync",
		description = "Update .dll files from thirdparty directory",

		execute = function()
			assert(_OPTIONS["buildarch"])
			assert(_OPTIONS["buildmode"])

			local archToDir = {
				["x86"] = "x86",
				["x86_64"] = "x64"
			}

			local archDir = archToDir[_OPTIONS["buildarch"]]
			assert(archDir)

			local binPaths = {}
			for k,configKey in pairs(frameworkConfigs) do
				local frameworkTable = Config[configKey]
				if (type(frameworkTable) ~= "table") then
					error("Unexpected value for " .. configKey .. " config")
				end

				local binPath

				local frameworkPackage = frameworkTable.PackageFolder
				if (frameworkPackage) then
					if (frameworkPackage ~= ":system") then
						binPath = frameworkPackage .. "/bin"
					end
				else
					binPath = frameworkTable.BinPath
				end

				if (binPath) then
					table.insert(binPaths, binPath)
					table.insert(binPaths, binPath .. "/" .. archDir)
				end
			end

			local updatedCount = 0
			local totalCount = 0

			local libs = {}
			for name,projectData in pairs(Projects) do
				local buildmodeDir = _OPTIONS["buildmode"]
				if (buildmodeDir == "ReleaseWithDebug") then
					buildmodeDir = "Release"
				end

				libs = table.join(libs, projectData.Libs, projectData["Libs" .. buildmodeDir], projectData.AdditionalDependencies)
			end

			for k,lib in pairs(libs) do
				lib = lib .. ".dll"
				local found = false
				local sourcePath
				for k,v in pairs(binPaths) do
					sourcePath = v .. "/" .. lib
					if (os.isfile(sourcePath)) then
						found = true
						break
					else
						sourcePath = v .. "/" .. path.getdirectory(lib) .. "/lib" .. path.getname(lib)
						if (os.isfile(sourcePath)) then
							lib = "lib" .. lib
							found = true
							break
						end
					end
				end

				if (found) then
					local fileName = path.getname(sourcePath)
					local targetPath = path.normalize(path.translate("../bin/" .. _OPTIONS["buildmode"] .. "/" .. lib), "/")

					local copy = true
					if (os.isfile(targetPath)) then
						local sourceUpdateTime = os.stat(sourcePath).mtime
						local targetUpdateTime = os.stat(targetPath).mtime

						if (targetUpdateTime > sourceUpdateTime) then
							copy = false
						end
					end

					if (copy) then
						print("Copying " .. lib .. "...")

						local ok, err = os.copyfile(sourcePath, targetPath)
						if (not ok) then
							error("Failed to copy " .. targetPath .. ": " .. tostring(err))
						end

						-- Copying using os.copyfile doesn't update modified time...
						local ok, err = os.touchfile(targetPath)
						if (not ok) then
							error("Failed to touch " .. targetPath .. ": " .. tostring(err))
						end

						updatedCount = updatedCount + 1
					end

					totalCount = totalCount + 1
				else
					print("Dependency not found: " .. lib)
				end
			end

			print("" .. updatedCount .. "/" .. totalCount .. " files required an update")
		end
	}
