// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ScriptDownloadState.hpp>
#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <ClientLib/ClientSession.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/Game/GameState.hpp>

namespace bw
{
	ScriptDownloadState::ScriptDownloadState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, Packets::AuthSuccess authSuccess, Packets::MatchData matchData, std::shared_ptr<VirtualDirectory> assetDirectory, std::shared_ptr<AbstractState> originalState) :
	CancelableState(std::move(stateData), std::move(originalState)),
	m_clientSession(std::move(clientSession)),
	m_authSuccess(std::move(authSuccess)),
	m_matchData(std::move(matchData))
	{
		ClientApp* app = GetStateData().app;

		bwLog(app->GetLogger(), LogLevel::Info, "Downloading scripts...");

		m_downloadManager.emplace(".scriptCache", m_clientSession);

		auto scriptDirectory = std::make_shared<VirtualDirectory>();

		m_downloadManager->OnFileChecked.Connect([scriptDirectory](ClientScriptDownloadManager* /*downloadManager*/, const std::string& filePath, const std::vector<Nz::UInt8>& fileContent)
		{
			scriptDirectory->StoreFile(filePath, fileContent);
		});

		m_downloadManager->OnDownloadRequest.Connect([this](ClientScriptDownloadManager* /*downloadManager*/, const Packets::DownloadClientScriptRequest& request)
		{
			UpdateStatus("Downloading " + request.path + "...", Nz::Color::White);

			m_clientSession->SendPacket(request);
		});

		m_downloadManager->OnFinished.Connect([this, app, assetDirectory, scriptDirectory](ClientScriptDownloadManager* /*downloadManager*/) mutable
		{
			bwLog(app->GetLogger(), LogLevel::Info, "Creating match...");
			UpdateStatus("Entering match...", Nz::Color::White);

			SwitchToState(std::make_shared<GameState>(GetStateDataPtr(), m_clientSession, m_authSuccess, m_matchData, std::move(assetDirectory), std::move(scriptDirectory)), 0.5f);
		});

		for (const auto& asset : m_matchData.scripts)
			m_downloadManager->RegisterFile(asset.path, asset.sha1Checksum);
	}

	void ScriptDownloadState::Enter(Ndk::StateMachine& fsm)
	{
		StatusState::Enter(fsm);

		m_downloadManager->Start();
	}
	
	void ScriptDownloadState::OnCancelled()
	{
		m_clientSession->Disconnect();
	}
}
