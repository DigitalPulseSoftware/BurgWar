// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_VIRTUALDIRECTORY_HPP
#define BURGWAR_CORELIB_VIRTUALDIRECTORY_HPP

#include <Nazara/Prerequisites.hpp>
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
			using DirectoryEntry = std::shared_ptr<VirtualDirectory>;
			using FileEntry = std::vector<Nz::UInt8>;
			using Entry = std::variant<DirectoryEntry, FileEntry>;

			inline VirtualDirectory(DirectoryEntry parentDirectory = nullptr);
			~VirtualDirectory() = default;

			template<typename F> void Foreach(F&& cb, bool includeDots = false);

			inline bool GetEntry(const std::string_view& path, Entry* entry);

			inline DirectoryEntry& Store(const std::string_view& path, DirectoryEntry directory);
			inline FileEntry& Store(const std::string_view& path, FileEntry file);

		private:
			inline void EnsureDots();
			inline bool RetrieveDirectory(const std::string_view& path, bool allowCreation, std::shared_ptr<VirtualDirectory>& directory, std::string_view& entryName);
			inline bool GetEntryInternal(const std::string_view& name, Entry* entry);
			inline DirectoryEntry& StoreInternal(std::string name, DirectoryEntry directory);
			inline FileEntry& StoreInternal(std::string name, FileEntry file);

			template<typename F1, typename F2> static bool SplitPath(std::string_view path, F1&& dirCB, F2&& fileCB);

			std::map<std::string /*name*/, Entry, std::less<>> m_content;
			DirectoryEntry m_parent;
			bool m_wereDotRegistered;
	};
}

#include <CoreLib/Utility/VirtualDirectory.inl>

#endif
