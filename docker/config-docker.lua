-- This file contains special configurations values, such as directories to extern libraries (Qt)
-- Please copy this file to a config.lua with your values

-- Use separate directories option otherwise

-- Use PackageFolder when directory contains include/bin(/lib) directories, use [Bin|Include|Lib]Path otherwise

-- Curl precompiled binaries
cURL = {
	PackageFolder = ":system" -- Use ":system" to tell this library is installed system-wide (typically on Linux)
}

-- Nazara base directory (should contains bin, include and lib directories)
Nazara = {
	PackageFolder = [[/usr/lib/NazaraEngine/package/]]
}

-- Qt base directory (should contains bin, include and lib directories)
Qt = {
	MocPath = [[/usr/lib/x86_64-linux-gnu/qt5/bin/moc]],
	BinPath = [[/usr/lib/x86_64-linux-gnu/]],
	LibPath = [[/usr/lib/x86_64-linux-gnu/]],
	IncludePath = [[/usr/include/x86_64-linux-gnu/qt5/]]
}

-- This work will work with root Dockerfile, you MUST NOT rename the file.
