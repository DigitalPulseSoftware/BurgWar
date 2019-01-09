WorkspaceName = "Burgwar"
Projects = {
	{
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
		Name = "Shared",
		Kind = "StaticLib",
		Defines = {},
		Files = {
			"../include/Shared/**.hpp",
			"../include/Shared/**.inl",
			"../src/Shared/**.hpp",
			"../src/Shared/**.inl",
			"../src/Shared/**.cpp"
		},
		LinkStatic = {},
		LinkStaticDebug = {"Lua-d"},
		LinkStaticRelease = {"Lua"},
		Libs = {},
		LibsDebug = {},
		LibsRelease = {},
		AdditionalDependencies = {}
	},
	{
		Name = "Client",
		Kind = "ConsoleApp",
		Defines = {},
		Files = {
			"../src/Client/**.hpp",
			"../src/Client/**.inl",
			"../src/Client/**.cpp"
		},
		LinkStatic = {},
		LinkStaticDebug = {"Lua-d", "Shared-d"},
		LinkStaticRelease = {"Lua", "Shared"},
		Libs = os.istarget("windows") and {} or {"pthread"},
		LibsDebug = {"NazaraAudio-d", "NazaraCore-d", "NazaraLua-d", "NazaraGraphics-d", "NazaraNetwork-d", "NazaraNoise-d", "NazaraRenderer-d", "NazaraPhysics2D-d", "NazaraPhysics3D-d", "NazaraPlatform-d", "NazaraSDK-d", "NazaraUtility-d"},
		LibsRelease = {"NazaraAudio", "NazaraCore", "NazaraLua", "NazaraGraphics", "NazaraNetwork", "NazaraNoise", "NazaraRenderer", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraPlatform", "NazaraSDK", "NazaraUtility"},
		AdditionalDependencies = {"Newton", "libsndfile-1", "soft_oal"}
	},
	{
		Name = "Server",
		Kind = "ConsoleApp",
		Defines = {"NDK_SERVER"},
		Files = {
			"../src/Server/**.hpp",
			"../src/Server/**.inl",
			"../src/Server/**.cpp"
		},
		LinkStatic = {},
		LinkStaticDebug = {"Lua-d", "Shared-d"},
		LinkStaticRelease = {"Lua", "Shared"},
		Libs = os.istarget("windows") and {} or {"pthread"},
		LibsDebug = {"NazaraCore-d", "NazaraLua-d", "NazaraNetwork-d", "NazaraNoise-d", "NazaraPhysics2D-d", "NazaraPhysics3D-d", "NazaraSDKServer-d", "NazaraUtility-d"},
		LibsRelease = {"NazaraCore", "NazaraLua", "NazaraNetwork", "NazaraNoise", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraSDKServer", "NazaraUtility"},
		AdditionalDependencies = {"Newton"}
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

local libs = {"NazaraPath"}
local libsDirs = {"", "/bin", "/include"}
if (os.istarget("windows")) then
	table.insert(libsDirs, "/lib")
end


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

	if (os.ishost("windows")) then
		local commandLine = "premake5.exe " .. table.concat(_ARGV, ' ')

		prebuildcommands("cd .. && " .. commandLine)
		filter("kind:*App")
			postbuildcommands("cd .. && premake5.exe --buildarch=%{cfg.architecture} --buildmode=%{cfg.buildcfg} thirdparty_sync")
	end

	for k,v in pairs(libs) do
		local dir = Config[v]

		if (#dir > 0) then
			filter {}
				includedirs(dir .. "/include")

			filter {"architecture:x86", "system:not Windows", "configurations:Debug"}
				libdirs(dir .. "/bin/debug")
				libdirs(dir .. "/bin/x86/debug")

			filter {"architecture:x86", "system:not Windows"}
				libdirs(dir .. "/bin")
				libdirs(dir .. "/bin/x86")

			filter {"architecture:x86_64", "system:not Windows", "configurations:Debug"}
				libdirs(dir .. "/bin/debug")
				libdirs(dir .. "/bin/x64/debug")

			filter {"architecture:x86_64", "system:not Windows"}
				libdirs(dir .. "/bin")
				libdirs(dir .. "/bin/x64")

			filter {"architecture:x86", "system:Windows", "configurations:Debug"}
				libdirs(dir .. "/lib/debug")
				libdirs(dir .. "/lib/x86/debug")

			filter {"architecture:x86", "system:Windows"}
				libdirs(dir .. "/lib")
				libdirs(dir .. "/lib/x86")

			filter {"architecture:x86_64", "system:Windows", "configurations:Debug"}
				libdirs(dir .. "/lib/debug")
				libdirs(dir .. "/lib/x64/debug")
			
			filter {"architecture:x86_64", "system:Windows"}
				libdirs(dir .. "/lib")
				libdirs(dir .. "/lib/x64")
		end
	end

	for _, data in pairs(Projects) do
		project(data.Name)
			kind(data.Kind)
			defines(data.Defines)
			files(data.Files)

			links(data.Libs)

			filter "configurations:Debug"
				defines({"DEBUG"})
				links(data.LibsDebug)
				links(data.LinkStaticDebug)
				symbols "On"

			filter "configurations:Release"
				defines({ "NDEBUG"})
				links(data.LibsRelease)
				links(data.LinkStaticRelease)
				optimize("On")
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
			for k,v in pairs(libs) do
				table.insert(binPaths, Config[v] .. "/bin")
				table.insert(binPaths, Config[v] .. "/bin/" .. archDir)
			end

			local updatedCount = 0
			local totalCount = 0

			local libs = {}
			for name,data in pairs(Projects) do
				libs = table.join(libs, data.Libs, data["Libs" .. _OPTIONS["buildmode"]], data.AdditionalDependencies)
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
