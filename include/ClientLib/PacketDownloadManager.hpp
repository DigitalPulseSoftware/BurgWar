// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTASSETDOWNLOADMANAGER_HPP
#define BURGWAR_CLIENTLIB_CLIENTASSETDOWNLOADMANAGER_HPP

#include <ClientLib/ClientSession.hpp>
#include <ClientLib/DownloadManager.hpp>
#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Core/File.hpp>
#include <Nazara/Core/AbstractHash.hpp>
#include <filesystem>
#include <limits>

namespace bw
{
	class PacketDownloadManager : public DownloadManager
	{
		public:
			PacketDownloadManager(std::shared_ptr<ClientSession> clientSession);
			~PacketDownloadManager() = default;

			const FileEntry& GetEntry(std::size_t fileIndex) const override;

			bool IsFinished() const override;

			void RegisterFile(std::string downloadPath, const std::array<Nz::UInt8, 20>& checksum, Nz::UInt64 expectedSize, std::filesystem::path outputPath, bool keepInMemory) override;

			void Update() override;

		private:
			void HandlePacket(const Packets::DownloadClientFileFragment& packet);
			void HandlePacket(const Packets::DownloadClientFileResponse& packet);
			void RequestNextFile();

			struct PendingFile : FileEntry
			{
				Nz::Bitset<Nz::UInt64> receivedFragment;
				Nz::UInt64 fragmentSize;
			};

			Nz::ByteArray m_byteArray;
			Nz::File m_outputFile;
			std::unique_ptr<Nz::AbstractHash> m_hash;
			std::filesystem::path m_clientFileCache;
			std::shared_ptr<ClientSession> m_clientSession;
			std::size_t m_nextFileIndex;
			std::vector<Nz::UInt8> m_fileContent;
			std::vector<PendingFile> m_downloadList;

			NazaraSlot(ClientSession, OnDownloadClientFileFragment, m_onDownloadFragmentSlot);
			NazaraSlot(ClientSession, OnDownloadClientFileResponse, m_onDownloadResponseSlot);
	};
}

#include <ClientLib/PacketDownloadManager.inl>

#endif
