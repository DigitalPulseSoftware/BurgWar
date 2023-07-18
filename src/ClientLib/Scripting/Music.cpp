// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Music.hpp>
#include <ClientLib/ClientEditorAppComponent.hpp>
#include <ClientLib/PlayerConfig.hpp>
#include <stdexcept>

namespace bw
{
	Music::Music(ClientEditorAppComponent& app, std::unique_ptr<Nz::Music> music) :
	m_music(std::move(music)),
	m_app(app)
	{
		auto& playerSettings = app.GetPlayerSettings();
		m_music->SetVolume(playerSettings.GetIntegerValue<Nz::UInt8>("Sound.MusicVolume"));

		ConnectSignals();
	}

	Music::Music(Music&& music) noexcept :
	m_music(std::move(music.m_music)),
	m_app(music.m_app)
	{
		music.DisconnectSignals();
		ConnectSignals();
	}
	
	Music& Music::operator=(Music&& music) noexcept
	{
		assert(&m_app == &music.m_app);

		music.DisconnectSignals();

		m_music = std::move(music.m_music);

		ConnectSignals();
		return *this;
	}

	void Music::ConnectSignals()
	{
		auto& playerSettings = m_app.GetPlayerSettings();
		m_musicVolumeUpdateSlot.Connect(playerSettings.GetIntegerUpdateSignal("Sound.MusicVolume"), [this](long long newValue)
		{
			m_music->SetVolume(float(newValue));
		});
	}

	void Music::DisconnectSignals()
	{
		m_musicVolumeUpdateSlot.Disconnect();
	}
}
