// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientScriptingContext.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <CoreLib/Utils.hpp>
#include <iostream>

namespace bw
{
	ClientScriptingContext::ClientScriptingContext(std::shared_ptr<VirtualDirectory> scriptDir) :
	m_scriptDirectory(std::move(scriptDir))
	{
	}

	bool ClientScriptingContext::Load(const std::filesystem::path& folderOrFile)
	{
		VirtualDirectory::Entry entry;
		if (!m_scriptDirectory->GetEntry(folderOrFile.generic_u8string(), &entry))
		{
			std::cerr << "Unknown path " << folderOrFile.generic_u8string() << std::endl;
			return false;
		}

		return std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry> || std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
			{
				m_currentFolder = folderOrFile.parent_path();

				sol::state& state = GetLuaState();
				sol::protected_function_result result;
				if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry>)
					result = state.do_string(std::string_view(reinterpret_cast<const char*>(arg.data()), arg.size()));
				else if constexpr (std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
				{
					std::vector<Nz::UInt8> content;

					Nz::File file(arg.generic_u8string());
					if (!file.Open(Nz::OpenMode_ReadOnly))
					{
						std::cerr << "Failed to load " << folderOrFile.generic_u8string() << ": failed to open file" << std::endl;
						return false;
					}

					content.resize(file.GetSize());
					if (file.Read(content.data(), content.size()) != content.size())
					{
						std::cerr << "Failed to load " << folderOrFile.generic_u8string() << ": failed to read file" << std::endl;
						return false;
					}

					result = state.do_string(std::string_view(reinterpret_cast<const char*>(content.data()), content.size()));
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive if");

				if (result.valid())
				{
					std::cout << "Loaded " << folderOrFile << std::endl;
					return true;
				}
				else
				{
					sol::error err = result;
					std::cerr << "Failed to load " << folderOrFile.generic_u8string() << ": " << err.what() << std::endl;
					return false;
				}
			}
			else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
			{
				arg->Foreach([&](const std::string& entryName, VirtualDirectory::Entry)
				{
					Load(folderOrFile / entryName);
				});

				return true;
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

		}, entry);
	}
}
