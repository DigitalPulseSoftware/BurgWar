// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientScriptDownloadManager.hpp>
#include <Nazara/Core/File.hpp>
#include <iostream>
#include <stdexcept>

namespace bw
{
	void ClientScriptDownloadManager::HandlePacket(const Packets::ClientScriptList& packet)
	{
		Nz::ByteArray checksum;
		for (auto& file : packet.scripts)
		{
			checksum.Assign(file.sha1Checksum.begin(), file.sha1Checksum.end());
			std::string hexChecksum = checksum.ToHex().ToStdString();

			std::filesystem::path clientFilePath = m_clientFileCache / file.path;
			clientFilePath.concat("." + hexChecksum);

			bool shouldDownload = false;
			if (std::filesystem::is_regular_file(clientFilePath))
			{
				// Check file against checksum (in case the user changed it)
				Nz::ByteArray checksum = Nz::File::ComputeHash(Nz::HashType_SHA1, clientFilePath.generic_u8string());
				if (checksum.size() != file.sha1Checksum.size() || std::memcmp(checksum.GetConstBuffer(), file.sha1Checksum.data(), checksum.GetSize() != 0))
					shouldDownload = true;
			}
			else
			{
				std::filesystem::path clientFolderPath = clientFilePath.parent_path();
				if (!std::filesystem::is_directory(clientFolderPath)) 
				{
					if (!std::filesystem::create_directories(clientFolderPath))
						throw std::runtime_error("Failed to create client script cache directory: " + clientFolderPath.generic_u8string());
				}

				shouldDownload = true;
			}

			if (shouldDownload)
			{
				PendingFile& pendingFile = m_downloadList.emplace_back();
				pendingFile.downloadPath = file.path;
				pendingFile.outputPath = std::move(clientFilePath);
			}
		}

		RequestNextFile();
	}

	void ClientScriptDownloadManager::HandlePacket(const Packets::DownloadClientScriptResponse& packet)
	{
		PendingFile& pendingFileData = m_downloadList[m_currentFileIndex];

		Nz::File outputFile(pendingFileData.outputPath.generic_u8string(), Nz::OpenMode_Truncate | Nz::OpenMode_WriteOnly);
		if (!outputFile.IsOpen())
			throw std::runtime_error("Failed to open file " + pendingFileData.outputPath.generic_u8string());

		outputFile.Write(packet.fileContent.data(), packet.fileContent.size());
		outputFile.Close();

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
