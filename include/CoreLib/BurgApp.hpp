// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_BURGAPP_HPP
#define BURGWAR_CORELIB_BURGAPP_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/LogSystem/Enums.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <Nazara/Prerequisites.hpp>

namespace bw
{
	class ConfigFile;

	class BURGWAR_CORELIB_API BurgApp
	{
		public:
			BurgApp(LogSide side, const ConfigFile& config);
			~BurgApp() = default;

			inline Nz::UInt64 GetAppTime() const;
			inline const ConfigFile& GetConfig() const;
			inline Logger& GetLogger();

			void Update();

		private:
			Logger m_logger;

		protected:
			const ConfigFile& m_config;

			Nz::UInt64 m_appTime;
			Nz::UInt64 m_lastTime;
	};
}

#include <CoreLib/BurgApp.inl>

#endif
