// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/AssetDownloadState.hpp>
#include <ClientLib/ClientSession.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/Game/ScriptDownloadState.hpp>

namespace bw
{
	AssetDownloadState::AssetDownloadState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, Packets::AuthSuccess authSuccess, Packets::MatchData matchData, std::shared_ptr<AbstractState> originalState) :
	CancelableState(std::move(stateData), std::move(originalState)),
	m_clientSession(std::move(clientSession)),
	m_authSuccess(std::move(authSuccess)),
	m_matchData(std::move(matchData))
	{
		ClientApp* app = GetStateData().app;

		bwLog(app->GetLogger(), LogLevel::Info, "Downloading assets...");

		auto resourceDirectory = std::make_shared<VirtualDirectory>(app->GetConfig().GetStringValue("Assets.ResourceFolder"));
		auto targetResourceDirectory = std::make_shared<VirtualDirectory>();

		m_httpDownloadManager.emplace(app->GetLogger(), ".assetCache", std::move(m_matchData.fastDownloadUrls), resourceDirectory);

		m_httpDownloadManager->OnDownloadStarted.Connect([this](HttpDownloadManager*, const std::string& resourcePath)
		{
			UpdateStatus("Downloading " + resourcePath, Nz::Color::White);
		});

		m_httpDownloadManager->OnFileChecked.Connect([this, targetResourceDirectory](HttpDownloadManager* /*downloadManager*/, const std::string& resourcePath, const std::filesystem::path& realPath)
		{
			targetResourceDirectory->StoreFile(resourcePath, realPath);
		});

		m_httpDownloadManager->OnFileCheckedMemory.Connect([this, targetResourceDirectory](HttpDownloadManager* /*downloadManager*/, const std::string& resourcePath, const std::vector<Nz::UInt8>& content)
		{
			targetResourceDirectory->StoreFile(resourcePath, content);
		});

		m_httpDownloadManager->OnFinished.Connect([this, targetResourceDirectory](HttpDownloadManager* /*downloadManager*/) mutable
		{
			UpdateStatus("Assets download finished", Nz::Color::White);
			SwitchToState(std::make_shared<ScriptDownloadState>(GetStateDataPtr(), m_clientSession, std::move(m_authSuccess), std::move(m_matchData), std::move(targetResourceDirectory), GetOriginalState()), 0.5f);
		});

		for (const auto& asset : m_matchData.assets)
			m_httpDownloadManager->RegisterFile(asset.path, asset.sha1Checksum, asset.size);
	}

	void AssetDownloadState::Enter(Ndk::StateMachine& fsm)
	{
		StatusState::Enter(fsm);

		m_httpDownloadManager->Start();
	}

	bool AssetDownloadState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!CancelableState::Update(fsm, elapsedTime))
			return false;

		m_httpDownloadManager->Update();

		return true;
	}

	void AssetDownloadState::OnCancelled()
	{
		m_clientSession->Disconnect();
	}
}
