// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/PacketDownloadManager.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	PacketDownloadManager::PacketDownloadManager(std::shared_ptr<ClientSession> clientSession) :
	m_clientSession(std::move(clientSession)),
	m_nextFileIndex(0)
	{
		m_hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);

		m_onDownloadFragmentSlot.Connect(m_clientSession->OnDownloadClientFileFragment, [this](ClientSession*, const Packets::DownloadClientFileFragment& packet)
		{
			HandlePacket(packet);
		});

		m_onDownloadResponseSlot.Connect(m_clientSession->OnDownloadClientFileResponse, [this](ClientSession*, const Packets::DownloadClientFileResponse& packet)
		{
			HandlePacket(packet);
		});
	}

	auto PacketDownloadManager::GetEntry(std::size_t fileIndex) const -> const FileEntry&
	{
		assert(fileIndex < m_downloadList.size());
		return m_downloadList[fileIndex];
	}

	bool PacketDownloadManager::IsFinished() const
	{
		if (m_nextFileIndex < m_downloadList.size())
			return false;

		// Check download completion
		if (m_nextFileIndex > 0)
		{
			std::size_t currentFileIndex = m_nextFileIndex - 1;
			if (m_downloadList[currentFileIndex].receivedFragment.GetSize() == 0 || !m_downloadList[currentFileIndex].receivedFragment.TestAll())
				return false;
		}

		return true;
	}

	void PacketDownloadManager::RegisterFile(std::string downloadPath, const std::array<Nz::UInt8, 20>& checksum, Nz::UInt64 expectedSize, std::filesystem::path outputPath, bool keepInMemory)
	{
		PendingFile& pendingFile = m_downloadList.emplace_back();
		pendingFile.downloadPath = std::move(downloadPath);
		pendingFile.expectedChecksum = checksum;
		pendingFile.expectedSize = expectedSize;
		pendingFile.outputPath = std::move(outputPath);
		pendingFile.keepInMemory = keepInMemory;
	}

	void PacketDownloadManager::HandlePacket(const Packets::DownloadClientFileFragment& packet)
	{
		assert(m_nextFileIndex > 0 && m_nextFileIndex <= m_downloadList.size());
		std::size_t currentFileIndex = m_nextFileIndex - 1;

		PendingFile& pendingFileData = m_downloadList[currentFileIndex];
		if (packet.fragmentIndex >= pendingFileData.receivedFragment.GetSize())
			throw std::runtime_error("unexpected fragment " + std::to_string(packet.fragmentIndex) + " from server");

		std::size_t offset = packet.fragmentIndex * pendingFileData.fragmentSize;
		m_outputFile.SetCursorPos(offset);
		m_outputFile.Write(packet.fragmentContent.data(), packet.fragmentContent.size());

		m_hash->Append(packet.fragmentContent.data(), packet.fragmentContent.size());

		std::size_t downloadedSize = (packet.fragmentIndex + 1) * pendingFileData.fragmentSize;
		if (pendingFileData.keepInMemory)
		{
			if (m_fileContent.size() < downloadedSize)
				m_fileContent.resize(downloadedSize);

			std::memcpy(&m_fileContent[offset], packet.fragmentContent.data(), packet.fragmentContent.size());
		}

		pendingFileData.receivedFragment.Set(packet.fragmentIndex, true);
		if (pendingFileData.receivedFragment.TestAll())
		{
			m_outputFile.Close();

			m_byteArray.Assign(pendingFileData.expectedChecksum.begin(), pendingFileData.expectedChecksum.end());

			if (m_byteArray == m_hash->End())
			{
				if (pendingFileData.keepInMemory)
					OnFileCheckedMemory(this, m_currentFileIndex, m_fileContent, 0);
				else
					OnFileChecked(this, m_currentFileIndex, pendingFileData.outputPath, 0);
			}
			else
				OnFileError(this, m_currentFileIndex, Error::ChecksumMismatch);
		}
	}

	void PacketDownloadManager::HandlePacket(const Packets::DownloadClientFileResponse& packet)
	{
		assert(m_nextFileIndex > 0 && m_nextFileIndex <= m_downloadList.size());
		std::size_t currentFileIndex = m_nextFileIndex - 1;

		PendingFile& pendingFileData = m_downloadList[currentFileIndex];
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Packets::DownloadClientFileResponse::Success>)
			{
				pendingFileData.receivedFragment.Resize(arg.fragmentCount);
				pendingFileData.fragmentSize = arg.fragmentSize;

				std::filesystem::path clientFolderPath = pendingFileData.outputPath.parent_path();
				std::string filePath = pendingFileData.outputPath.generic_u8string();

				if (!clientFolderPath.empty() && !std::filesystem::is_directory(clientFolderPath))
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

				OnFileError(this, m_currentFileIndex, error);
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

		}, packet.content);
	}

	void PacketDownloadManager::Update()
	{
		RequestNextFile();
	}

	void PacketDownloadManager::RequestNextFile()
	{
		if (IsFinished())
		{
			OnFinished(this);
			return;
		}

		if (m_nextFileIndex >= m_downloadList.size())
			return; //< All remaining files are being processed

		m_fileContent.clear();
		m_hash->Begin();

		const std::string& downloadPath = m_downloadList[m_nextFileIndex].downloadPath;

		Packets::DownloadClientFileRequest requestPacket;
		requestPacket.path = downloadPath;

		m_clientSession->SendPacket(requestPacket);

		OnDownloadStarted(this, m_nextFileIndex);

		m_nextFileIndex++;
	}
}
