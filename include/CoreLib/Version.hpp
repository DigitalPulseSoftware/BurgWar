// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_VERSION_HPP
#define BURGWAR_CORELIB_VERSION_HPP

#include <cstdint>
#include <string>

#define BURGWAR_VERSION_MAJOR 0
#define BURGWAR_VERSION_MINOR 1
#define BURGWAR_VERSION_PATCH 1

#define BURGWAR_BUILD_VERSION(major, minor, patch) ((static_cast<std::uint32_t>(major) << 22) | (static_cast<std::uint32_t>(minor) << 12) | static_cast<std::uint32_t>(patch))
#define BURGWAR_VERSION BURGWAR_BUILD_VERSION(BURGWAR_VERSION_MAJOR, BURGWAR_VERSION_MINOR, BURGWAR_VERSION_PATCH)

namespace bw
{
	extern std::uint32_t MajorVersion;
	extern std::uint32_t MinorVersion;
	extern std::uint32_t PatchVersion;

	extern const char* BuildSystem;
	extern const char* BuildBranch;
	extern const char* BuildCommit;
	extern const char* BuildDate;

	std::string GetBuildInfo();
}

#endif
