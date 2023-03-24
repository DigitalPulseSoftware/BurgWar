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
#include <Nazara/Core/ApplicationComponent.hpp>
#include <Nazara/Core/Clock.hpp>
#include <tsl/hopscotch_map.h>
#include <memory>
#include <optional>

namespace bw
{
	class ConfigFile;
	class Mod;

	class BURGWAR_CORELIB_API BurgAppComponent : public Nz::ApplicationComponent
	{
		public:
			BurgAppComponent(Nz::ApplicationBase& app, LogSide side, const ConfigFile& config);
			~BurgAppComponent();

			inline Nz::Time GetAppTime() const;
			inline const ConfigFile& GetConfig() const;
			inline Logger& GetLogger();
			inline Nz::Time GetLogTime() const;
			inline const tsl::hopscotch_map<std::string, std::shared_ptr<Mod>>& GetMods() const;
			inline WebService& GetWebService();

			void Update(Nz::Time elapsedTime) override;

		private:
			static void HandleInterruptSignal(const char* signalName);

			void InstallInterruptHandlers();

			Logger m_logger;
			
			static BurgAppComponent* s_application;

		protected:
			void LoadMods();

			const ConfigFile& m_config;
			std::optional<WebService> m_webService;
			tsl::hopscotch_map<std::string, std::shared_ptr<Mod>> m_mods;
			Nz::Time m_appTime;
			Nz::Time m_startTime;
	};
}

#include <CoreLib/BurgAppComponent.inl>

#endif
