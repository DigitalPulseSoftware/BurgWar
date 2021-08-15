// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_VERSION_HPP
#define BURGWAR_CORELIB_VERSION_HPP

#include <CoreLib/Export.hpp>
#include <cstdint>
#include <string>

namespace bw
{
	BURGWAR_CORELIB_API extern const char* BuildSystem;
	BURGWAR_CORELIB_API extern const char* BuildBranch;
	BURGWAR_CORELIB_API extern const char* BuildCommit;
	BURGWAR_CORELIB_API extern const char* BuildDate;
	BURGWAR_CORELIB_API std::string GetBuildInfo();

	constexpr std::uint32_t BuildVersion(std::uint32_t majorVersion, std::uint32_t minorVersion, std::uint32_t patchVersion)
	{
		return majorVersion << 22 | minorVersion << 12 | patchVersion;
	}

	constexpr void DecodeVersion(std::uint32_t version, std::uint32_t& majorVersion, std::uint32_t& minorVersion, std::uint32_t& patchVersion)
	{
		majorVersion = (version >> 22) & 0x3FF;
		minorVersion = (version >> 12) & 0x3FF;
		patchVersion = (version >> 0) & 0xFFF;
	}

	constexpr std::uint32_t GameMajorVersion = 0;
	constexpr std::uint32_t GameMinorVersion = 2;
	constexpr std::uint32_t GamePatchVersion = 0;

	constexpr std::uint32_t GameVersion = BuildVersion(GameMajorVersion, GameMinorVersion, GamePatchVersion);
}

#include <CoreLib/Version.inl>

#endif
