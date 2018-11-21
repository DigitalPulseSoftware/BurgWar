// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ScriptStore.hpp>
#include <cassert>
#include <filesystem>

namespace bw
{
	template<typename Element>
	ScriptStore<Element>::ScriptStore(std::shared_ptr<SharedScriptingContext> context, bool isServer) :
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
			func(entity);
	}
	
	template<typename Element>
	const Element& ScriptStore<Element>::GetElement(std::size_t index) const
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
		Nz::LuaState& state = GetLuaState();

		for (auto& p : std::filesystem::directory_iterator(folder))
		{
			if (p.is_regular_file() || p.is_directory())
			{
				std::string elementName;
				if (p.is_regular_file())
					elementName = p.path().stem().u8string();
				else
					elementName = p.path().filename().u8string();

				state.PushTable();
				state.PushValue(-1);
				int tableRef = state.CreateReference();
				Nz::CallOnExit destroyRef([&]()
				{
					state.DestroyReference(tableRef);
				});

				// TABLE.__index = TABLE
				state.PushValue(-1);
				state.SetField("__index");

				state.PushField("Name", elementName);

				InitializeElementTable(state);

				state.SetGlobal(m_tableName);

				std::cout << "Loading " << m_elementTypeName << ": " << elementName << std::endl;

				bool hasError = false;
				auto Load = [&](const std::filesystem::path& path)
				{
					if (!std::filesystem::exists(path))
						return false;

					if (state.ExecuteFromFile(path.generic_u8string()))
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

				Element element;
				element.name = std::move(elementName);
				element.fullName = m_elementTypeName + "_" + element.name;
				element.tableRef = tableRef;

				state.PushReference(tableRef);

				try
				{
					InitializeElement(state, element);
				}
				catch (const std::exception& e)
				{
					std::cerr << "Failed to initialize " << m_elementTypeName << " " << elementName << ": " << e.what() << std::endl;
				}

				state.Pop();

				//if (IsServer && !isNetworked && hasSharedFiles)
				//	std::cerr << "Warning: " << m_elementTypeName << " " << elementName << " has client-side files but is not marked as networked, this is likely an error" << std::endl;

				destroyRef.Reset();

				m_elementsByName[element.fullName] = m_elements.size();
				m_elements.emplace_back(std::move(element));
			}
		}

		state.PushNil();
		state.SetGlobal(m_tableName);

		return true;
	}

	template<typename Element>
	Nz::LuaState& ScriptStore<Element>::GetLuaState()
	{
		return m_context->GetLuaInstance();
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
