// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDAPPCONFIG_HPP
#define BURGWAR_CORELIB_SHAREDAPPCONFIG_HPP

#include <CoreLib/ConfigFile.hpp>

namespace bw
{
	class BurgApp;

	class SharedAppConfig : public ConfigFile
	{
		public:
			SharedAppConfig(BurgApp& app);
			~SharedAppConfig() = default;
	};
}

#include <CoreLib/SharedAppConfig.inl>

#endif
