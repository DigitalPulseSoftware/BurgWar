// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVERAPP_HPP
#define BURGWAR_SERVERAPP_HPP

#include <CoreLib/BurgAppComponent.hpp>
#include <CoreLib/Match.hpp>
#include <Server/ServerAppConfig.hpp>
#include <memory>

namespace bw
{
	class ServerAppComponent : public BurgAppComponent
	{
		public:
			ServerAppComponent(Nz::ApplicationBase& app);
			~ServerAppComponent() = default;

			void Update(Nz::Time elapsedTime) override;

		private:
			std::unique_ptr<Match> m_match;
			ServerAppConfig m_configFile;
	};
}

#include <Server/ServerAppComponent.inl>

#endif
