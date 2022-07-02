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
		SetElementName("Entity");

		ReloadLibraries(); // This function creates the metatable
	}

	void SharedEntityStore::BindCallbacks(const ScriptedEntity& /*entityClass*/, entt::handle entity) const
	{
		if (InputComponent* entityInputs = entity.try_get<InputComponent>())
		{
			entityInputs->OnInputUpdate.Connect([entity](InputComponent* input)
			{
				auto& entityScript = entity.get<ScriptComponent>();

				entityScript.ExecuteCallback<ElementEvent::InputUpdate>(input->GetInputs());
			});
		}
	}

	void SharedEntityStore::InitializeElement(sol::main_table& /*elementTable*/, ScriptedEntity& /*element*/)
	{
	}

	bool SharedEntityStore::InitializeEntity(const ScriptedEntity& entityClass, entt::handle entity) const
	{
		if (!ScriptStore::InitializeEntity(entityClass, entity))
			return false;

		BindCallbacks(entityClass, entity);
		return true;
	}
}
