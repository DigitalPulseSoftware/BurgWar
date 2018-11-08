// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/EntityStore.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <filesystem>
#include <iostream>

namespace bw
{
	bool EntityStore::Load(const std::string& folder)
	{
		for (auto& p : std::filesystem::directory_iterator(folder))
		{
			if (p.is_regular_file() || p.is_directory())
			{
				std::string entityName;
				if (p.is_regular_file())
					entityName = p.path().stem().u8string();
				else
					entityName = p.path().filename().u8string();

				m_state.PushTable();
				m_state.PushValue(-1);
				int tableRef = m_state.CreateReference();
				Nz::CallOnExit destroyRef([&]()
				{
					m_state.DestroyReference(tableRef);
				});

				m_state.PushField("Name", entityName);

				m_state.PushField("IsNetworked", false);

				m_state.SetGlobal("ENTITY");

				std::cout << "Loading entity: " << entityName << std::endl;

				bool hasError = false;
				auto Load = [&](const std::filesystem::path& path)
				{
					if (!std::filesystem::exists(path))
						return false;

					if (m_state.ExecuteFromFile(path.generic_u8string()))
						return true;
					else
					{
						std::cerr << path << " failed: " << m_state.GetLastError() << std::endl;
						hasError = true;
						return false;
					}

				};

				bool hasSharedFiles = false;
				if (p.is_regular_file())
				{
					// Entity script
					Load(p);
				}
				else
				{
					// Entity folder
					entityName = p.path().filename().u8string();

					const std::filesystem::path& folderPath = p;

					if (Load(folderPath / "shared.lua"))
						hasSharedFiles = true;

					if (m_isServer)
						Load(folderPath / "sv_init.lua");
					else
						Load(folderPath / "cl_init.lua");
				}

				m_state.PushReference(tableRef);
				bool isNetworked;
				try
				{
					isNetworked = m_state.CheckField<bool>("IsNetworked");
				}
				catch (const std::exception& e)
				{
					std::cerr << "Invalid IsNetworked value: " << e.what() << std::endl;
					isNetworked = false;
				}

				m_state.Pop();

				if (m_isServer && !isNetworked && hasSharedFiles)
					std::cerr << "Warning: entity " << entityName << " has client-side files but is not marked as networked, this is likely an error" << std::endl;

				destroyRef.Reset();

				m_entitiesByName[entityName] = m_entities.size();
				Entity& entity = m_entities.emplace_back();
				entity.isNetworked = isNetworked;
				entity.name = std::move(entityName);
				entity.tableRef = tableRef;
			}
		}

		m_state.PushNil();
		m_state.SetGlobal("ENTITY");

		return true;
	}
}
