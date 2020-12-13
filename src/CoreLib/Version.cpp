// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Version.hpp>
#include <fmt/format.h>

namespace bw
{
	std::uint32_t MajorVersion = BURGWAR_VERSION_MAJOR;
	std::uint32_t MinorVersion = BURGWAR_VERSION_MINOR;
	std::uint32_t PatchVersion = BURGWAR_VERSION_PATCH;

	std::string GetBuildInfo()
	{
		return fmt::format("{} - {} ({}) - {}", BuildSystem, BuildBranch, BuildCommit, BuildDate);
	}

#include "VersionData.hpp"
}
