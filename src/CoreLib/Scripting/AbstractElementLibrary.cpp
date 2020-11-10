// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/AbstractElementLibrary.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <cassert>

namespace bw
{
	AbstractElementLibrary::~AbstractElementLibrary() = default;

	std::shared_ptr<ScriptedElement> AbstractElementLibrary::AssertScriptElement(const sol::table& entityTable)
	{
		std::shared_ptr<ScriptedElement> element = RetrieveScriptElement(entityTable);
		if (!element)
			throw std::runtime_error("Invalid element");

		return element;
	}

	Ndk::EntityHandle AbstractElementLibrary::AssertScriptEntity(const sol::table& entityTable)
	{
		Ndk::EntityHandle entity = RetrieveScriptEntity(entityTable);
		if (!entity || !entity->HasComponent<ScriptComponent>())
			throw std::runtime_error("Invalid entity");

		return entity;
	}

	std::shared_ptr<ScriptedElement> AbstractElementLibrary::RetrieveScriptElement(const sol::table& entityTable)
	{
		sol::object entityObject = entityTable["_Element"];
		if (!entityObject)
			return nullptr;

		return entityObject.as<std::shared_ptr<ScriptedElement>>();
	}

	Ndk::EntityHandle AbstractElementLibrary::RetrieveScriptEntity(const sol::table& entityTable)
	{
		sol::object entityObject = entityTable["_Entity"];
		if (!entityObject)
			return Ndk::EntityHandle::InvalidHandle;

		return entityObject.as<Ndk::EntityHandle>();
	}

	sol::object AbstractElementLibrary::TranslateEntity(const Ndk::EntityHandle& entity)
	{
		if (!entity || !entity->HasComponent<ScriptComponent>())
			return sol::nil;

		auto& entityScript = entity->GetComponent<ScriptComponent>();
		return entityScript.GetTable();
	}
}
