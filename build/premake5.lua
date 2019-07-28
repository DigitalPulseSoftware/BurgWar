WorkspaceName = "Burgwar"
Projects = {
	{
		Group = "Dependencies",
		Name = "Lua",
		Kind = "StaticLib",
		Defines = {},
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
		Group = "Dependencies",
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
		LinkStaticDebug = {"Lua-d"},
		LinkStaticRelease = {"Lua"},
		Libs = {},
		LibsDebug = {},
		LibsRelease = {},
		AdditionalDependencies = {}
	},
	{
		Group = "Dependencies",
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
		Frameworks = {"Nazara"},
		LinkStatic = {},
		LinkStaticDebug = {"CoreLib-d"},
		LinkStaticRelease = {"CoreLib"},
		Libs = {},
		LibsDebug = {},
		LibsRelease = {},
		AdditionalDependencies = {}
	},
	{
		Group = "Executable",
		Name = "Client",
		Kind = "ConsoleApp",
		Defines = { "SOL_SAFE_NUMERICS=1" },
		Files = {
			"../src/Client/**.hpp",
			"../src/Client/**.inl",
			"../src/Client/**.cpp"
		},
		Frameworks = {"Nazara"},
		LinkStatic = {},
		LinkStaticDebug = {"CoreLib-d", "ClientLib-d", "Lua-d"},
		LinkStaticRelease = {"CoreLib", "ClientLib", "Lua"},
		Libs = {},
		LibsDebug = {"NazaraAudio-d", "NazaraCore-d", "NazaraLua-d", "NazaraGraphics-d", "NazaraNetwork-d", "NazaraNoise-d", "NazaraRenderer-d", "NazaraPhysics2D-d", "NazaraPhysics3D-d", "NazaraPlatform-d", "NazaraSDK-d", "NazaraUtility-d"},
		LibsRelease = {"NazaraAudio", "NazaraCore", "NazaraLua", "NazaraGraphics", "NazaraNetwork", "NazaraNoise", "NazaraRenderer", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraPlatform", "NazaraSDK", "NazaraUtility"},
		AdditionalDependencies = {"Newton", "libsndfile-1", "soft_oal"}
	},
	{
		Group = "Executable",
		Name = "Server",
		Kind = "ConsoleApp",
		Defines = {"NDK_SERVER", "SOL_SAFE_NUMERICS=1"},
		Files = {
			"../src/Server/**.hpp",
			"../src/Server/**.inl",
			"../src/Server/**.cpp"
		},
		Frameworks = {"Nazara"},
		LinkStatic = {},
		LinkStaticDebug = {"CoreLib-d", "Lua-d"},
		LinkStaticRelease = {"CoreLib", "Lua"},
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
		LinkStaticDebug = {"CoreLib-d", "ClientLib-d", "Lua-d"},
		LinkStaticRelease = {"CoreLib", "ClientLib", "Lua"},
		Libs = os.istarget("windows") and {} or {"pthread"},
		LibsDebug = {"Qt5Cored", "Qt5Guid", "Qt5Widgetsd", "NazaraAudio-d", "NazaraCore-d", "NazaraLua-d", "NazaraGraphics-d", "NazaraNetwork-d", "NazaraNoise-d", "NazaraRenderer-d", "NazaraPhysics2D-d", "NazaraPhysics3D-d", "NazaraPlatform-d", "NazaraSDK-d", "NazaraUtility-d"},
		LibsRelease = {"Qt5Core", "Qt5Gui", "Qt5Widgets", "NazaraAudio", "NazaraCore", "NazaraLua", "NazaraGraphics", "NazaraNetwork", "NazaraNoise", "NazaraRenderer", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraPlatform", "NazaraSDK", "NazaraUtility"},
		AdditionalDependencies = {"Newton", "libsndfile-1", "soft_oal"}
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
	Curl = "cUrl",
	Nazara = "Nazara",
	Qt = "Qt"
}

location(_ACTION)

workspace("Burgwar")
	configurations({"Debug", "Release"})
	platforms("x64")
	architecture("x86_64")
	language("C++")
	cppdialect("C++17")

	filter("configurations:Debug")
		symbols("On")

	filter("configurations:Release")
		optimize("On")

	filter {"configurations:Debug", "kind:*Lib"}
		targetsuffix("-d")

	filter "action:vs*"
		defines "_CRT_SECURE_NO_WARNINGS"

	filter({})

	flags { "MultiProcessorCompile", "NoMinimalRebuild" }

	debugdir("../bin/%{cfg.buildcfg}")
	targetdir("../bin/%{cfg.buildcfg}")

	includedirs({
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
				if (type(frameworkTable) ~= "table") then
					error("Unexpected value for " .. configKey .. " config")
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
						print("Framework config key " .. configKey .. " is either not set or invalid, skipping project " .. projectData.Name)
						skipProject = true
						break
					end
				end

				usedFrameworks[framework] = frameworkPackage
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

				links(projectData.Libs)

				filter("configurations:Debug")
					defines({"DEBUG"})
					links(projectData.LibsDebug)
					links(projectData.LinkStaticDebug)
					symbols("On")

				filter("configurations:Release")
					defines({ "NDEBUG"})
					links(projectData.LibsRelease)
					links(projectData.LinkStaticRelease)
					optimize("On")

				filter "action:gmake or gmake2"
					links "stdc++fs"
					links "pthread"

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

			if (usedFrameworks["Qt"]) then
				local mocPath = usedFrameworks["Qt"].MocPath
				if (not mocPath or not os.isfile(mocPath)) then
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
							if (not targetStats or targetStats.mtime < sourceStats.mtime) then
								local file, err = io.open(targetName, "w+")
								if (not file) then
									error(string.format("Failed to open output file %s: %s", targetName, err))
								end

								local command = string.format("%s \"%s\" -i", mocExe, path.getabsolute(filePath))

								print("> " .. command)
								local content, errCode = os.outputof(command)

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
			{ "Release", "/Release" }
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
				libs = table.join(libs, projectData.Libs, projectData["Libs" .. _OPTIONS["buildmode"]], projectData.AdditionalDependencies)
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
