// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_DOWNLOADMANAGER_HPP
#define BURGWAR_CLIENTLIB_DOWNLOADMANAGER_HPP

#include <Nazara/Core/ByteArray.hpp>
#include <Nazara/Core/Signal.hpp>
#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace bw
{
	class DownloadManager
	{
		public:
			using Checksum = std::array<Nz::UInt8, 20>; //< SHA1 output

			enum class Error
			{
				ChecksumMismatch,
				FileNotFound,
				SizeMismatch
			};

			struct FileEntry;

			DownloadManager() = default;
			virtual ~DownloadManager();

			virtual const FileEntry& GetEntry(std::size_t fileIndex) const = 0;

			virtual bool IsFinished() const = 0;

			virtual void RegisterFile(std::string downloadPath, const Checksum& checksum, Nz::UInt64 expectedSize, std::filesystem::path outputPath, bool keepInMemory) = 0;

			virtual void Update() = 0;

			struct FileEntry
			{
				Checksum expectedChecksum;
				std::filesystem::path outputPath;
				std::string downloadPath;
				Nz::UInt64 expectedSize;
				bool keepInMemory;
			};

			NazaraSignal(OnDownloadError, DownloadManager* /*downloadManager*/, std::size_t /*fileIndex*/, Error /*error*/);
			NazaraSignal(OnDownloadFinished, DownloadManager* /*downloadManager*/, std::size_t /*fileIndex*/, const std::filesystem::path& /*realPath*/, Nz::UInt64 /*downloadSpeed*/);
			NazaraSignal(OnDownloadFinishedMemory, DownloadManager* /*downloadManager*/, std::size_t /*fileIndex*/, const std::vector<Nz::UInt8>& /*content*/, Nz::UInt64 /*downloadSpeed*/);
			NazaraSignal(OnDownloadProgress, DownloadManager* /*downloadManager*/, std::size_t /*fileIndex*/, Nz::UInt64 /*downloadedSize*/);
			NazaraSignal(OnDownloadStarted, DownloadManager* /*downloadManager*/, std::size_t /*fileIndex*/);
			NazaraSignal(OnFinished, DownloadManager* /*downloadManager*/);
	};
}

#include <ClientLib/DownloadManager.inl>

#endif
