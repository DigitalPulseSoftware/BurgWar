// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptStore.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/Utility/VirtualDirectory.hpp>
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
	bool ScriptStore<Element>::Load(const std::filesystem::path& directoryPath, const std::shared_ptr<VirtualDirectory>& directory)
	{
		directory->Foreach([&](const std::string& entryName, const VirtualDirectory::Entry& entry)
		{
			LoadElement(std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry), directoryPath / entryName);
		});

		sol::state& state = GetLuaState();
		state[m_tableName] = nullptr;

		return true;
	}

	template<typename Element>
	bool ScriptStore<Element>::Load(const std::filesystem::path& directoryPath)
	{
		for (auto& p : std::filesystem::directory_iterator(directoryPath))
			LoadElement(p.is_directory(), p.path());

		sol::state& state = GetLuaState();
		state[m_tableName] = nullptr;

		return true;
	}

	template<typename Element>
	inline bool ScriptStore<Element>::LoadElement(bool isDirectory, const std::filesystem::path& elementPath)
	{
		sol::state& state = GetLuaState();

		std::string elementName;
		if (!isDirectory)
			elementName = elementPath.stem().u8string();
		else
			elementName = elementPath.filename().u8string();

		sol::table elementTable = state.create_table();
		elementTable["__index"] = elementTable;

		elementTable["Name"] = elementName;

		elementTable["GetProperty"] = [](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			sol::state_view lua(s);

			const Ndk::EntityHandle& entity = table["Entity"];

			auto& properties = entity->GetComponent<ScriptComponent>();

			auto propertyVal = properties.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				const EntityProperty& property = propertyVal.value();
				assert(!std::holds_alternative<std::monostate>(property));

				return std::visit([&](auto&& value) -> sol::object
				{
					using T = std::decay_t<decltype(value)>;

					if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, float> || std::is_same_v<T, Nz::Int64> || std::is_same_v<T, std::string>)
						return sol::make_object(lua, value);
					else if constexpr (std::is_same_v<T, std::monostate>)
						throw std::runtime_error("Unexpected monostate in property");
					else
						static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				}, property);
			}
			else
				return sol::nil;
		};

		InitializeElementTable(elementTable);

		state[m_tableName] = elementTable;

		std::cout << "Loading " << m_elementTypeName << ": " << elementName << std::endl;

		bool hasError = false;
		auto LoadFile = [&](const std::filesystem::path& path)
		{
			if (m_context->Load(path))
				return true;
			else
			{
				std::cerr << path << " failed" << std::endl;
				hasError = true;
				return false;
			}

		};

		bool hasSharedFiles = false;
		if (!isDirectory)
		{
			// Element script
			LoadFile(elementPath);
		}
		else
		{
			// Element folder
			if (LoadFile(elementPath / "shared.lua"))
				hasSharedFiles = true;

			if (m_isServer)
				LoadFile(elementPath / "sv_init.lua");
			else
				LoadFile(elementPath / "cl_init.lua");
		}

		std::shared_ptr<Element> element = std::make_shared<Element>();
		element->name = std::move(elementName);
		element->fullName = m_elementTypeName + "_" + element->name;
		element->elementTable = std::move(elementTable);
		element->tickFunction = element->elementTable["OnTick"];

		sol::object properties = element->elementTable["Properties"];
		if (properties)
		{
			sol::table elementProperties = properties.as<sol::table>();

			for (const auto& kv : elementProperties)
			{
				sol::table propertyTable = kv.second;

				std::string propertyName = propertyTable["Name"];

				try
				{
					ScriptedElement::Property property;

					property.type = propertyTable["Type"];
					
					sol::object propertyShared = propertyTable["Shared"];
					if (propertyShared)
						property.shared = propertyShared.as<bool>();

					sol::object propertyDefault = propertyTable["Default"];
					if (propertyDefault)
					{
						// Waiting for C++20 template lambda

						auto PropertyChecker = [&](auto dummyType, std::initializer_list<PropertyType> expectedTypes)
						{
							using T = std::decay_t<decltype(dummyType)>;

							if (propertyDefault.is<T>())
							{
								if (std::find(expectedTypes.begin(), expectedTypes.end(), property.type) == expectedTypes.end())
									throw std::runtime_error("Property " + propertyName + " default value doesn't match property type");

								property.defaultValue = propertyDefault.as<T>();
								return true;
							}
							else
								return false;
						};

						if (!PropertyChecker(bool(), { PropertyType::Bool }) &&
							!PropertyChecker(float(), { PropertyType::Float }) &&
							!PropertyChecker(Nz::Int64(), { PropertyType::Integer }) &&
							!PropertyChecker(std::string(), { PropertyType::String, PropertyType::Texture }))
						{
							throw std::runtime_error("Property " + propertyName + " default value has unhandled type");
						}
					}

					auto it = element->properties.find(propertyName);
					if (it == element->properties.end())
						element->properties.emplace(std::move(propertyName), std::move(property));
					else
						throw std::runtime_error("Property " + propertyName + " found twice");
				}
				catch (const std::exception& e)
				{
					std::cerr << "Failed to load property " << propertyName << " for entity " << element->name << ": " << e.what() << std::endl;
				}
			}
		}

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

		return true;
	}

	template<typename Element>
	const Ndk::EntityHandle& ScriptStore<Element>::CreateEntity(Ndk::World& world, std::shared_ptr<const ScriptedElement> element, const EntityProperties& properties)
	{
		const Ndk::EntityHandle& entity = world.CreateEntity();

		EntityProperties filteredProperties; //< Without potential unused properties (FIXME: Is it really necessary?)

		for (auto&& [propertyName, propertyInfo] : element->properties)
		{
			if (auto it = properties.find(propertyName); it != properties.end())
			{
				// Property exists, check its type

				//TODO: Check property type

				filteredProperties[propertyName] = it->second;
			}
			else
			{
				// Property doesn't exist, check for it's default value
				if (std::holds_alternative<std::monostate>(propertyInfo.defaultValue))
					throw std::runtime_error("Missing property " + propertyName);
			}
		}

		const auto& scriptingContext = GetScriptingContext();

		sol::state& state = scriptingContext->GetLuaState();

		sol::table entityTable = state.create_table();
		entityTable["Entity"] = entity;
		entityTable[sol::metatable_key] = element->elementTable;

		entity->AddComponent<ScriptComponent>(std::move(element), scriptingContext, std::move(entityTable), std::move(filteredProperties));

		return entity;
	}

	template<typename Element>
	sol::state& ScriptStore<Element>::GetLuaState()
	{
		return m_context->GetLuaState();
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
