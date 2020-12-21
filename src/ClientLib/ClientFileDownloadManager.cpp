// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientFileDownloadManager.hpp>

namespace bw
{
	void ClientFileDownloadManager::RegisterFile(std::string downloadPath, const std::array<Nz::UInt8, 20>& checksum, std::filesystem::path outputPath)
	{
		PendingFile& pendingFile = m_downloadList.emplace_back();
		pendingFile.downloadPath = std::move(downloadPath);
		pendingFile.expectedChecksum.Assign(checksum.begin(), checksum.end());
		pendingFile.outputPath = std::move(outputPath);
	}

	void ClientFileDownloadManager::HandlePacket(const Packets::DownloadClientFileFragment& packet)
	{
		PendingFile& pendingFileData = m_downloadList[m_currentFileIndex];
		if (packet.fragmentIndex >= pendingFileData.receivedFragment.GetSize())
			throw std::runtime_error("unexpected fragment " + std::to_string(packet.fragmentIndex) + " from server");

		std::size_t offset = packet.fragmentIndex * pendingFileData.fragmentSize;
		m_outputFile.SetCursorPos(offset);
		m_outputFile.Write(packet.fragmentContent.data(), packet.fragmentContent.size());

		if (m_keepInMemory)
		{
			std::size_t minSize = (packet.fragmentIndex + 1) * pendingFileData.fragmentSize;
			if (m_fileContent.size() < minSize)
				m_fileContent.resize(minSize);

			std::memcpy(&m_fileContent[offset], packet.fragmentContent.data(), packet.fragmentContent.size());
		}

		pendingFileData.receivedFragment.Set(packet.fragmentIndex, true);
		if (pendingFileData.receivedFragment.TestAll())
		{
			// Check file hash
			Nz::ByteArray checksum;
			if (m_keepInMemory)
			{
				auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
				hash->Begin();
				hash->Append(m_fileContent.data(), m_fileContent.size());
				checksum = hash->End();
			}
			else
				checksum = Nz::File::ComputeHash(Nz::HashType_SHA1, m_outputFile.GetPath());

			m_outputFile.Close();

			if (checksum == pendingFileData.expectedChecksum)
			{
				if (m_keepInMemory)
					OnFileCheckedMemory(this, pendingFileData.downloadPath, m_fileContent);

				OnFileChecked(this, pendingFileData.downloadPath, pendingFileData.outputPath);
			}
			else
				OnFileError(this, pendingFileData.downloadPath, Error::ChecksumMismatch);

			m_currentFileIndex++;
			RequestNextFile();
		}
	}

	void ClientFileDownloadManager::HandlePacket(const Packets::DownloadClientFileResponse& packet)
	{
		PendingFile& pendingFileData = m_downloadList[m_currentFileIndex];
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Packets::DownloadClientFileResponse::Success>)
			{
				pendingFileData.receivedFragment.Resize(arg.fragmentCount);
				pendingFileData.fragmentSize = arg.fragmentSize;

				std::filesystem::path clientFolderPath = pendingFileData.outputPath.parent_path();
				std::string filePath = pendingFileData.outputPath.generic_u8string();

				if (!std::filesystem::is_directory(clientFolderPath))
				{
					if (!std::filesystem::create_directories(clientFolderPath))
						throw std::runtime_error("failed to create client script cache directory: " + clientFolderPath.generic_u8string());
				}

				if (!m_outputFile.Open(filePath, Nz::OpenMode_Truncate | Nz::OpenMode_WriteOnly))
					throw std::runtime_error("failed to open file " + filePath);
			}
			else if constexpr (std::is_same_v<T, Packets::DownloadClientFileResponse::Failure>)
			{
				using PacketError = Packets::DownloadClientFileResponse::Error;

				Error error = Error::FileNotFound;
				switch (arg.error)
				{
					case PacketError::FileNotFound:
						error = Error::FileNotFound;
						break;
				}

				OnFileError(this, pendingFileData.downloadPath, error);

				m_currentFileIndex++;
				RequestNextFile();
				return;
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

		}, packet.content);
	}

	void ClientFileDownloadManager::Start()
	{
		m_onDownloadFragmentSlot.Connect(m_clientSession->OnDownloadClientFileFragment, [this](ClientSession*, const Packets::DownloadClientFileFragment& packet)
		{
			HandlePacket(packet);
		});

		m_onDownloadResponseSlot.Connect(m_clientSession->OnDownloadClientFileResponse, [this](ClientSession*, const Packets::DownloadClientFileResponse& packet)
		{
			HandlePacket(packet);
		});

		RequestNextFile();
	}

	void ClientFileDownloadManager::RequestNextFile()
	{
		if (m_currentFileIndex >= m_downloadList.size())
			OnFinished(this);
		else
		{
			m_fileContent.clear();

			const std::string& downloadPath = m_downloadList[m_currentFileIndex].downloadPath;

			Packets::DownloadClientFileRequest requestPacket;
			requestPacket.path = downloadPath;

			m_clientSession->SendPacket(requestPacket);

			OnDownloadStarted(this, downloadPath);
		}
	}
}
