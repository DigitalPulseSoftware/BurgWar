// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVERAPP_HPP
#define BURGWAR_SERVERAPP_HPP

#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Match.hpp>
#include <Server/ServerAppConfig.hpp>
#include <memory>

namespace bw
{
	class ServerApp : public BurgApp
	{
		public:
			ServerApp(int argc, char* argv[]);
			~ServerApp() = default;

			int Run();
			void Quit() override;

		private:
			std::unique_ptr<Match> m_match;
			ServerAppConfig m_configFile;
			bool m_running;
	};
}

#include <Server/ServerApp.inl>

#endif
