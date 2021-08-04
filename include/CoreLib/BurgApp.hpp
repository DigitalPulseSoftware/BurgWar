// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_BURGAPP_HPP
#define BURGWAR_CORELIB_BURGAPP_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/WebService.hpp>
#include <CoreLib/LogSystem/Enums.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <Nazara/Prerequisites.hpp>
#include <tsl/hopscotch_map.h>
#include <memory>

namespace bw
{
	class ConfigFile;
	class Mod;

	class BURGWAR_CORELIB_API BurgApp
	{
		public:
			BurgApp(LogSide side, const ConfigFile& config);
			~BurgApp() = default;

			inline Nz::UInt64 GetAppTime() const;
			inline const ConfigFile& GetConfig() const;
			inline Logger& GetLogger();
			inline const tsl::hopscotch_map<std::string, std::shared_ptr<Mod>>& GetMods() const;
			inline WebService& GetWebService();

			void Update();

		private:
			Logger m_logger;

		protected:
			void LoadMods();

			const ConfigFile& m_config;
			tsl::hopscotch_map<std::string, std::shared_ptr<Mod>> m_mods;
			Nz::UInt64 m_appTime;
			Nz::UInt64 m_lastTime;
			WebService m_webService;
	};
}

#include <CoreLib/BurgApp.inl>

#endif
