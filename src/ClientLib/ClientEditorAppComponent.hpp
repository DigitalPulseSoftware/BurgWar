// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTEDITORAPP_HPP
#define BURGWAR_CLIENTLIB_CLIENTEDITORAPP_HPP

#include <CoreLib/BurgAppComponent.hpp>
#include <ClientLib/PlayerConfig.hpp>

namespace bw
{
	class SharedAppConfig;

	class BURGWAR_CLIENTLIB_API ClientEditorAppComponent : public BurgAppComponent
	{
		public:
			ClientEditorAppComponent(Nz::ApplicationBase& app, int argc, char* argv[], LogSide side, const SharedAppConfig& configFile);
			~ClientEditorAppComponent();

			inline ConfigFile& GetPlayerSettings();
			inline const ConfigFile& GetPlayerSettings() const;

			void SavePlayerConfig();

		protected:
			void FillStores();

		private:
			PlayerConfig m_playerSettings;
	};
}

#include <ClientLib/ClientEditorAppComponent.inl>

#endif
