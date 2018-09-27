// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_BURGAPP_HPP
#define BURGWAR_SERVER_BURGAPP_HPP

#include <Server/SessionManager.hpp>
#include <NDK/Application.hpp>
#include <optional>

namespace bw
{
	class BurgApp : public Ndk::Application
	{
		public:
			BurgApp(int argc, char* argv[]);
			~BurgApp() = default;

			int Run();

		private:
			void SetupNetwork();

			std::optional<SessionManager> m_sessionManager;
	};
}

#include <Server/BurgApp.inl>

#endif