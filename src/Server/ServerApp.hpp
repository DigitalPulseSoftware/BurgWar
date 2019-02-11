// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVERAPP_HPP
#define BURGWAR_SERVERAPP_HPP

#include <GameLibShared/BurgApp.hpp>
#include <GameLibShared/Match.hpp>
#include <NDK/Application.hpp>
#include <memory>

namespace bw
{
	class ServerApp : public Ndk::Application, public BurgApp
	{
		public:
			ServerApp(int argc, char* argv[]);
			~ServerApp() = default;

			int Run();

		private:
			std::unique_ptr<Match> m_match;
	};
}

#include <Server/ServerApp.inl>

#endif