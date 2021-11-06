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
#include <optional>

namespace bw
{
	class ConfigFile;
	class Mod;

	class BURGWAR_CORELIB_API BurgApp
	{
		public:
			BurgApp(LogSide side, const ConfigFile& config);
			~BurgApp();

			inline Nz::UInt64 GetAppTime() const;
			inline const ConfigFile& GetConfig() const;
			inline Logger& GetLogger();
			inline Nz::UInt64 GetLogTime() const;
			inline const tsl::hopscotch_map<std::string, std::shared_ptr<Mod>>& GetMods() const;
			inline WebService& GetWebService();

			void Update();

		private:
			static void HandleInterruptSignal(const char* signalName);

			void InstallInterruptHandlers();

			Logger m_logger;
			
			static BurgApp* s_application;

		protected:
			void LoadMods();

			virtual void Quit() = 0;

			const ConfigFile& m_config;
			std::optional<WebService> m_webService;
			tsl::hopscotch_map<std::string, std::shared_ptr<Mod>> m_mods;
			Nz::UInt64 m_appTime;
			Nz::UInt64 m_lastTime;
			Nz::UInt64 m_startTime;
	};
}

#include <CoreLib/BurgApp.inl>

#endif
