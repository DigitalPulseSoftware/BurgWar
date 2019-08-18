// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedElementLibrary.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <NDK/Entity.hpp>
#include <sol3/sol.hpp>
#include <iostream>

namespace bw
{
	SharedElementLibrary::~SharedElementLibrary() = default;

	const Ndk::EntityHandle& SharedElementLibrary::AssertScriptEntity(const sol::table& entityTable)
	{
		sol::object entityObject = entityTable["_Entity"];
		if (!entityObject)
			throw std::runtime_error("Invalid entity");

		const Ndk::EntityHandle& entity = entityObject.as<Ndk::EntityHandle>();

		if (!entity || !entity->HasComponent<ScriptComponent>())
			throw std::runtime_error("Invalid entity");

		return entity;
	}

	void SharedElementLibrary::RegisterCommonLibrary(sol::table& elementTable)
	{
		elementTable["GetOwner"] = [](sol::this_state s, const sol::table& table) -> sol::object
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(table);

			if (!entity->HasComponent<OwnerComponent>())
				return sol::nil;

			return sol::make_object(s, entity->GetComponent<OwnerComponent>().GetOwner()->CreateHandle());
		};

		elementTable["GetProperty"] = [](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(table);

			auto& properties = entity->GetComponent<ScriptComponent>();

			auto propertyVal = properties.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const EntityProperty& property = propertyVal.value();

				return TranslateEntityPropertyToLua(lua, property);
			}
			else
				return sol::nil;
		};
	}
}
