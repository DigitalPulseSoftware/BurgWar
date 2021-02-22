// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTEDITORAPP_HPP
#define BURGWAR_CLIENTLIB_CLIENTEDITORAPP_HPP

#include <CoreLib/BurgApp.hpp>
#include <ClientLib/PlayerConfig.hpp>
#include <NDK/ClientApplication.hpp>

namespace bw
{
	class SharedAppConfig;

	class BURGWAR_CLIENTLIB_API ClientEditorApp : public Ndk::ClientApplication, public BurgApp
	{
		public:
			ClientEditorApp(int argc, char* argv[], LogSide side, const SharedAppConfig& configFile);
			~ClientEditorApp();

			inline ConfigFile& GetPlayerSettings();
			inline const ConfigFile& GetPlayerSettings() const;

			void SavePlayerConfig();

		protected:
			void FillStores();

		private:
			PlayerConfig m_playerSettings;
	};
}

#include <ClientLib/ClientEditorApp.inl>

#endif
