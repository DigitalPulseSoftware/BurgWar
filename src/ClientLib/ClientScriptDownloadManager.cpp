// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientScriptDownloadManager.hpp>
#include <Nazara/Core/File.hpp>
#include <stdexcept>

namespace bw
{
	void ClientScriptDownloadManager::HandlePacket(const Packets::ClientScriptList& packet)
	{
		Nz::ByteArray checksum;
		for (auto& fileData : packet.scripts)
		{
			checksum.Assign(fileData.sha1Checksum.begin(), fileData.sha1Checksum.end());
			std::string hexChecksum = checksum.ToHex().ToStdString();

			std::filesystem::path clientFilePath = m_clientFileCache / fileData.path;
			clientFilePath.concat("." + hexChecksum);

			bool shouldDownload = false;
			if (std::filesystem::is_regular_file(clientFilePath))
			{
				// Check file against checksum (in case the user changed it)
				Nz::ByteArray checksum;
				std::vector<Nz::UInt8> content;

				Nz::File file(clientFilePath.generic_u8string());
				if (file.Open(Nz::OpenMode_ReadOnly))
				{
					content.resize(file.GetSize());
					if (file.Read(content.data(), content.size()) == content.size())
					{
						auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
						hash->Begin();
						hash->Append(content.data(), content.size());
						checksum = hash->End();
					}
				}

				if (checksum.size() != fileData.sha1Checksum.size() || std::memcmp(checksum.GetConstBuffer(), fileData.sha1Checksum.data(), checksum.GetSize() != 0))
					shouldDownload = true;
				else
					OnFileChecked(this, fileData.path, content);
			}
			else
				shouldDownload = true;

			if (shouldDownload)
			{
				PendingFile& pendingFile = m_downloadList.emplace_back();
				pendingFile.downloadPath = fileData.path;
				pendingFile.outputPath = std::move(clientFilePath);
			}
		}

		RequestNextFile();
	}

	void ClientScriptDownloadManager::HandlePacket(const Packets::DownloadClientScriptResponse& packet)
	{
		PendingFile& pendingFileData = m_downloadList[m_currentFileIndex];

		std::filesystem::path clientFolderPath = pendingFileData.outputPath.parent_path();
		std::string filePath = pendingFileData.outputPath.generic_u8string();

		if (!std::filesystem::is_directory(clientFolderPath))
		{
			if (!std::filesystem::create_directories(clientFolderPath))
				throw std::runtime_error("Failed to create client script cache directory: " + clientFolderPath.generic_u8string());
		}

		Nz::File outputFile(filePath, Nz::OpenMode_Truncate | Nz::OpenMode_WriteOnly);
		if (!outputFile.IsOpen())
			throw std::runtime_error("Failed to open file " + filePath);

		outputFile.Write(packet.fileContent.data(), packet.fileContent.size());
		outputFile.Close();

		OnFileChecked(this, pendingFileData.downloadPath, packet.fileContent);

		m_currentFileIndex++;
		RequestNextFile();
	}

	void ClientScriptDownloadManager::RequestNextFile()
	{
		if (m_currentFileIndex >= m_downloadList.size())
			OnFinished(this);
		else
		{
			Packets::DownloadClientScriptRequest requestPacket;
			requestPacket.path = m_downloadList[m_currentFileIndex].downloadPath;

			OnDownloadRequest(this, requestPacket);
		}
	}
}
