// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTCONFIGFILE_HPP
#define BURGWAR_CLIENTCONFIGFILE_HPP

#include <CoreLib/SharedAppConfig.hpp>

namespace bw
{
	class ClientApp;

	class ClientAppConfig : public SharedAppConfig
	{
		public:
			ClientAppConfig(ClientApp& app);
			~ClientAppConfig() = default;
	};
}

#include <Client/ClientAppConfig.inl>

#endif
