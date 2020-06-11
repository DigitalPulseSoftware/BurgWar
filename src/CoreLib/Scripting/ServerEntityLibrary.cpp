// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerEntityLibrary.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>

namespace bw
{
	void ServerEntityLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedEntityLibrary::RegisterLibrary(elementMetatable);

		RegisterServerLibrary(elementMetatable);
	}

	void ServerEntityLibrary::RegisterServerLibrary(sol::table& /*elementMetatable*/)
	{
	}
	
	void ServerEntityLibrary::SetMass(const Ndk::EntityHandle& entity, float mass, bool recomputeMomentOfInertia)
	{
		SharedEntityLibrary::SetMass(entity, mass, recomputeMomentOfInertia);

		Ndk::World* world = entity->GetWorld();
		world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}
	
	void ServerEntityLibrary::SetMomentOfInertia(const Ndk::EntityHandle& entity, float momentOfInertia)
	{
		SharedEntityLibrary::SetMomentOfInertia(entity, momentOfInertia);

		Ndk::World* world = entity->GetWorld();
		world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}
}
