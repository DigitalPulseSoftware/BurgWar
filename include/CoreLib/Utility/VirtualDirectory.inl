// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <cassert>

namespace bw
{
	inline VirtualDirectory::VirtualDirectory(VirtualDirectoryEntry parentDirectory) :
	m_parent(std::move(parentDirectory)),
	m_wereDotRegistered(false)
	{
	}

	inline VirtualDirectory::VirtualDirectory(std::filesystem::path physicalPath, VirtualDirectoryEntry parentDirectory) :
	m_physicalPath(std::move(physicalPath)),
	m_parent(std::move(parentDirectory)),
	m_wereDotRegistered(false)
	{
	}

	template<typename F>
	void VirtualDirectory::Foreach(F&& cb, bool includeDots)
	{
		if (includeDots)
			EnsureDots();

		for (auto&& pair : m_content)
		{
			if (!includeDots && (pair.first == "." || pair.first == ".."))
				continue;
	
			cb(pair.first, pair.second);
		}

		if (m_physicalPath)
		{
			for (auto&& physicalEntry : std::filesystem::directory_iterator(*m_physicalPath))
			{
				Entry entry;

				std::string filename = physicalEntry.path().filename().generic_u8string();
				if (m_content.find(filename) != m_content.end())
					continue; //< Physical file/directory has been overriden by a virtual one

				if (physicalEntry.is_regular_file())
					entry.emplace<PhysicalFileEntry>(physicalEntry.path());
				else if (physicalEntry.is_directory())
				{
					// FIXME: Allocating a shared_ptr on iteration is bad, not sure about a workaround
					entry.emplace<VirtualDirectoryEntry>(std::make_shared<VirtualDirectory>(physicalEntry.path(), shared_from_this()));
				}
				else
					continue;

				cb(physicalEntry.path().filename().generic_u8string(), entry);
			}
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

	inline auto VirtualDirectory::StoreDirectory(const std::string_view& path, VirtualDirectoryEntry directory) -> VirtualDirectoryEntry&
	{
		std::shared_ptr<VirtualDirectory> dir;
		std::string_view entryName;
		if (!RetrieveDirectory(path, true, dir, entryName))
			throw std::runtime_error("invalid path");

		return dir->StoreDirectoryInternal(std::string(entryName), std::move(directory));
	}

	inline auto VirtualDirectory::StoreDirectory(const std::string_view& path, std::filesystem::path directoryPath) -> VirtualDirectoryEntry&
	{
		std::shared_ptr<VirtualDirectory> dir;
		std::string_view entryName;
		if (!RetrieveDirectory(path, true, dir, entryName))
			throw std::runtime_error("invalid path");

		return dir->StoreDirectoryInternal(std::string(entryName), std::move(directoryPath));
	}

	inline auto VirtualDirectory::StoreFile(const std::string_view& path, FileContentEntry file) -> FileContentEntry&
	{
		std::shared_ptr<VirtualDirectory> dir;
		std::string_view entryName;
		if (!RetrieveDirectory(path, true, dir, entryName))
			throw std::runtime_error("invalid path");

		return dir->StoreFileInternal(std::string(entryName), std::move(file));
	}

	inline auto VirtualDirectory::StoreFile(const std::string_view& path, std::filesystem::path filePath) -> PhysicalFileEntry&
	{
		std::shared_ptr<VirtualDirectory> dir;
		std::string_view entryName;
		if (!RetrieveDirectory(path, true, dir, entryName))
			throw std::runtime_error("invalid path");

		return dir->StoreFileInternal(std::string(entryName), std::move(filePath));
	}

	inline void VirtualDirectory::EnsureDots()
	{
		if (!m_wereDotRegistered)
		{
			StoreDirectoryInternal(".", shared_from_this());
			if (m_parent)
				StoreDirectoryInternal("..", m_parent);
			else
				StoreDirectoryInternal("..", shared_from_this());

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
				if (auto dir = std::get_if<VirtualDirectoryEntry>(&entry))
					directory = *dir;
				else
					return false;
			}
			else
			{
				if (allowCreation)
				{
					auto newDirectory = std::make_shared<VirtualDirectory>(directory);
					directory = directory->StoreDirectoryInternal(std::string(dirName), newDirectory);
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
		{
			if (m_physicalPath)
			{
				std::filesystem::path entryPath = *m_physicalPath / name;

				if (std::filesystem::is_regular_file(entryPath))
					entry->emplace<PhysicalFileEntry>(entryPath);
				else if (std::filesystem::is_directory(entryPath))
				{
					// FIXME: Allocating a shared_ptr on iteration is bad, not sure about a workaround
					*entry = StoreDirectoryInternal(std::string(name), entryPath);
				}
				else
					return false;

				return true;
			}

			return false;
		}
	}

	inline auto VirtualDirectory::StoreDirectoryInternal(std::string name, std::filesystem::path directoryPath) -> VirtualDirectoryEntry&
	{
		assert(name.find_first_of("\\/:") == name.npos);

		auto it = m_content.insert_or_assign(std::move(name), std::make_shared<VirtualDirectory>(directoryPath, shared_from_this())).first;
		return std::get<VirtualDirectoryEntry>(it->second);
	}

	inline auto VirtualDirectory::StoreDirectoryInternal(std::string name, VirtualDirectoryEntry directory) -> VirtualDirectoryEntry&
	{
		assert(name.find_first_of("\\/:") == name.npos);

		auto it = m_content.insert_or_assign(std::move(name), std::move(directory)).first;
		return std::get<VirtualDirectoryEntry>(it->second);
	}

	inline auto VirtualDirectory::StoreFileInternal(std::string name, FileContentEntry file) -> FileContentEntry&
	{
		assert(name.find_first_of("\\/:") == name.npos);

		auto it = m_content.insert_or_assign(std::move(name), std::move(file)).first;
		return std::get<FileContentEntry>(it->second);
	}

	inline auto VirtualDirectory::StoreFileInternal(std::string name, std::filesystem::path file) -> PhysicalFileEntry&
	{
		assert(name.find_first_of("\\/:") == name.npos);

		auto it = m_content.insert_or_assign(std::move(name), std::move(file)).first;
		return std::get<PhysicalFileEntry>(it->second);
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
