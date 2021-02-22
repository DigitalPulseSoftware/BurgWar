// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_VERSION_HPP
#define BURGWAR_CORELIB_VERSION_HPP

#include <CoreLib/Export.hpp>
#include <cstdint>
#include <string>

#define BURGWAR_VERSION_MAJOR 0
#define BURGWAR_VERSION_MINOR 1
#define BURGWAR_VERSION_PATCH 1

#define BURGWAR_BUILD_VERSION(major, minor, patch) ((static_cast<std::uint32_t>(major) << 22) | (static_cast<std::uint32_t>(minor) << 12) | static_cast<std::uint32_t>(patch))
#define BURGWAR_VERSION BURGWAR_BUILD_VERSION(BURGWAR_VERSION_MAJOR, BURGWAR_VERSION_MINOR, BURGWAR_VERSION_PATCH)

namespace bw
{
	BURGWAR_CORELIB_API extern std::uint32_t MajorVersion;
	BURGWAR_CORELIB_API extern std::uint32_t MinorVersion;
	BURGWAR_CORELIB_API extern std::uint32_t PatchVersion;

	BURGWAR_CORELIB_API extern const char* BuildSystem;
	BURGWAR_CORELIB_API extern const char* BuildBranch;
	BURGWAR_CORELIB_API extern const char* BuildCommit;
	BURGWAR_CORELIB_API extern const char* BuildDate;

	BURGWAR_CORELIB_API std::string GetBuildInfo();
}

#endif
