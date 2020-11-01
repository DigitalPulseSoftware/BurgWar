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
#include <sstream>

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
	bool ScriptStore<Element>::LoadElement(bool isDirectory, std::filesystem::path elementPath)
	{
		std::string elementName;
		if (!isDirectory)
			elementName = elementPath.stem().u8string();
		else
			elementName = elementPath.filename().u8string();

		CurrentElementData elementData;
		elementData.elementPath = std::move(elementPath);
		elementData.name = std::move(elementName);
		elementData.fullName = m_elementTypeName + "_" + elementData.name;
		elementData.directory = isDirectory;

		m_currentElementData = &elementData;
		Nz::CallOnExit resetOnExit([&] { m_currentElementData = nullptr; });

		bwLog(m_logger, LogLevel::Info, "Loading {0} {1}", m_elementTypeName, elementData.name);

		std::optional<ScriptingContext::FileLoadCoroutine> fileLoadCoOpt;

		if (isDirectory)
			fileLoadCoOpt = m_context->Load(elementData.elementPath / "shared.lua", ScriptingContext::Async{});
		else
			fileLoadCoOpt = m_context->Load(elementData.elementPath, ScriptingContext::Async{});

		if (!fileLoadCoOpt)
		{
			bwLog(m_logger, LogLevel::Error, "{0} loading failed", elementPath.generic_u8string());
			return false;
		}

		elementData.fileCoro = std::move(fileLoadCoOpt.value());

		if (!m_context->Exec(elementData.fileCoro))
		{
			bwLog(m_logger, LogLevel::Error, "{0} loading failed", elementPath.generic_u8string());
			return false;
		}

		if (isDirectory)
		{
			const char* fileName = (m_isServer) ? "sv_init.lua" : "cl_init.lua";

			if (!m_context->Load(elementData.elementPath / fileName))
			{
				bwLog(m_logger, LogLevel::Error, "{0} loading failed", elementData.elementPath.generic_u8string());
				return false;
			}
		}

		return true;
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

		state["Scripted" + m_elementName] = [this](sol::this_state L, std::optional<sol::table> parameters)
		{
			if (!m_currentElementData)
				throw std::runtime_error("you can only call this function in a scripted " + m_elementTypeName + " file");

			if (parameters.has_value())
				return CreateElement(L, std::move(parameters.value()));
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

					for (auto&& pendingElement : elements)
					{
						if (!m_context->Exec(pendingElement.fileCoro, pendingElement.element->elementTable))
						{
							bwLog(m_logger, LogLevel::Error, "{0} loading failed", pendingElement.elementPath.generic_u8string());
							continue;
						}

						if (pendingElement.directory)
						{
							const char* fileName = (m_isServer) ? "sv_init.lua" : "cl_init.lua";

							if (!m_context->Load(pendingElement.elementPath / fileName))
							{
								bwLog(m_logger, LogLevel::Error, "{0} loading failed", pendingElement.elementPath.generic_u8string());
								continue;
							}
						}

						if (RegisterElement(std::move(pendingElement.element)))
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
			for (auto&& pendingElement : elements)
				bwLog(m_logger, LogLevel::Error, "Failed to initialize {0} {1}: dependency {2} does not exist", m_elementTypeName, pendingElement.element->name, dependency);
		}
		m_pendingElements.clear();
	}

	template<typename Element>
	std::shared_ptr<Element> ScriptStore<Element>::CreateElement() const
	{
		return std::make_shared<Element>();
	}

	template<typename Element>
	const Ndk::EntityHandle& ScriptStore<Element>::CreateEntity(Ndk::World& world, std::shared_ptr<const ScriptedElement> element, PropertyValueMap properties) const
	{
		const Ndk::EntityHandle& entity = world.CreateEntity();

		PropertyValueMap filteredProperties; //< Without potential unused properties (FIXME: Is it really necessary?)

		for (auto&& [propertyName, propertyInfo] : element->properties)
		{
			if (auto it = properties.find(propertyName); it != properties.end())
			{
				auto&& value = std::move(it.value());

				auto [propertyType, isArray] = ExtractPropertyType(value);

				if (propertyInfo.type != propertyType || propertyInfo.isArray != isArray)
				{
					std::ostringstream ss;
					ss << "property " << propertyName << ": expected ";

					if (propertyInfo.isArray)
						ss << "an array of ";
					else
						ss << "a single ";

					ss << ToString(propertyInfo.type) << ", got ";

					if (isArray)
						ss << "an array of ";
					else
						ss << "a single ";

					ss << ToString(propertyType);

					throw std::runtime_error(std::move(ss).str());
				}

				filteredProperties.emplace(propertyName, std::move(value));
			}
			else
			{
				// Property doesn't exist, check for it's default value
				if (!propertyInfo.defaultValue)
					throw std::runtime_error("missing mandatory property " + propertyName);
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
	void ScriptStore<Element>::InitializeElementTable(sol::main_table& elementTable)
	{
		assert(m_elementMetatable);

		elementTable[sol::metatable_key] = m_elementMetatable;
		elementTable["__index"] = elementTable;

		elementTable["FullName"] = m_currentElementData->element->fullName;
		elementTable["Name"] = m_currentElementData->element->name;

		elementTable["_Element"] = m_currentElementData->element;
	}

	template<typename Element>
	sol::table ScriptStore<Element>::CreateElement(lua_State* L, sol::table initTable)
	{
		assert(m_currentElementData);
		assert(initTable.valid());

		if (m_currentElementData->element)
			throw std::runtime_error("you can only initialize an " + m_elementTypeName + " once");

		std::shared_ptr<Element>& element = m_currentElementData->element;
		element = CreateElement();
		element->fullName = std::move(m_currentElementData->fullName);
		element->name = std::move(m_currentElementData->name);

		element->elementTable = std::move(initTable);

		InitializeElementTable(element->elementTable);

		std::string& baseElement = element->base;
		baseElement = element->elementTable.get_or("Base", std::string());

		// If no base element (or element already loaded), initialize it now
		if (baseElement.empty() || m_elementsByName.find(baseElement) != m_elementsByName.end())
			RegisterElement(element);
		else
		{
			// Else, add it in a pending list
			auto& pendingElements = m_pendingElements[baseElement];
			auto& pendingElementData = pendingElements.emplace_back();
			pendingElementData.directory = m_currentElementData->directory;
			pendingElementData.element = std::move(element);
			pendingElementData.elementPath = std::move(m_currentElementData->elementPath);
			pendingElementData.fileCoro = m_currentElementData->fileCoro; //< Don't move since we're inside this coroutine

			lua_yield(L, 0);
		}

		return element->elementTable;
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
	void ScriptStore<Element>::RegisterCustomEvents(const std::shared_ptr<Element>& element, Element* baseElement)
	{
		if (!baseElement->base.empty())
		{
			auto it = m_elementsByName.find(element->base);
			assert(it != m_elementsByName.end());

			Element* parentElement = m_elements[it->second].get();

			// Merge parent regular events
			for (std::size_t i = 0; i < ElementEventCount; ++i)
			{
				const auto& parentCallbacks = parentElement->eventCallbacks[i];
				auto& callbacks = element->eventCallbacks[i];

				std::copy(parentCallbacks.rbegin(), parentCallbacks.rend(), std::inserter(callbacks, callbacks.begin()));
			}

			// Merge parent custom events
			for (std::size_t i = 0; i < parentElement->customEventCallbacks.size(); ++i)
			{
				const auto& parentCallbacks = parentElement->customEventCallbacks[i];

				if (element->customEventCallbacks.size() <= i)
					element->customEventCallbacks.resize(i + 1);

				auto& callbacks = element->customEventCallbacks[i];

				std::copy(parentCallbacks.rbegin(), parentCallbacks.rend(), std::inserter(callbacks, callbacks.begin()));
			}

			RegisterCustomEvents(element, parentElement);
		}

		sol::object customEvents = baseElement->elementTable.template raw_get<sol::object>("CustomEvents"); //< raw get as we don't want to fetch from the base
		if (customEvents)
		{
			sol::table elementCustomEvents = customEvents.as<sol::table>();

			for (const auto& kv : elementCustomEvents)
			{
				sol::table propertyTable = kv.second;

				try
				{
					std::size_t eventIndex = element->customEvents.size();
					ScriptedEvent event = InitEventFromLua(eventIndex, propertyTable);

					auto it = element->customEventByName.find(event.name);
					if (it == element->customEventByName.end())
					{
						element->customEventByName.emplace(event.name, eventIndex);
						element->customEvents.emplace_back(std::move(event));
					}
					else
						throw std::runtime_error("Custom event " + event.name + " already exists");
				}
				catch (const std::exception& e)
				{
					std::string_view eventName = propertyTable.get_or<std::string_view>("Name", "<No name set>");
					bwLog(m_logger, LogLevel::Error, "Failed to load custom event {0} for {1} {2}: {3}", eventName, m_elementName, element->name, e.what());
				}
			}
		}
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

		RegisterCustomEvents(element, element.get());
		RegisterProperties(element, element.get());

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
	void ScriptStore<Element>::RegisterProperties(const std::shared_ptr<Element>& element, Element* baseElement)
	{
		if (!baseElement->base.empty())
		{
			auto it = m_elementsByName.find(element->base);
			assert(it != m_elementsByName.end());

			Element* parentElement = m_elements[it->second].get();
			RegisterProperties(element, parentElement);
		}

		sol::object properties = baseElement->elementTable.template raw_get<sol::object>("Properties"); //< raw get as we don't want to fetch from the base
		if (properties)
		{
			sol::table elementProperties = properties.as<sol::table>();

			for (const auto& kv : elementProperties)
			{
				sol::table propertyTable = kv.second;

				std::string propertyName = propertyTable["Name"];

				try
				{
					std::size_t propertyIndex = element->properties.size();
					ScriptedProperty property = InitPropertyFromLua(propertyIndex, propertyTable);

					auto it = element->properties.find(propertyName);
					if (it == element->properties.end())
						element->properties.emplace(std::move(propertyName), std::move(property));
					else
						throw std::runtime_error("Property " + propertyName + " already exists");
				}
				catch (const std::exception& e)
				{
					bwLog(m_logger, LogLevel::Error, "Failed to load property {0} for {1} {2}: {3}", propertyName, m_elementName, element->name, e.what());
				}
			}
		}
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
	void ScriptStore<Element>::SetElementName(std::string elementName)
	{
		m_elementName = std::move(elementName);
	}

	template<typename Element>
	void ScriptStore<Element>::SetElementTypeName(std::string typeName)
	{
		m_elementTypeName = std::move(typeName);
	}
}
