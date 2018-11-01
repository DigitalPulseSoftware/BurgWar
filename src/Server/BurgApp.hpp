// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_BURGAPP_HPP
#define BURGWAR_SERVER_BURGAPP_HPP

#include <Shared/Match.hpp>
#include <NDK/Application.hpp>
#include <memory>

namespace bw
{
	class BurgApp : public Ndk::Application
	{
		public:
			BurgApp(int argc, char* argv[]);
			~BurgApp() = default;

			int Run();

		private:
			std::unique_ptr<Match> m_match;
	};
}

#include <Server/BurgApp.inl>

#endif