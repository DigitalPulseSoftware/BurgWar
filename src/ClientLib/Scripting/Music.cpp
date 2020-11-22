// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Music.hpp>
#include <ClientLib/ClientEditorApp.hpp>
#include <ClientLib/PlayerConfig.hpp>
#include <stdexcept>

namespace bw
{
	Music::Music(ClientEditorApp& app, Nz::Music music) :
	m_music(std::move(music))
	{
		auto& playerSettings = app.GetPlayerSettings();

		m_music.SetVolume(playerSettings.GetIntegerValue<Nz::UInt8>("Sound.MusicVolume"));
		m_musicVolumeUpdateSlot.Connect(playerSettings.GetIntegerUpdateSignal("Sound.MusicVolume"), [this](long long newValue)
		{
			m_music.SetVolume(float(newValue));
		});
	}
}
