// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedEntityStore.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <stdexcept>

namespace bw
{
	SharedEntityStore::SharedEntityStore(const Logger& logger, std::shared_ptr<ScriptingContext> context, bool isServer) :
	ScriptStore(logger, std::move(context), isServer)
	{
		SetElementTypeName("entity");
		SetTableName("ENTITY");
	}

	void SharedEntityStore::BindCallbacks(const ScriptedEntity& /*entityClass*/, const Ndk::EntityHandle& entity) const
	{
		if (entity->HasComponent<InputComponent>())
		{
			InputComponent& entityInputs = entity->GetComponent<InputComponent>();
			entityInputs.OnInputUpdate.Connect([](InputComponent* input)
			{
				const Ndk::EntityHandle& entity = input->GetEntity();
				auto& entityScript = entity->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback("OnInputUpdate", input->GetInputs());
			});
		}

		entity->OnEntityDestruction.Connect([&](Ndk::Entity* entity)
		{
			auto& entityScript = entity->GetComponent<ScriptComponent>();

			entityScript.ExecuteCallback("OnKilled");
		});
	}

	void SharedEntityStore::InitializeElement(sol::table& /*elementTable*/, ScriptedEntity& /*element*/)
	{
	}

	bool SharedEntityStore::InitializeEntity(const ScriptedEntity& entityClass, const Ndk::EntityHandle& entity) const
	{
		if (!ScriptStore::InitializeEntity(entityClass, entity))
			return false;

		BindCallbacks(entityClass, entity);

		return true;
	}
}
