// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedElementLibrary.hpp>
#include <CoreLib/Components/EntityOwnerComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Utils.hpp>
#include <NDK/Components/LifetimeComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Thirdparty/sol3/sol.hpp>

namespace bw
{
	SharedElementLibrary::~SharedElementLibrary() = default;

	void SharedElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		RegisterCommonLibrary(elementMetatable);
	}

	void SharedElementLibrary::RegisterCommonLibrary(sol::table& elementMetatable)
	{
		elementMetatable["DeleteOnRemove"] = [](const sol::table& entityTable, const sol::table& targetEntityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			const Ndk::EntityHandle& targetEntity = AssertScriptEntity(targetEntityTable);

			if (!entity->HasComponent<EntityOwnerComponent>())
				entity->AddComponent<EntityOwnerComponent>();

			entity->GetComponent<EntityOwnerComponent>().Register(targetEntity);
		};

		elementMetatable["GetDirection"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			Nz::Vector2f direction(nodeComponent.GetRotation(Nz::CoordSys_Global) * Nz::Vector2f::UnitX());
			if (nodeComponent.GetScale().x < 0.f)
				direction = -direction;

			return direction;
		};

		elementMetatable["GetPosition"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.GetPosition(Nz::CoordSys_Global));
		};

		elementMetatable["GetRotation"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::DegreeAnglef(AngleFromQuaternion(nodeComponent.GetRotation(Nz::CoordSys_Global))); //<FIXME: not very efficient
		};

		elementMetatable["IsLookingRight"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return nodeComponent.GetScale().x > 0.f;
		};

		elementMetatable["IsValid"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::RetrieveScriptEntity(entityTable);
			return entity.IsValid();
		};

		elementMetatable["SetLifeTime"] = [](const sol::table& entityTable, float lifetime)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::RetrieveScriptEntity(entityTable);
			entity->AddComponent<Ndk::LifetimeComponent>(lifetime);
		};

		elementMetatable["ToLocalPosition"] = [](const sol::table& entityTable, const Nz::Vector2f& globalPosition)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.ToLocalPosition(globalPosition));
		};

		elementMetatable["ToGlobalPosition"] = [](const sol::table& entityTable, const Nz::Vector2f& localPosition)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.ToGlobalPosition(localPosition));
		};
	}
}
