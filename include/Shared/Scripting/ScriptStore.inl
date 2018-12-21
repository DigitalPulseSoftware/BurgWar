// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ScriptStore.hpp>
#include <cassert>
#include <filesystem>

namespace bw
{
	template<typename Element>
	ScriptStore<Element>::ScriptStore(std::shared_ptr<Gamemode> gamemode, std::shared_ptr<SharedScriptingContext> context, bool isServer) :
	m_gamemode(std::move(gamemode)),
	m_context(std::move(context)),
	m_isServer(isServer)
	{
		assert(m_context);
	}

	template<typename Element>
	template<typename F>
	void ScriptStore<Element>::ForEachElement(const F& func) const
	{
		for (const auto& entity : m_elements)
			func(static_cast<const Element&>(*entity));
	}
	
	template<typename Element>
	const std::shared_ptr<Element>& ScriptStore<Element>::GetElement(std::size_t index) const
	{
		assert(index < m_elements.size());
		return m_elements[index];
	}

	template<typename Element>
	std::size_t ScriptStore<Element>::GetElementIndex(const std::string& name) const
	{
		auto it = m_elementsByName.find(name);
		if (it == m_elementsByName.end())
			return InvalidIndex;

		return it->second;
	}

	template<typename Element>
	bool ScriptStore<Element>::Load(const std::string& folder)
	{
		sol::state& state = GetLuaState();

		for (auto& p : std::filesystem::directory_iterator(folder))
		{
			if (p.is_regular_file() || p.is_directory())
			{
				std::string elementName;
				if (p.is_regular_file())
					elementName = p.path().stem().u8string();
				else
					elementName = p.path().filename().u8string();

				sol::table elementTable = state.create_table();
				elementTable["__index"] = elementTable;

				elementTable["Name"] = elementName;

				InitializeElementTable(elementTable);

				state[m_tableName] = elementTable;

				std::cout << "Loading " << m_elementTypeName << ": " << elementName << std::endl;

				bool hasError = false;
				auto Load = [&](const std::filesystem::path& path)
				{
					if (!std::filesystem::exists(path))
						return false;

					if (m_context->Load(path))
						return true;
					else
					{
						std::cerr << path << " failed: " << state.GetLastError() << std::endl;
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

					if (m_isServer)
						Load(folderPath / "sv_init.lua");
					else
						Load(folderPath / "cl_init.lua");
				}

				std::shared_ptr<Element> element = std::make_shared<Element>();
				element->name = std::move(elementName);
				element->fullName = m_elementTypeName + "_" + element->name;
				element->elementTable = std::move(elementTable);

				element->tickFunction = element->elementTable["OnTick"];

				try
				{
					InitializeElement(element->elementTable, *element);
				}
				catch (const std::exception& e)
				{
					std::cerr << "Failed to initialize " << m_elementTypeName << " " << elementName << ": " << e.what() << std::endl;
				}

				//if (IsServer && !isNetworked && hasSharedFiles)
				//	std::cerr << "Warning: " << m_elementTypeName << " " << elementName << " has client-side files but is not marked as networked, this is likely an error" << std::endl;

				m_elementsByName[element->fullName] = m_elements.size();
				m_elements.emplace_back(std::move(element));
			}
		}

		state.PushNil();
		state.SetGlobal(m_tableName);

		return true;
	}

	template<typename Element>
	sol::state& ScriptStore<Element>::GetLuaState()
	{
		return m_context->GetLuaState();
	}

	template<typename Element>
	const std::shared_ptr<Gamemode>& ScriptStore<Element>::GetGamemode()
	{
		return m_gamemode;
	}

	template<typename Element>
	const std::shared_ptr<SharedScriptingContext>& ScriptStore<Element>::GetScriptingContext()
	{
		return m_context;
	}

	template<typename Element>
	void ScriptStore<Element>::SetElementTypeName(std::string typeName)
	{
		m_elementTypeName = std::move(typeName);
	}

	template<typename Element>
	void ScriptStore<Element>::SetTableName(std::string tableName)
	{
		m_tableName = std::move(tableName);
	}
}
