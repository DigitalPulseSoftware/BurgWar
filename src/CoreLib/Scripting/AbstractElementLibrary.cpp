// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/AbstractElementLibrary.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <cassert>

namespace bw
{
	AbstractElementLibrary::~AbstractElementLibrary() = default;

	const Ndk::EntityHandle& AbstractElementLibrary::AssertScriptEntity(const sol::table& entityTable)
	{
		const Ndk::EntityHandle& entity = RetrieveScriptEntity(entityTable);
		if (!entity || !entity->HasComponent<ScriptComponent>())
			throw std::runtime_error("Invalid entity");

		return entity;
	}

	const Ndk::EntityHandle& AbstractElementLibrary::RetrieveScriptEntity(const sol::table& entityTable)
	{
		sol::object entityObject = entityTable["_Entity"];
		if (!entityObject)
			throw std::runtime_error("Invalid entity");

		return entityObject.as<Ndk::EntityHandle>();
	}

}