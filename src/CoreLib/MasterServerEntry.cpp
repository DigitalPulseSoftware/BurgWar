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
	namespace
	{
		constexpr Nz::UInt32 MasterServerDataVersion = 1U;
	}

	MasterServerEntry::MasterServerEntry(Nz::WebService& webService, Match& match, std::string masterServerURL) :
	m_masterServerURL(std::move(masterServerURL)),
	m_webService(webService),
	m_match(match)
	{
	}

	void MasterServerEntry::Update(Nz::Time elapsedTime)
	{
		m_webService.Poll();

		m_timeBeforeRefresh -= elapsedTime;
		if (m_timeBeforeRefresh >= Nz::Time::Zero())
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
		serverData["data_version"] = MasterServerDataVersion;
		serverData["game_version"] = GameVersion;
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

	void MasterServerEntry::HandleResponse(Nz::WebRequestResult&& result, bool refresh)
	{
		if (!result)
		{
			bwLog(m_match.GetLogger(), LogLevel::Error, (refresh) ? "failed to refresh to {0}, register request failed: {1}" : "failed to register to {0}, register request failed: {1}", m_masterServerURL, result.GetErrorMessage());
			return;
		}

		switch (result.GetStatusCode())
		{
			case 200:
			{
				std::string updateToken;

				try
				{
					nlohmann::json response = nlohmann::json::parse(result.GetBody());
					Nz::UInt32 dataVersion = response["data_version"];
					if (dataVersion != MasterServerDataVersion)
						bwLog(m_match.GetLogger(), LogLevel::Warning, "unexpected data version (expected {0}, got {1})", MasterServerDataVersion, dataVersion);

					nlohmann::json ipv4UrlValue = response["register_ipv4_url"];
					updateToken = response["token"];

					if (!refresh && ipv4UrlValue.is_string())
					{
						std::string ipv4Url = ipv4UrlValue;
						if (!ipv4Url.empty())
						{
							std::unique_ptr<Nz::WebRequest> request = m_webService.CreatePostRequest(ipv4Url, [&](Nz::WebRequestResult&& result)
							{
								if (result.HasSucceeded())
								{
									if (result.GetStatusCode() == 200)
										bwLog(m_match.GetLogger(), LogLevel::Debug, "successfully registered ipv4 to master server {0}", m_masterServerURL);
									else
										bwLog(m_match.GetLogger(), LogLevel::Error, "failed to register ipv4 to master server {0}, unexpected response {1}: {2}", m_masterServerURL, result.GetStatusCode(), result.GetBody());
								}
								else
									bwLog(m_match.GetLogger(), LogLevel::Error, "failed to register ipv4 to master server {0}: {1}", m_masterServerURL, result.GetErrorMessage());
							});

							request->ForceProtocol(Nz::NetProtocol::IPv4); //< Just in case

							nlohmann::json requestData;
							requestData["update_token"] = updateToken;

							request->SetJSonContent(requestData.dump());

							m_webService.QueueRequest(std::move(request));
						}
					}
				}
				catch (const std::exception& e)
				{
					bwLog(m_match.GetLogger(), LogLevel::Error, (refresh) ? "failed to refresh to {0}: failed to parse response: {1}" : "failed to register to {0}: failed to parse response: {1}", m_masterServerURL, e.what());
					return;
				}

				bwLog(m_match.GetLogger(), LogLevel::Info, (refresh) ? "successfully refreshed server to {0}" : "successfully registered server to {0}", m_masterServerURL);
				m_updateToken = std::move(updateToken);
				m_timeBeforeRefresh = Nz::Time::Seconds(30.f);
				break;
			}

			case 404:
			{
				if (refresh)
				{
					bwLog(m_match.GetLogger(), LogLevel::Warning, "master server {0} rejected token, retrying to register server...", m_masterServerURL);
					m_updateToken.clear();
					m_timeBeforeRefresh = Nz::Time::Seconds(1.f);
					break;
				}
				else
					[[fallthrough]];
			}

			default:
				bwLog(m_match.GetLogger(), LogLevel::Info, (refresh) ? "failed to refresh to {0}: refresh request failed with code {1}" : "failed to register to {0}: register request failed with code {1}", m_masterServerURL, result.GetStatusCode());
				break;
		}
	}

	void MasterServerEntry::Refresh()
	{
		std::unique_ptr<Nz::WebRequest> request = m_webService.CreatePostRequest(m_masterServerURL + "/servers", [&](Nz::WebRequestResult&& result)
		{
			HandleResponse(std::move(result), true);
		});

		request->SetServiceName("MasterServer");

		nlohmann::json serverData = BuildServerInfo();
		serverData["update_token"] = m_updateToken;

		request->SetJSonContent(serverData.dump());

		m_webService.QueueRequest(std::move(request));
		m_timeBeforeRefresh = Nz::Time::Seconds(15.f);
	}

	void MasterServerEntry::Register()
	{
		std::unique_ptr<Nz::WebRequest> request = m_webService.CreatePostRequest(m_masterServerURL + "/servers", [&](Nz::WebRequestResult&& result)
		{
			HandleResponse(std::move(result), false);
		});

		request->SetServiceName("MasterServer");

		request->SetJSonContent(BuildServerInfo().dump());

		m_webService.QueueRequest(std::move(request));
		m_timeBeforeRefresh = Nz::Time::Seconds(15.f);
	}
}
