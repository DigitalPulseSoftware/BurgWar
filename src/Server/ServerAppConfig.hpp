// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVERAPPCONFIG_HPP
#define BURGWAR_SERVERAPPCONFIG_HPP

#include <CoreLib/SharedAppConfig.hpp>

namespace bw
{
	class ServerApp;

	class ServerAppConfig : public SharedAppConfig
	{
		public:
			ServerAppConfig(ServerApp& app);
			~ServerAppConfig() = default;
	};
}

#include <Server/ServerAppConfig.inl>

#endif
