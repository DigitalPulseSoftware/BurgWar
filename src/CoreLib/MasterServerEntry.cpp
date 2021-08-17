// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MasterServerEntry.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Version.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Scripting/ServerGamemode.hpp>
#include <nlohmann/json.hpp>
#include <functional>

namespace bw
{
	MasterServerEntry::MasterServerEntry(Match& match, std::string masterServerURL) :
	m_masterServerURL(std::move(masterServerURL)),
	m_match(match),
	m_webService(m_match.GetLogger())
	{
	}

	void MasterServerEntry::Update(float elapsedTime)
	{
		m_webService.Poll();

		m_timeBeforeRefresh -= elapsedTime;
		if (m_timeBeforeRefresh >= 0.f)
			return;

		if (!m_updateToken.empty())
			Refresh();
		else
			Register();
	}

	nlohmann::json MasterServerEntry::BuildServerInfo() const
	{
		const auto& matchSettings = m_match.GetSettings();
		const auto& modSettings = m_match.GetModSettings();

		nlohmann::json serverData;
		serverData["version"] = GameVersion;
		serverData["name"] = matchSettings.name;
		serverData["description"] = matchSettings.description;
		serverData["gamemode"] = m_match.GetGamemode()->GetGamemodeName();
		serverData["map"] = m_match.GetMap().GetMapInfo().name;

		std::size_t currentPlayerCount = 0;
		m_match.ForEachPlayer([&](Player*) { currentPlayerCount++; }, false);

		serverData["current_player_count"] = currentPlayerCount;
		serverData["maximum_player_count"] = matchSettings.maxPlayerCount;

		std::vector<std::string> mods;
		mods.reserve(modSettings.enabledMods.size());

		for (auto&& [modName, modEntry] : modSettings.enabledMods)
			mods.push_back(modName);

		std::sort(mods.begin(), mods.end());

		serverData["mods"] = mods;

		serverData["port"] = matchSettings.port;
		serverData["uptime"] = 0; //< TODO

		return serverData;
	}

	void MasterServerEntry::Refresh()
	{
		std::unique_ptr<WebRequest> request = WebRequest::Post(m_masterServerURL + "/servers", [&](WebRequestResult&& result)
		{
			if (!result)
			{
				bwLog(m_match.GetLogger(), LogLevel::Error, "failed to refresh to {0}, register request failed: {1}", m_masterServerURL, result.GetErrorMessage());
				return;
			}

			switch (result.GetReponseCode())
			{
				case 200:
					bwLog(m_match.GetLogger(), LogLevel::Debug, "successfully refreshed master server entry of {0}", m_masterServerURL);
					m_updateToken = std::move(result.GetBody());
					m_timeBeforeRefresh = 30.f;
					break;

				case 404:
					bwLog(m_match.GetLogger(), LogLevel::Warning, "master server {0} rejected token, retrying to register server...", m_masterServerURL);
					m_updateToken.clear();
					m_timeBeforeRefresh = 1.f;
					break;

				default:
					bwLog(m_match.GetLogger(), LogLevel::Error, "failed to refresh master server {0}, refresh request failed with code {1}", m_masterServerURL, result.GetReponseCode());
					break;
			}
		});

		request->SetServiceName("MasterServer");

		nlohmann::json serverData = BuildServerInfo();
		serverData["update_token"] = m_updateToken;

		request->SetJSonContent(serverData.dump());

		m_webService.AddRequest(std::move(request));
		m_timeBeforeRefresh = 15.f;
	}

	void MasterServerEntry::Register()
	{
		std::unique_ptr<WebRequest> request = WebRequest::Post(m_masterServerURL + "/servers", [&](WebRequestResult&& result)
		{
			if (!result)
			{
				bwLog(m_match.GetLogger(), LogLevel::Error, "failed to register to {0}, register request failed: {1}", m_masterServerURL, result.GetErrorMessage());
				return;
			}

			switch (result.GetReponseCode())
			{
				case 200:
					bwLog(m_match.GetLogger(), LogLevel::Info, "successfully registered server to {0}", m_masterServerURL);
					m_updateToken = std::move(result.GetBody());
					m_timeBeforeRefresh = 30.f;
					break;

				default:
					bwLog(m_match.GetLogger(), LogLevel::Info, "failed to register to {0}, register request failed with code {1}", m_masterServerURL, result.GetReponseCode());
					break;
			}
		});

		request->SetServiceName("MasterServer");

		request->SetJSonContent(BuildServerInfo().dump());

		m_webService.AddRequest(std::move(request));
		m_timeBeforeRefresh = 15.f;
	}
}
