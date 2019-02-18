// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_VIRTUALDIRECTORY_HPP
#define BURGWAR_CORELIB_VIRTUALDIRECTORY_HPP

#include <Nazara/Prerequisites.hpp>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace bw
{
	class VirtualDirectory : public std::enable_shared_from_this<VirtualDirectory>
	{
		public:
			using FileContentEntry = std::vector<Nz::UInt8>;
			using PhysicalFileEntry = std::filesystem::path;
			using VirtualDirectoryEntry = std::shared_ptr<VirtualDirectory>;

			using Entry = std::variant<FileContentEntry, PhysicalFileEntry, VirtualDirectoryEntry>;

			inline VirtualDirectory(VirtualDirectoryEntry parentDirectory = nullptr);
			inline VirtualDirectory(std::filesystem::path physicalPath, VirtualDirectoryEntry parentDirectory = nullptr);
			~VirtualDirectory() = default;

			template<typename F> void Foreach(F&& cb, bool includeDots = false);

			inline bool GetEntry(const std::string_view& path, Entry* entry);

			inline VirtualDirectoryEntry& StoreDirectory(const std::string_view& path, VirtualDirectoryEntry directory);
			inline VirtualDirectoryEntry& StoreDirectory(const std::string_view& path, std::filesystem::path directoryPath);
			inline FileContentEntry& StoreFile(const std::string_view& path, FileContentEntry file);
			inline PhysicalFileEntry& StoreFile(const std::string_view& path, std::filesystem::path filePath);

		private:
			inline void EnsureDots();
			inline bool RetrieveDirectory(const std::string_view& path, bool allowCreation, std::shared_ptr<VirtualDirectory>& directory, std::string_view& entryName);
			inline bool GetEntryInternal(const std::string_view& name, Entry* entry);
			inline VirtualDirectoryEntry& StoreDirectoryInternal(std::string name, std::filesystem::path directoryPath);
			inline VirtualDirectoryEntry& StoreDirectoryInternal(std::string name, VirtualDirectoryEntry directory);
			inline FileContentEntry& StoreFileInternal(std::string name, FileContentEntry file);
			inline PhysicalFileEntry& StoreFileInternal(std::string name, std::filesystem::path file);

			template<typename F1, typename F2> static bool SplitPath(std::string_view path, F1&& dirCB, F2&& fileCB);

			std::map<std::string /*name*/, Entry, std::less<>> m_content;
			std::optional<std::filesystem::path> m_physicalPath;
			VirtualDirectoryEntry m_parent;
			bool m_wereDotRegistered;
	};
}

#include <CoreLib/Utility/VirtualDirectory.inl>

#endif
