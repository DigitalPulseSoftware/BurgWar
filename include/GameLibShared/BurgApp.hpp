// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_BURGAPP_HPP
#define BURGWAR_SHARED_BURGAPP_HPP

#include <Nazara/Prerequisites.hpp>

namespace bw
{
	class BurgApp
	{
		public:
			BurgApp();
			~BurgApp() = default;

			inline Nz::UInt64 GetAppTime() const;

			void Update();

		private:
			Nz::UInt64 m_appTime;
			Nz::UInt64 m_lastTime;
	};
}

#include <GameLibShared/BurgApp.inl>

#endif
