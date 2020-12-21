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
		const ConfigFile& config = app->GetConfig();

		bwLog(app->GetLogger(), LogLevel::Info, "Downloading scripts...");

		auto scriptDirectory = std::make_shared<VirtualDirectory>(app->GetConfig().GetStringValue("Assets.ScriptFolder"));
		auto targetScriptDirectory = std::make_shared<VirtualDirectory>();

		std::filesystem::path resourceCacheFolder = config.GetStringValue("Assets.ScriptCacheFolder");

		m_downloadManager.emplace(m_clientSession, true);

		m_downloadManager->OnDownloadStarted.Connect([this](ClientFileDownloadManager* /*downloadManager*/, const std::string& downloadPath)
		{
			bwLog(GetStateData().app->GetLogger(), LogLevel::Info, "Downloading {}...", downloadPath);

			UpdateStatus("Downloading " + downloadPath + "...", Nz::Color::White);
		});
		
		m_downloadManager->OnFileCheckedMemory.Connect([this, targetScriptDirectory](ClientFileDownloadManager* /*downloadManager*/, const std::string& downloadPath, const std::vector<Nz::UInt8>& fileContent)
		{
			bwLog(GetStateData().app->GetLogger(), LogLevel::Info, "Downloaded {}", downloadPath);

			targetScriptDirectory->StoreFile(downloadPath, fileContent);
		});

		m_downloadManager->OnFileError.Connect([this, targetScriptDirectory](ClientFileDownloadManager* /*downloadManager*/, const std::string& downloadPath, ClientFileDownloadManager::Error error)
		{
			const char* errorReason = "<internal error>";
			switch (error)
			{
				case ClientFileDownloadManager::Error::ChecksumMismatch:
					errorReason = "checksum didn't match";
					break;

				case ClientFileDownloadManager::Error::FileNotFound:
					errorReason = "file not found";
					break;
			}

			bwLog(GetStateData().app->GetLogger(), LogLevel::Error, "Failed to download {}: {}", downloadPath, errorReason);
		});

		m_downloadManager->OnFinished.Connect([this, app, assetDirectory, targetScriptDirectory](ClientFileDownloadManager* /*downloadManager*/) mutable
		{
			bwLog(app->GetLogger(), LogLevel::Info, "Creating match...");
			UpdateStatus("Entering match...", Nz::Color::White);

			SwitchToState(std::make_shared<GameState>(GetStateDataPtr(), m_clientSession, m_authSuccess, m_matchData, std::move(assetDirectory), std::move(targetScriptDirectory)), 0.5f);
		});

		for (const auto& script : m_matchData.scripts)
		{
			Nz::ByteArray expectedChecksum;
			expectedChecksum.Assign(script.sha1Checksum.begin(), script.sha1Checksum.end());

			// Try to find script file in scripts folder (useful for a local client to not redownload scripts)
			VirtualDirectory::Entry entry;
			if (scriptDirectory->GetEntry(script.path, &entry))
			{
				bool isScriptPresent = std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry>)
					{
						auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
						hash->Begin();
						hash->Append(arg.data(), arg.size());

						if (expectedChecksum != hash->End())
							return false;

						targetScriptDirectory->StoreFile(script.path, arg);
						return true;
					}
					else if constexpr (std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
					{
						Nz::ByteArray fileChecksum;
						std::vector<Nz::UInt8> content;

						Nz::File file(arg.generic_u8string());
						if (!file.Open(Nz::OpenMode_ReadOnly))
							return false;

						content.resize(file.GetSize());
						if (file.Read(content.data(), content.size()) != content.size())
							return false;

						auto fileHash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
						fileHash->Begin();
						fileHash->Append(content.data(), content.size());

						if (expectedChecksum != fileHash->End())
							return false;

						targetScriptDirectory->StoreFile(script.path, content);
						return true;
					}
					else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
					{
						return false;
					}
					else
						static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				}, entry);

				if (isScriptPresent)
					continue;
			}

			std::filesystem::path cacheFile = resourceCacheFolder / std::filesystem::u8path(script.path);
			{
				std::string hexChecksum = expectedChecksum.ToHex().ToStdString();

				cacheFile.concat("." + hexChecksum);
			}

			if (std::filesystem::is_regular_file(script.path))
			{
				// Check file against checksum (in case the user changed it)
				Nz::ByteArray fileChecksum;
				std::vector<Nz::UInt8> content;

				Nz::File file(cacheFile.generic_u8string());
				if (file.Open(Nz::OpenMode_ReadOnly))
				{
					content.resize(file.GetSize());
					if (file.Read(content.data(), content.size()) == content.size())
					{
						auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
						hash->Begin();
						hash->Append(content.data(), content.size());
						fileChecksum = hash->End();

						if (expectedChecksum == fileChecksum)
						{
							targetScriptDirectory->StoreFile(script.path, content);
							continue;
						}
					}
				}
			}

			m_downloadManager->RegisterFile(script.path, script.sha1Checksum, cacheFile);
		}
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
