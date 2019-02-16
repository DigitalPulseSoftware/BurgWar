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
			~VirtualDirectory() = default;

			template<typename F> void Foreach(F&& cb, bool includeDots = false);

			inline bool GetEntry(const std::string_view& path, Entry* entry);

			inline FileContentEntry& Store(const std::string_view& path, FileContentEntry file);
			inline PhysicalFileEntry& Store(const std::string_view& path, PhysicalFileEntry filePath);
			inline VirtualDirectoryEntry& Store(const std::string_view& path, VirtualDirectoryEntry directory);

		private:
			inline void EnsureDots();
			inline bool RetrieveDirectory(const std::string_view& path, bool allowCreation, std::shared_ptr<VirtualDirectory>& directory, std::string_view& entryName);
			inline bool GetEntryInternal(const std::string_view& name, Entry* entry);
			inline FileContentEntry& StoreInternal(std::string name, FileContentEntry file);
			inline PhysicalFileEntry& StoreInternal(std::string name, PhysicalFileEntry file);
			inline VirtualDirectoryEntry& StoreInternal(std::string name, VirtualDirectoryEntry directory);

			template<typename F1, typename F2> static bool SplitPath(std::string_view path, F1&& dirCB, F2&& fileCB);

			std::map<std::string /*name*/, Entry, std::less<>> m_content;
			VirtualDirectoryEntry m_parent;
			bool m_wereDotRegistered;
	};
}

#include <CoreLib/Utility/VirtualDirectory.inl>

#endif
