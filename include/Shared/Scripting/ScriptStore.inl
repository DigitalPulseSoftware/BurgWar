// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ScriptStore.hpp>
#include <cassert>
#include <filesystem>

namespace bw
{
	template<typename Element, bool IsServer>
	ScriptStore<Element, IsServer>::ScriptStore(Nz::LuaState& state) :
	m_state(state)
	{
	}

	template<typename Element, bool IsServer>
	template<typename F>
	void ScriptStore<Element, IsServer>::ForEachElement(const F& func) const
	{
		for (const auto& entity : m_elements)
			func(entity);
	}
	
	template<typename Element, bool IsServer>
	const Element& ScriptStore<Element, IsServer>::GetElement(std::size_t index) const
	{
		assert(index < m_elements.size());
		return m_elements[index];
	}

	template<typename Element, bool IsServer>
	std::size_t ScriptStore<Element, IsServer>::GetElementIndex(const std::string& name) const
	{
		auto it = m_elementsByName.find(name);
		if (it == m_elementsByName.end())
			return InvalidIndex;

		return it->second;
	}

	template<typename Element, bool IsServer>
	bool ScriptStore<Element, IsServer>::Load(const std::string& folder)
	{
		for (auto& p : std::filesystem::directory_iterator(folder))
		{
			if (p.is_regular_file() || p.is_directory())
			{
				std::string elementName;
				if (p.is_regular_file())
					elementName = p.path().stem().u8string();
				else
					elementName = p.path().filename().u8string();

				m_state.PushTable();
				m_state.PushValue(-1);
				int tableRef = m_state.CreateReference();
				Nz::CallOnExit destroyRef([&]()
				{
					m_state.DestroyReference(tableRef);
				});

				m_state.PushField("Name", elementName);

				InitializeElementTable(m_state);

				m_state.SetGlobal(m_tableName);

				std::cout << "Loading " << m_elementTypeName << ": " << elementName << std::endl;

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
					// Element script
					Load(p);
				}
				else
				{
					// Element folder
					const std::filesystem::path& folderPath = p;

					if (Load(folderPath / "shared.lua"))
						hasSharedFiles = true;

					if constexpr (IsServer)
						Load(folderPath / "sv_init.lua");
					else
						Load(folderPath / "cl_init.lua");
				}

				Element element;
				element.name = std::move(elementName);
				element.fullName = m_elementTypeName + "_" + element.name;
				element.tableRef = tableRef;

				m_state.PushReference(tableRef);

				try
				{
					InitializeElement(m_state, element);
				}
				catch (const std::exception& e)
				{
					std::cerr << "Failed to initialize " << m_elementTypeName << " " << elementName << ": " << e.what() << std::endl;
				}

				m_state.Pop();

				//if (IsServer && !isNetworked && hasSharedFiles)
				//	std::cerr << "Warning: " << m_elementTypeName << " " << elementName << " has client-side files but is not marked as networked, this is likely an error" << std::endl;

				destroyRef.Reset();

				m_elementsByName[element.fullName] = m_elements.size();
				m_elements.emplace_back(std::move(element));
			}
		}

		m_state.PushNil();
		m_state.SetGlobal(m_tableName);

		return true;
	}

	template<typename Element, bool IsServer>
	Nz::LuaState & ScriptStore<Element, IsServer>::GetState()
	{
		return m_state;
	}

	template<typename Element, bool IsServer>
	void ScriptStore<Element, IsServer>::SetElementTypeName(std::string typeName)
	{
		m_elementTypeName = std::move(typeName);
	}

	template<typename Element, bool IsServer>
	void ScriptStore<Element, IsServer>::SetTableName(std::string tableName)
	{
		m_tableName = std::move(tableName);
	}
}
