// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ResourceDownloadState.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/HttpDownloadManager.hpp>
#include <ClientLib/PacketDownloadManager.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/Game/GameState.hpp>
#include <Nazara/Core/VirtualDirectory.hpp>
#include <sstream>

namespace bw
{
	ResourceDownloadState::ResourceDownloadState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, Packets::AuthSuccess authSuccess, Packets::MatchData matchData, std::shared_ptr<AbstractState> originalState) :
	CancelableState(std::move(stateData), std::move(originalState)),
	m_clientSession(std::move(clientSession)),
	m_authSuccess(std::move(authSuccess)),
	m_matchData(std::move(matchData))
	{
		ClientApp* app = GetStateData().app;
		const ConfigFile& config = app->GetConfig();

		bwLog(app->GetLogger(), LogLevel::Info, "Downloading resources...");

		m_targetAssetDirectory = std::make_shared<VirtualDirectory>();
		m_targetScriptDirectory = std::make_shared<VirtualDirectory>();

		m_downloadManagers.emplace_back(std::make_unique<PacketDownloadManager>(m_clientSession));

		// Register scripts before adding HTTP download manager (since we won't get theses files from fast download)
		auto scriptDir = std::make_shared<VirtualDirectory>(config.GetStringValue("Resources.ScriptDirectory"));
		RegisterFiles(m_matchData.scripts, scriptDir, m_targetScriptDirectory, config.GetStringValue("Resources.ScriptCacheDirectory"), m_scriptData, true);

		if (!m_matchData.fastDownloadUrls.empty())
		{
			if (WebService::IsInitialized())
				m_downloadManagers.emplace(m_downloadManagers.begin(), std::make_unique<HttpDownloadManager>(app->GetLogger(), std::move(m_matchData.fastDownloadUrls), 2));
			else
				bwLog(app->GetLogger(), LogLevel::Warning, "web services are not initialized, fast download will be disabled");
		}

		// Register assets files
		auto assetDir = std::make_shared<VirtualDirectory>(config.GetStringValue("Resources.AssetDirectory"));
		RegisterFiles(m_matchData.assets, assetDir, m_targetAssetDirectory, config.GetStringValue("Resources.AssetCacheDirectory"), m_assetData, false);

		for (auto& downloadManagerPtr : m_downloadManagers)
		{
			downloadManagerPtr->OnDownloadProgress.Connect([this](DownloadManager* downloadManager, std::size_t fileIndex, Nz::UInt64 downloadedSize)
			{
				const auto& fileEntry = downloadManager->GetEntry(fileIndex);
				auto& downloadData = GetDownloadData(fileEntry.downloadPath);
				downloadData.downloadedSize = downloadedSize;

				UpdateStatus();
			});

			downloadManagerPtr->OnDownloadStarted.Connect([this](DownloadManager* downloadManager, std::size_t fileIndex, const std::string& downloadPath)
			{
				const auto& fileEntry = downloadManager->GetEntry(fileIndex);
				bwLog(GetStateData().app->GetLogger(), LogLevel::Info, "Downloading {}... ({})", downloadPath, ByteToString(fileEntry.expectedSize));
			});

			downloadManagerPtr->OnDownloadFinished.Connect([this](DownloadManager* downloadManager, std::size_t fileIndex, const std::filesystem::path& realPath, Nz::UInt64 downloadSpeed)
			{
				const auto& fileEntry = downloadManager->GetEntry(fileIndex);

				bool isAsset;

				auto& downloadData = GetDownloadData(fileEntry.downloadPath, &isAsset);
				downloadData.downloadedSize = downloadData.totalSize;

				bwLog(GetStateData().app->GetLogger(), LogLevel::Info, "Downloaded {} ({})", fileEntry.downloadPath, ByteToString(downloadSpeed, true));
				if (isAsset)
					m_targetAssetDirectory->StoreFile(fileEntry.downloadPath, realPath);
				else
					m_targetScriptDirectory->StoreFile(fileEntry.downloadPath, realPath);

				UpdateStatus();
			});

			downloadManagerPtr->OnDownloadFinishedMemory.Connect([this](DownloadManager* downloadManager, std::size_t fileIndex, const std::vector<Nz::UInt8>& content, Nz::UInt64 downloadSpeed)
			{
				const auto& fileEntry = downloadManager->GetEntry(fileIndex);

				bool isAsset;

				auto& downloadData = GetDownloadData(fileEntry.downloadPath, &isAsset);
				downloadData.downloadedSize = downloadData.totalSize;

				bwLog(GetStateData().app->GetLogger(), LogLevel::Info, "Downloaded {} ({})", fileEntry.downloadPath, ByteToString(downloadSpeed, true));
				if (isAsset)
					m_targetAssetDirectory->StoreFile(fileEntry.downloadPath, content);
				else
					m_targetScriptDirectory->StoreFile(fileEntry.downloadPath, content);

				UpdateStatus();
			});

			downloadManagerPtr->OnDownloadError.Connect([this](DownloadManager* downloadManager, std::size_t fileIndex, DownloadManager::Error error)
			{
				const auto& fileEntry = downloadManager->GetEntry(fileIndex);

				const char* errorReason = "<internal error>";
				switch (error)
				{
					case DownloadManager::Error::ChecksumMismatch:
						errorReason = "checksum didn't match";
						break;

					case DownloadManager::Error::FileNotFound:
						errorReason = "file not found";
						break;

					case DownloadManager::Error::SizeMismatch:
						errorReason = "size didn't match";
						break;
				}

				bwLog(GetStateData().app->GetLogger(), LogLevel::Error, "File download failed ({}): {}", fileEntry.downloadPath, errorReason);

				// Reset file data and try to download it with another download manager
				auto& fileData = GetDownloadData(fileEntry.downloadPath);
				fileData.downloadedSize = 0;

				auto it = std::find_if(m_downloadManagers.begin(), m_downloadManagers.end(), [&](const auto& downloadManagerPtr) { return downloadManagerPtr.get() == downloadManager; });
				assert(it != m_downloadManagers.end());

				// Go to the next download manager, if any
				++it;
				if (it == m_downloadManagers.end())
					return;

				bwLog(GetStateData().app->GetLogger(), LogLevel::Info, "Retrying with another download manager...");
				const auto& nextDownloadManager = *it;
				nextDownloadManager->RegisterFile(fileEntry.downloadPath, fileEntry.expectedChecksum, fileEntry.expectedSize, fileEntry.outputPath, fileEntry.keepInMemory);
			});
		}
	}

	bool ResourceDownloadState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!CancelableState::Update(fsm, elapsedTime))
			return false;

		bool hasFinished = true;
		for (auto& downloadManagerPtr : m_downloadManagers)
		{
			downloadManagerPtr->Update();

			if (hasFinished && !downloadManagerPtr->IsFinished())
				hasFinished = false;
		}

		if (hasFinished && !IsSwitching())
		{
			bwLog(GetStateData().app->GetLogger(), LogLevel::Info, "Creating match...");
			UpdateStatus("Entering match...", Nz::Color::White);

			SwitchToState(std::make_shared<GameState>(GetStateDataPtr(), m_clientSession, m_authSuccess, m_matchData, std::move(m_targetAssetDirectory), std::move(m_targetScriptDirectory)), 0.5f);
		}

		return true;
	}

	auto ResourceDownloadState::GetDownloadData(const std::string& downloadUrl, bool* isAsset) -> FileData&
	{
		FileMap::iterator it;

		it = m_assetData.find(downloadUrl);
		if (it != m_assetData.end())
		{
			if (isAsset)
				*isAsset = true;

			return it.value();
		}

		it = m_scriptData.find(downloadUrl);
		assert(it != m_scriptData.end());

		if (isAsset)
			*isAsset = false;

		return it.value();
	}

	void ResourceDownloadState::OnCancelled()
	{
		m_clientSession->Disconnect();
	}

	void ResourceDownloadState::RegisterFiles(const std::vector<Packets::MatchData::ClientFile>& files, const std::shared_ptr<Nz::VirtualDirectory>& resourceDir, const std::shared_ptr<Nz::VirtualDirectory>& targetDir, const std::string& cacheDir, FileMap& fileMap, bool keepInMemory)
	{
		assert(!m_downloadManagers.empty());

		auto& downloadManagerPtr = m_downloadManagers.front();

		Nz::ByteArray expectedChecksum;
		for (const auto& resource : files)
		{
			expectedChecksum.Assign(resource.sha1Checksum.begin(), resource.sha1Checksum.end());

			// Try to find file in resource directory
			VirtualDirectory::Entry entry;
			if (resourceDir->GetEntry(resource.path, &entry))
			{
				bool isFilePresent = std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T,Nz::VirtualDirectory::FileContentEntry>)
					{
						std::size_t fileSize = arg.size();
						if (fileSize != resource.size)
							return false;

						auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
						hash->Begin();
						hash->Append(arg.data(), arg.size());

						if (expectedChecksum != hash->End())
							return false;

						targetDir->StoreFile(resource.path, arg);
						return true;
					}
					else if constexpr (std::is_same_v<T,Nz::VirtualDirectory::PhysicalFileEntry>)
					{
						if (keepInMemory)
						{
							Nz::ByteArray fileChecksum;
							std::vector<Nz::UInt8> content;

							Nz::File file(arg.generic_u8string());
							if (!file.Open(Nz::OpenMode::ReadOnly))
								return false;

							content.resize(file.GetSize());
							if (file.Read(content.data(), content.size()) != content.size())
								return false;

							auto fileHash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
							fileHash->Begin();
							fileHash->Append(content.data(), content.size());

							if (expectedChecksum != fileHash->End())
								return false;

							targetDir->StoreFile(resource.path, content);
							return true;
						}
						else
						{
							std::size_t fileSize = std::filesystem::file_size(arg);
							if (fileSize != resource.size)
								return false;

							if (expectedChecksum != Nz::File::ComputeHash(Nz::HashType_SHA1, arg.generic_u8string()))
								return false;

							targetDir->StoreFile(resource.path, arg);
							return true;
						}
					}
					else if constexpr (std::is_same_v<T,Nz::VirtualDirectory::DirectoryEntry>)
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
			std::string hexChecksum = expectedChecksum.ToHex();

			std::filesystem::path cachePath = cacheDir / std::filesystem::u8path(resource.path);
			cachePath.replace_extension(hexChecksum + cachePath.extension().generic_u8string());

			if (std::filesystem::is_regular_file(cachePath))
			{
				std::size_t fileSize = std::filesystem::file_size(cachePath);
				if (fileSize == resource.size)
				{
					if (expectedChecksum == Nz::File::ComputeHash(Nz::HashType_SHA1, cachePath.generic_u8string()))
					{
						targetDir->StoreFile(resource.path, cachePath);
						continue;
					}
				}
			}

			downloadManagerPtr->RegisterFile(resource.path, resource.sha1Checksum, resource.size, std::move(cachePath), keepInMemory);
			fileMap.emplace(resource.path, FileData{ 0, resource.size });
		}
	}

	void ResourceDownloadState::UpdateStatus()
	{
		std::ostringstream status;
		auto WriteData = [&](const char* name, const FileMap& fileMap)
		{
			if (!fileMap.empty())
			{
				std::size_t fullyDownloadedCount = 0;
				Nz::UInt64 downloadedSize = 0;
				Nz::UInt64 totalSize = 0;

				for (const auto& [downloadPath, data] : fileMap)
				{
					if (data.downloadedSize == data.totalSize)
						fullyDownloadedCount++;

					downloadedSize += data.downloadedSize;
					totalSize += data.totalSize;
				}

				if (fullyDownloadedCount < fileMap.size())
				{
					status << "Downloading " << name << "... (";
					status << ByteToString(downloadedSize) << " / " << ByteToString(totalSize);
					status << ")";
				}
				else
					status << "Downloaded " << name << ". (" << ByteToString(totalSize) << ")";

				status << "\n";
			}
		};

		WriteData("assets", m_assetData);
		WriteData("scripts", m_scriptData);

		UpdateStatus(status.str(), Nz::Color::White);
	}
}
