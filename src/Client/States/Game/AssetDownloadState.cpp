// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/AssetDownloadState.hpp>
#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <ClientLib/ClientSession.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/Game/ScriptDownloadState.hpp>

namespace bw
{
	AssetDownloadState::AssetDownloadState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, Packets::AuthSuccess authSuccess, Packets::MatchData matchData, std::shared_ptr<AbstractState> originalState) :
	CancelableState(std::move(stateData), std::move(originalState)),
	m_clientSession(std::move(clientSession)),
	m_authSuccess(std::move(authSuccess)),
	m_matchData(std::move(matchData)),
	m_downloadFinished(false),
	m_httpFinished(false)
	{
		ClientApp* app = GetStateData().app;
		const ConfigFile& config = app->GetConfig();

		bwLog(app->GetLogger(), LogLevel::Info, "Downloading assets...");

		auto resourceDirectory = std::make_shared<VirtualDirectory>(config.GetStringValue("Assets.ResourceFolder"));
		m_targetResourceDirectory = std::make_shared<VirtualDirectory>();

		const std::string& assetCacheFolder = config.GetStringValue("Assets.AssetCacheFolder");

		bool hasFastdownloadUrls = !m_matchData.fastDownloadUrls.empty();

		m_httpFinished = true;
		for (std::size_t assetIndex = 0; assetIndex < m_matchData.assets.size(); ++assetIndex)
		{
			const auto& asset = m_matchData.assets[assetIndex];

			Nz::ByteArray expectedChecksum;
			expectedChecksum.Assign(asset.sha1Checksum.begin(), asset.sha1Checksum.end());

			// Try to find file in resource directory
			VirtualDirectory::Entry entry;
			if (resourceDirectory->GetEntry(asset.path, &entry))
			{
				bool isFilePresent = std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry>)
					{
						std::size_t fileSize = arg.size();
						if (fileSize != asset.size)
							return false;

						auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
						hash->Begin();
						hash->Append(arg.data(), arg.size());

						if (expectedChecksum != hash->End())
							return false;

						m_targetResourceDirectory->StoreFile(asset.path, arg);
						return true;
					}
					else if constexpr (std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
					{
						std::size_t fileSize = std::filesystem::file_size(arg);
						if (fileSize != asset.size)
							return false;

						if (expectedChecksum != Nz::File::ComputeHash(Nz::HashType_SHA1, arg.generic_u8string()))
							return false;

						m_targetResourceDirectory->StoreFile(asset.path, arg);
						return true;
					}
					else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
					{
						return false;
					}
					else
						static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				}, entry);

				if (isFilePresent)
					continue;
			}

			// Try to find file in cache
			std::string hexChecksum = expectedChecksum.ToHex().ToStdString();

			std::filesystem::path cachePath = assetCacheFolder / std::filesystem::u8path(asset.path);
			cachePath.replace_extension(hexChecksum + cachePath.extension().generic_u8string());

			if (std::filesystem::is_regular_file(cachePath))
			{
				std::size_t fileSize = std::filesystem::file_size(cachePath);
				if (fileSize == asset.size)
				{
					if (expectedChecksum == Nz::File::ComputeHash(Nz::HashType_SHA1, cachePath.generic_u8string()))
					{
						m_targetResourceDirectory->StoreFile(asset.path, cachePath);
						continue;
					}
				}
			}

			// We have to download the file
			if (hasFastdownloadUrls)
			{
				if (!m_httpDownloadManager)
				{
					m_httpDownloadManager.emplace(app->GetLogger(), std::move(m_matchData.fastDownloadUrls));

					m_httpDownloadManager->OnDownloadStarted.Connect([this](HttpDownloadManager*, const std::string& resourcePath)
					{
						UpdateStatus("Downloading " + resourcePath, Nz::Color::White);
					});

					m_httpDownloadManager->OnFileChecked.Connect([this](HttpDownloadManager* /*downloadManager*/, const std::string& downloadPath, const std::filesystem::path& realPath)
					{
						m_targetResourceDirectory->StoreFile(downloadPath, realPath);
					});

					m_httpDownloadManager->OnFileError.Connect([this, assetIndex, cachePath](HttpDownloadManager* /*downloadManager*/, const std::string& /*downloadPath*/)
					{
						m_remainingFiles.push_back(RemainingFileData{ assetIndex, cachePath });
					});

					m_httpDownloadManager->OnFinished.Connect([this](HttpDownloadManager* /*downloadManager*/)
					{
						m_httpFinished = true;
					});

					m_httpFinished = false;
				}

				// File not found in cache, download it from HTTP sources
				m_httpDownloadManager->RegisterFile(asset.path, asset.sha1Checksum, asset.size, std::move(cachePath));
			}
			else
				m_remainingFiles.push_back(RemainingFileData{ assetIndex, cachePath });
		}
	}

	void AssetDownloadState::Enter(Ndk::StateMachine& fsm)
	{
		StatusState::Enter(fsm);

		if (m_httpDownloadManager)
			m_httpDownloadManager->Start();
	}

	bool AssetDownloadState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!CancelableState::Update(fsm, elapsedTime))
			return false;

		if (m_httpDownloadManager)
			m_httpDownloadManager->Update();

		if (m_httpFinished)
		{
			m_httpDownloadManager.reset();

			if (!m_remainingFiles.empty())
			{
				if (!m_downloadManager)
				{
					m_downloadManager.emplace(m_clientSession);
				
					m_downloadManager->OnDownloadStarted.Connect([this](ClientFileDownloadManager*, const std::string& downloadPath)
					{
						bwLog(GetStateData().app->GetLogger(), LogLevel::Info, "Downloading {}...", downloadPath);

						UpdateStatus("Downloading " + downloadPath, Nz::Color::White);
					});

					m_downloadManager->OnFileChecked.Connect([this](ClientFileDownloadManager* /*downloadManager*/, const std::string& downloadPath, const std::filesystem::path& realPath)
					{
						bwLog(GetStateData().app->GetLogger(), LogLevel::Info, "Downloaded {} (stored to {})", downloadPath, realPath.generic_u8string());

						m_targetResourceDirectory->StoreFile(downloadPath, realPath);
					});
				
					m_downloadManager->OnFileError.Connect([this](ClientFileDownloadManager* /*downloadManager*/, const std::string& downloadPath, ClientFileDownloadManager::Error error)
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

					m_downloadManager->OnFinished.Connect([this](ClientFileDownloadManager* /*downloadManager*/)
					{
						m_downloadFinished = true;
					});

					for (const auto& fileData : m_remainingFiles)
					{
						const auto& asset = m_matchData.assets[fileData.assetIndex];
						m_downloadManager->RegisterFile(asset.path, asset.sha1Checksum, fileData.outputPath);
					}

					m_downloadManager->Start();
				}
			}
			else
				m_downloadFinished = true;

			if (m_downloadFinished && !IsSwitching())
			{
				UpdateStatus("Assets download finished", Nz::Color::White);
				SwitchToState(std::make_shared<ScriptDownloadState>(GetStateDataPtr(), m_clientSession, std::move(m_authSuccess), std::move(m_matchData), std::move(m_targetResourceDirectory), GetOriginalState()), 0.5f);
			}
		}

		return true;
	}

	void AssetDownloadState::OnCancelled()
	{
		m_clientSession->Disconnect();
	}
}
