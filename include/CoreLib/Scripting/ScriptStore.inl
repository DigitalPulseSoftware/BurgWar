// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptStore.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <NDK/World.hpp>
#include <cassert>
#include <filesystem>

namespace bw
{
	template<typename Element>
	ScriptStore<Element>::ScriptStore(const Logger& logger, std::shared_ptr<ScriptingContext> context, bool isServer) :
	m_context(std::move(context)),
	m_logger(logger),
	m_isServer(isServer)
	{
		assert(m_context);
	}

	template<typename Element>
	void ScriptStore<Element>::ClearElements()
	{
		m_elements.clear();
		m_elementsByName.clear();
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
	sol::table& ScriptStore<Element>::GetElementMetatable()
	{
		return m_elementMetatable;
	}

	template<typename Element>
	const Logger& ScriptStore<Element>::GetLogger() const
	{
		return m_logger;
	}

	template<typename Element>
	inline void ScriptStore<Element>::UpdateEntityElement(const Ndk::EntityHandle& entity)
	{
		assert(entity->HasComponent<ScriptComponent>());

		auto& entityScript = entity->GetComponent<ScriptComponent>();
		const auto& entityElement = entityScript.GetElement();

		if (auto it = m_elementsByName.find(entityElement->fullName); it != m_elementsByName.end())
		{
			const auto& newElement = m_elements[it->second];

			sol::table& entityTable = entityScript.GetTable();
			entityTable[sol::metatable_key] = newElement->elementTable;

			entityScript.UpdateElement(newElement);
		}
	}

	template<typename Element>
	void ScriptStore<Element>::LoadDirectory(const std::filesystem::path& directoryPath)
	{
		const auto& scriptDir = m_context->GetScriptDirectory();

		VirtualDirectory::Entry entry;
		if (scriptDir->GetEntry(directoryPath.generic_u8string(), &entry) && std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry))
		{
			VirtualDirectory::VirtualDirectoryEntry& directory = std::get<VirtualDirectory::VirtualDirectoryEntry>(entry);
			directory->Foreach([&](const std::string& entryName, const VirtualDirectory::Entry& entry)
			{
				LoadElement(std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry), directoryPath / entryName);
			});
		}
	}

	template<typename Element>
	bool ScriptStore<Element>::LoadElement(bool isDirectory, const std::filesystem::path& elementPath)
	{
		std::string elementName;
		if (!isDirectory)
			elementName = elementPath.stem().u8string();
		else
			elementName = elementPath.filename().u8string();

		CurrentElementData elementData;
		elementData.name = std::move(elementName);
		elementData.fullName = m_elementTypeName + "_" + elementData.name;

		m_currentElementData = &elementData;
		Nz::CallOnExit resetOnExit([&] { m_currentElementData = nullptr; });

		bwLog(m_logger, LogLevel::Info, "Loading {0} {1}", m_elementTypeName, elementData.name);

		bool hasError = false;
		auto LoadFile = [&](const std::filesystem::path& path)
		{
			if (m_context->Load(path))
				return true;
			else
			{
				bwLog(m_logger, LogLevel::Error, "{0} loading failed", path.generic_u8string());
				hasError = true;
				return false;
			}
		};

		if (!isDirectory)
		{
			// Element script
			LoadFile(elementPath);
		}
		else
		{
			// Element folder
			LoadFile(elementPath / "shared.lua");

			if (m_isServer)
				LoadFile(elementPath / "sv_init.lua");
			else
				LoadFile(elementPath / "cl_init.lua");
		}

		if (!elementData.element || hasError)
			return false;

		std::string& baseElement = elementData.element->base;
		baseElement = elementData.element->elementTable.get_or("Base", std::string());

		// If no base element (or element already loaded), initialize it now
		if (baseElement.empty() || m_elementsByName.find(baseElement) != m_elementsByName.end())
			return RegisterElement(std::move(elementData.element));
		else
		{
			// Else, add it in a pending list
			auto& pendingElements = m_pendingElements[baseElement];
			pendingElements.emplace_back(std::move(elementData.element));

			return true;
		}
	}

	template<typename Element>
	void ScriptStore<Element>::LoadLibrary(std::shared_ptr<AbstractElementLibrary> library)
	{
		library->RegisterLibrary(m_elementMetatable);

		m_libraries.emplace_back(std::move(library));
	}

	template<typename Element>
	void ScriptStore<Element>::ReloadLibraries()
	{
		sol::state& state = GetLuaState();

		m_elementMetatable = state.create_table();
		m_elementMetatable["__index"] = m_elementMetatable;

		for (const auto& libPtr : m_libraries)
			libPtr->RegisterLibrary(m_elementMetatable);

		// Link new metatables
		for (const auto& elementPtr : m_elements)
			elementPtr->elementTable[sol::metatable_key] = m_elementMetatable;

		state["Scripted" + m_elementName] = [this](std::optional<sol::table> parameters)
		{
			if (!m_currentElementData)
				throw std::runtime_error("you can only call this function in a scripted " + m_elementTypeName + " file");

			if (parameters.has_value())
				return CreateElement(std::move(parameters.value()));
			else
				return GetElementTable();
		};
	}

	template<typename Element>
	void ScriptStore<Element>::Resolve()
	{
		bool continueResolving;

		do 
		{
			continueResolving = false;

			// Cries in topology sort
			for (auto it = m_pendingElements.begin(); it != m_pendingElements.end();)
			{
				const auto& dependency = it.key();
				if (m_elementsByName.find(dependency) != m_elementsByName.end())
				{
					auto& elements = it.value();

					for (auto&& element : elements)
					{
						if (RegisterElement(std::move(element)))
							continueResolving = true;
					}

					it = m_pendingElements.erase(it);
				}
				else
					++it;
			}
		}
		while (continueResolving);

		for (const auto& [dependency, elements] : m_pendingElements)
		{
			for (auto&& element : elements)
				bwLog(m_logger, LogLevel::Error, "Failed to initialize {0} {1}: dependency {2} does not exist", m_elementTypeName, element->name, dependency);
		}
		m_pendingElements.clear();
	}

	template<typename Element>
	std::shared_ptr<Element> ScriptStore<Element>::CreateElement() const
	{
		return std::make_shared<Element>();
	}

	template<typename Element>
	const Ndk::EntityHandle& ScriptStore<Element>::CreateEntity(Ndk::World& world, std::shared_ptr<const ScriptedElement> element, const PropertyValueMap& properties) const
	{
		const Ndk::EntityHandle& entity = world.CreateEntity();

		PropertyValueMap filteredProperties; //< Without potential unused properties (FIXME: Is it really necessary?)

		for (auto&& [propertyName, propertyInfo] : element->properties)
		{
			if (auto it = properties.find(propertyName); it != properties.end())
			{
				// Property exists, check its type

				//TODO: Check property type

				filteredProperties.emplace(propertyName, it->second);
			}
			else
			{
				// Property doesn't exist, check for it's default value
				if (!propertyInfo.defaultValue)
					throw std::runtime_error("Missing property " + propertyName);
			}
		}

		const auto& scriptingContext = GetScriptingContext();

		sol::state& state = scriptingContext->GetLuaState();

		sol::table entityTable = state.create_table();
		entityTable["_Entity"] = entity;
		entityTable[sol::metatable_key] = element->elementTable;

		entity->AddComponent<ScriptComponent>(m_logger, std::move(element), scriptingContext, std::move(entityTable), std::move(filteredProperties));

		return entity;
	}

	template<typename Element>
	void ScriptStore<Element>::InitializeElementTable(sol::table& elementTable)
	{
		assert(m_elementMetatable);

		elementTable[sol::metatable_key] = m_elementMetatable;
		elementTable["__index"] = elementTable;

		elementTable["FullName"] = m_currentElementData->element->fullName;
		elementTable["Name"] = m_currentElementData->element->name;

		elementTable["_Element"] = m_currentElementData->element;
	}

	template<typename Element>
	sol::table ScriptStore<Element>::CreateElement(sol::table initTable)
	{
		assert(m_currentElementData);
		assert(initTable.valid());

		if (m_currentElementData->element)
			throw std::runtime_error("you can only initialize an " + m_elementTypeName + " once");

		m_currentElementData->element = CreateElement();
		m_currentElementData->element->fullName = std::move(m_currentElementData->fullName);
		m_currentElementData->element->name = std::move(m_currentElementData->name);

		InitializeElementTable(initTable);

		m_currentElementData->element->elementTable = std::move(initTable);

		return m_currentElementData->element->elementTable;
	}

	template<typename Element>
	sol::table ScriptStore<Element>::GetElementTable()
	{
		assert(m_currentElementData);

		if (!m_currentElementData->element)
			throw std::runtime_error("you must initialize the " + m_elementTypeName + " first");

		return m_currentElementData->element->elementTable;
	}

	template<typename Element>
	std::size_t ScriptStore<Element>::HandleProperties(const std::shared_ptr<Element>& element, Element* baseElement)
	{
		std::size_t propertyIndex = 0;
		if (!baseElement->base.empty())
		{
			auto it = m_elementsByName.find(element->base);
			assert(it != m_elementsByName.end());

			Element* parentElement = m_elements[it->second].get();
			
			// Merge parent events
			for (std::size_t i = 0; i < ElementEventCount; ++i)
			{
				const auto& parentCallbacks = parentElement->events[i];
				auto& callbacks = element->events[i];

				std::copy(parentCallbacks.rbegin(), parentCallbacks.rend(), std::inserter(callbacks, callbacks.begin()));
			}

			propertyIndex += HandleProperties(element, parentElement);
		}

		sol::object properties = baseElement->elementTable.raw_get<sol::object>("Properties"); //< raw get as we don't want to fetch from the base
		if (properties)
		{
			sol::table elementProperties = properties.as<sol::table>();

			for (const auto& kv : elementProperties)
			{
				sol::table propertyTable = kv.second;

				std::string propertyName = propertyTable["Name"];

				try
				{
					ScriptedProperty property;
					property.index = propertyIndex;
					property.type = propertyTable["Type"];

					sol::object propertyShared = propertyTable["Shared"];
					if (propertyShared)
						property.shared = propertyShared.as<bool>();

					sol::object propertyArray = propertyTable["Array"];
					if (propertyArray)
						property.isArray = propertyArray.as<bool>();

					sol::object propertyDefault = propertyTable["Default"];
					if (!propertyDefault.is<sol::nil_t>())
						property.defaultValue = TranslatePropertyFromLua(nullptr, propertyDefault, property.type, property.isArray);

					auto it = element->properties.find(propertyName);
					if (it == element->properties.end())
						element->properties.emplace(std::move(propertyName), std::move(property));
					else
						throw std::runtime_error("Property " + propertyName + " already exists");
				}
				catch (const std::exception& e)
				{
					bwLog(m_logger, LogLevel::Error, "Failed to load property {0} for entity {1}: {2}", propertyName, element->name, e.what());
				}

				propertyIndex++;
			}
		}

		return propertyIndex;
	}

	template<typename Element>
	bool ScriptStore<Element>::RegisterElement(std::shared_ptr<Element> element)
	{
		std::shared_ptr<Element> baseElement;
		if (!element->base.empty())
		{
			auto it = m_elementsByName.find(element->base);
			assert(it != m_elementsByName.end());

			baseElement = m_elements[it->second];

			element->elementTable["Base"] = baseElement->elementTable;
			element->elementTable[sol::metatable_key] = baseElement->elementTable;
		}

		HandleProperties(element, element.get());

		try
		{
			InitializeElement(element->elementTable, *element);
		}
		catch (const std::exception& e)
		{
			bwLog(m_logger, LogLevel::Error, "Failed to initialize {0} {1}: {2}", m_elementTypeName, element->name, e.what());
			return false;
		}

		m_elementsByName[element->fullName] = m_elements.size();
		m_elements.emplace_back(std::move(element));

		return true;
	}

	template<typename Element>
	bool ScriptStore<Element>::InitializeEntity(const Element& entityClass, const Ndk::EntityHandle& entity) const
	{
		auto& entityScript = entity->GetComponent<ScriptComponent>();
		if (!entityScript.ExecuteCallback<ElementEvent::Init>())
		{
			//TODO: Retrieve error message
			bwLog(m_logger, LogLevel::Error, "Failed to initialize {0} {1}", m_elementTypeName, entityClass.name);
			return false;
		}

		return true;
	}
	template<typename Element>
	sol::state& ScriptStore<Element>::GetLuaState()
	{
		return m_context->GetLuaState();
	}

	template<typename Element>
	const std::shared_ptr<ScriptingContext>& ScriptStore<Element>::GetScriptingContext() const
	{
		return m_context;
	}

	template<typename Element>
	void ScriptStore<Element>::SetElementTypeName(std::string typeName)
	{
		m_elementTypeName = std::move(typeName);
	}

	template<typename Element>
	void ScriptStore<Element>::SetElementName(std::string elementName)
	{
		m_elementName = std::move(elementName);
	}
}
