// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientElementLibrary.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Components/LayerEntityComponent.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/Scripting/Sound.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <sol3/sol.hpp>

namespace bw
{
	void ClientElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedElementLibrary::RegisterLibrary(elementMetatable);

		RegisterClientLibrary(elementMetatable);
	}

	void ClientElementLibrary::RegisterClientLibrary(sol::table& elementTable)
	{
		auto DealDamage = [this](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 /*damage*/, Nz::Rectf damageZone, float pushbackForce = 0.f)
		{
			// Client-side this function only applies push-back forces
			if (Nz::NumberEquals(pushbackForce, 0.f))
				return;

			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);
			Ndk::World* world = entity->GetWorld();
			assert(world);

			world->GetSystem<Ndk::PhysicsSystem2D>().RegionQuery(damageZone, 0, 0xFFFFFFFF, 0xFFFFFFFF, [&](const Ndk::EntityHandle& hitEntity)
			{
				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitEntityPhys.GetMassCenter(Nz::CoordSys_Global) - origin) * pushbackForce);
				}
			});
		};

		elementTable["DealDamage"] = sol::overload(DealDamage,
			[=](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone) { DealDamage(entityTable, origin, damage, damageZone); });

		elementTable["GetLayerIndex"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			return entity->GetComponent<LocalMatchComponent>().GetLayerIndex();
		};
		
		elementTable["GetProperty"] = [](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(table);

			auto& entityScript = entity->GetComponent<ScriptComponent>();

			auto propertyVal = entityScript.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const EntityProperty& property = propertyVal.value();

				LocalMatch* match;
				if (entity->HasComponent<LocalMatchComponent>())
					match = &entity->GetComponent<LocalMatchComponent>().GetLocalMatch();
				else
					match = nullptr;

				const auto& entityElement = entityScript.GetElement();

				auto propertyIt = entityElement->properties.find(propertyName);
				assert(propertyIt != entityElement->properties.end());

				return TranslateEntityPropertyToLua(match, lua, property, propertyIt->second.type);
			}
			else
				return sol::nil;
		};

		elementTable["PlaySound"] = [this](const sol::table& entityTable, const std::string& soundPath, bool isAttachedToEntity, bool isLooping, bool isSpatialized)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);
			auto& entityMatch = entity->GetComponent<LocalMatchComponent>();

			const Nz::SoundBufferRef& soundBuffer = m_assetStore.GetSoundBuffer(soundPath);
			if (!soundBuffer)
				throw std::runtime_error("failed to load " + soundPath);

			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();

			auto& layer = entityMatch.GetLayer();

			std::optional<LocalLayerSound> localLayerSound;
			if (isAttachedToEntity)
				localLayerSound.emplace(layer, entityNode);
			else
				localLayerSound.emplace(layer, Nz::Vector2f(entityNode.GetPosition()));

			auto& layerSound = layer.RegisterSound(std::move(localLayerSound.value()));

			std::size_t soundIndex = layerSound.PlaySound(soundBuffer, isLooping, isSpatialized);
			return Sound(layerSound.CreateHandle(), soundIndex);
		};
	}
}
