// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	inline VirtualDirectory::VirtualDirectory(DirectoryEntry parentDirectory) :
	m_parent(std::move(parentDirectory)),
	m_wereDotRegistered(false)
	{
	}

	template<typename F>
	inline void VirtualDirectory::Foreach(F&& cb, bool includeDots)
	{
		if (includeDots)
			EnsureDots();

		for (auto&& pair : m_content)
		{
			if (!includeDots && (pair.first == "." || pair.first == ".."))
				continue;
	
			cb(pair.first, pair.second);
		}
	}

	inline bool VirtualDirectory::GetEntry(const std::string_view& path, Entry* entry)
	{
		std::shared_ptr<VirtualDirectory> dir;
		std::string_view entryName;
		if (!RetrieveDirectory(path, false, dir, entryName))
			return false;

		if (!dir->GetEntryInternal(entryName, entry))
			return false;

		return true;
	}

	inline auto VirtualDirectory::Store(const std::string_view& path, DirectoryEntry directory) -> DirectoryEntry&
	{
		std::shared_ptr<VirtualDirectory> dir;
		std::string_view entryName;
		if (!RetrieveDirectory(path, true, dir, entryName))
			throw std::runtime_error("Invalid path");

		return dir->StoreInternal(std::string(entryName), std::move(directory));
	}

	inline auto VirtualDirectory::Store(const std::string_view& path, FileEntry file) -> FileEntry&
	{
		std::shared_ptr<VirtualDirectory> dir;
		std::string_view entryName;
		if (!RetrieveDirectory(path, true, dir, entryName))
			throw std::runtime_error("Invalid path");

		return dir->StoreInternal(std::string(entryName), std::move(file));
	}

	inline void VirtualDirectory::EnsureDots()
	{
		if (!m_wereDotRegistered)
		{
			StoreInternal(".", shared_from_this());
			if (m_parent)
				StoreInternal("..", m_parent);
			else
				StoreInternal("..", shared_from_this());

			m_wereDotRegistered = true;
		}
	}

	inline bool VirtualDirectory::RetrieveDirectory(const std::string_view& path, bool allowCreation, std::shared_ptr<VirtualDirectory>& directory, std::string_view& entryName)
	{
		directory = shared_from_this();

		return SplitPath(path, [&](std::string_view dirName)
		{
			Entry entry;
			if (directory->GetEntryInternal(dirName, &entry))
			{
				if (auto dir = std::get_if<DirectoryEntry>(&entry))
					directory = *dir;
				else
					return false;
			}
			else
			{
				if (allowCreation)
				{
					auto newDirectory = std::make_shared<VirtualDirectory>(directory);
					directory = directory->StoreInternal(std::string(dirName), newDirectory);
				}
				else
					return false;
			}

			return true;
		}, 
		[&](std::string_view name)
		{
			entryName = name;
		});
	}

	inline bool VirtualDirectory::GetEntryInternal(const std::string_view& name, Entry* entry)
	{
		EnsureDots();

		auto it = m_content.find(name);
		if (it != m_content.end())
		{
			*entry = it->second;
			return true;
		}
		else
			return false;
	}

	inline auto VirtualDirectory::StoreInternal(std::string name, DirectoryEntry directory) -> DirectoryEntry&
	{
		assert(name.find_first_of("\\/:") == name.npos);

		auto it = m_content.insert_or_assign(std::move(name), std::move(directory)).first;
		return std::get<DirectoryEntry>(it->second);
	}

	inline auto VirtualDirectory::StoreInternal(std::string name, FileEntry file) -> FileEntry&
	{
		assert(name.find_first_of("\\/:") == name.npos);

		auto it = m_content.insert_or_assign(std::move(name), std::move(file)).first;
		return std::get<FileEntry>(it->second);
	}

	template<typename F1, typename F2>
	inline bool VirtualDirectory::SplitPath(std::string_view path, F1&& dirCB, F2&& lastCB)
	{
		std::size_t pos;
		while ((pos = path.find_first_of("\\/:")) != std::string::npos)
		{
			if (!dirCB(path.substr(0, pos)))
				return false;

			path = path.substr(pos + 1);
		}

		lastCB(path);
		return true;
	}
}
