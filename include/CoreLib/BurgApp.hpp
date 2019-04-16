// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_BURGAPP_HPP
#define BURGWAR_CORELIB_BURGAPP_HPP

#include <Nazara/Prerequisites.hpp>
#include <CoreLib/ConfigFile.hpp>

namespace bw
{
	class BurgApp
	{
		public:
			BurgApp();
			~BurgApp() = default;

			inline Nz::UInt64 GetAppTime() const;
			inline const ConfigFile& GetConfig() const;

			void Update();

		protected:
			ConfigFile m_config;

		private:
			void RegisterBaseConfig();

			Nz::UInt64 m_appTime;
			Nz::UInt64 m_lastTime;
	};
}

#include <CoreLib/BurgApp.inl>

#endif
