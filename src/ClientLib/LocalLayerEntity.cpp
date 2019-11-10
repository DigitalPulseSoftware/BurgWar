// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayerEntity.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components.hpp>

namespace bw
{
	LocalLayerEntity::LocalLayerEntity(LocalLayer& layer, const Ndk::EntityHandle& entity, Nz::UInt32 serverEntityId, bool isPhysical) :
	m_entity(entity),
	m_serverEntityId(serverEntityId),
	m_layer(layer),
	m_isPhysical(isPhysical)
	{
	}

	void LocalLayerEntity::AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder)
	{
		auto& renderableData = m_attachedRenderables.emplace_back();
		renderableData.offset = offsetMatrix;
		renderableData.renderable = std::move(renderable);
		renderableData.renderOrder = renderOrder;

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->AttachRenderable(renderableData.renderable, renderableData.offset, renderableData.renderOrder);
	}

	void LocalLayerEntity::DetachRenderable(const Nz::InstancedRenderableRef& renderable)
	{
		auto it = std::find_if(m_attachedRenderables.begin(), m_attachedRenderables.end(), [&](const RenderableData& renderableData) { return renderableData.renderable == renderable; });
		m_attachedRenderables.erase(it);

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->DetachRenderable(renderable);
	}

	void LocalLayerEntity::Enable(bool enable)
	{
		if (m_entity->IsEnabled() == enable)
			return;

		m_entity->Enable(enable);
		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->Enable(enable);
	}

	Nz::Vector2f LocalLayerEntity::GetPosition() const
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		return Nz::Vector2f(entityNode.GetPosition());
	}

	Nz::RadianAnglef LocalLayerEntity::GetRotation() const
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		return entityNode.GetRotation().ToEulerAngles().roll; //< FIXME
	}

	void LocalLayerEntity::InitializeHealth(Nz::UInt16 maxHealth, Nz::UInt16 currentHealth)
	{
		auto& healthData = m_health.emplace();
		healthData.currentHealth = currentHealth;
		healthData.maxHealth = maxHealth;

		/*auto& gfxComponent = m_entity->GetComponent<Ndk::GraphicsComponent>();

		const Nz::Boxf& aabb = gfxComponent.GetAABB();

		healthData.spriteWidth = std::max(aabb.width, aabb.height) * 0.85f;*/
		healthData.spriteWidth = 200;

		Nz::SpriteRef lostHealthBar = Nz::Sprite::New();
		lostHealthBar->SetMaterial(Nz::MaterialLibrary::Get("SpriteNoDepth"));
		lostHealthBar->SetSize(healthData.spriteWidth, 10);
		lostHealthBar->SetColor(Nz::Color::Red);
		lostHealthBar->SetOrigin(Nz::Vector2f(healthData.spriteWidth / 2.f, lostHealthBar->GetSize().y));

		Nz::SpriteRef healthBar = Nz::Sprite::New();
		healthBar->SetMaterial(Nz::MaterialLibrary::Get("SpriteNoDepth"));
		healthBar->SetSize(healthData.spriteWidth * healthData.currentHealth / healthData.maxHealth, 10);
		healthBar->SetColor(Nz::Color::Green);
		healthBar->SetOrigin(Nz::Vector2f(healthData.spriteWidth / 2.f, healthBar->GetSize().y));

		healthData.lostHealthSprite = lostHealthBar;
		healthData.healthSprite = healthBar;
	}

	void LocalLayerEntity::InitializeName(const std::string& name)
	{
		auto& nameData = m_name.emplace();

		Nz::TextSpriteRef nameSprite = Nz::TextSprite::New();
		nameSprite->Update(Nz::SimpleTextDrawer::Draw(name, 24, Nz::TextStyle_Regular, Nz::Color::White, 2.f, Nz::Color::Black));
	}

	void LocalLayerEntity::SyncVisuals()
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();

		for (VisualEntity* visualEntity : m_visualEntities)
		{
			auto& visualNode = visualEntity->GetEntity()->GetComponent<Ndk::NodeComponent>();
			visualNode.SetPosition(entityNode.GetPosition());
			visualNode.SetRotation(entityNode.GetRotation());
			visualNode.SetScale(entityNode.GetScale());
		}
	}

	void LocalLayerEntity::UpdateAnimation(Nz::UInt8 animationId)
	{
		auto& animComponent = m_entity->GetComponent<AnimationComponent>();
		animComponent.Play(animationId, m_layer.GetMatch().GetCurrentTime());
	}

	void LocalLayerEntity::UpdatePlayerMovement(bool isFacingRight)
	{
		auto& playerMovementComponent = m_entity->GetComponent<PlayerMovementComponent>();

		if (playerMovementComponent.UpdateFacingRightState(isFacingRight))
		{
			auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
			entityNode.Scale(-1.f, 1.f);
		}
	}

	void LocalLayerEntity::UpdateHealth(Nz::UInt16 newHealth)
	{
		assert(m_health);

		Nz::UInt16 oldHealth = m_health->currentHealth;

		m_health->currentHealth = newHealth;
		m_health->healthSprite->SetSize(m_health->spriteWidth * m_health->currentHealth / m_health->maxHealth, 10);

		if (m_entity->HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = m_entity->GetComponent<ScriptComponent>();
			scriptComponent.ExecuteCallback("OnHealthUpdate", oldHealth, newHealth);
		}
	}
	
	void LocalLayerEntity::UpdateInputs(const PlayerInputData& inputData)
	{
		//if (!serverEntity.isLocalPlayerControlled)
		m_entity->GetComponent<InputComponent>().UpdateInputs(inputData);
	}

	void LocalLayerEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation)
	{
		assert(!m_isPhysical);

		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		entityNode.SetPosition(position);
		entityNode.SetRotation(rotation);
	}

	void LocalLayerEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation, const Nz::Vector2f& linearVel, const Nz::RadianAnglef& angularVel)
	{
		assert(m_isPhysical);

		auto& entityPhys = m_entity->GetComponent<Ndk::PhysicsComponent2D>();
		entityPhys.SetAngularVelocity(angularVel);
		entityPhys.SetPosition(position);
		entityPhys.SetRotation(rotation);
		entityPhys.SetVelocity(linearVel);
	}

	void LocalLayerEntity::UpdateWeaponEntity(const LocalLayerEntityHandle& entity)
	{
		if (m_weaponEntity)
		{
			auto& entityWeapon = m_weaponEntity->GetEntity()->GetComponent<WeaponComponent>();
			entityWeapon.SetActive(false);

			m_weaponEntity->Disable();
		}

		m_weaponEntity = entity;
		if (m_weaponEntity)
		{
			auto& entityWeapon = m_weaponEntity->GetEntity()->GetComponent<WeaponComponent>();
			entityWeapon.SetActive(true);

			m_weaponEntity->Enable();
		}
	}

	void LocalLayerEntity::NotifyVisualEntityMoved(VisualEntity* oldPointer, VisualEntity* newPointer)
	{
		auto it = std::find(m_visualEntities.begin(), m_visualEntities.end(), oldPointer);
		assert(it != m_visualEntities.end());

		*it = newPointer;
	}
	
	void LocalLayerEntity::RegisterVisualEntity(VisualEntity* visualEntity)
	{
		assert(std::find(m_visualEntities.begin(), m_visualEntities.end(), visualEntity) == m_visualEntities.end());
		m_visualEntities.push_back(visualEntity);

		visualEntity->Enable(m_entity->IsEnabled());

		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();

		auto& visualNode = visualEntity->GetEntity()->GetComponent<Ndk::NodeComponent>();
		visualNode.SetPosition(entityNode.GetPosition());
		visualNode.SetRotation(entityNode.GetRotation());
		visualNode.SetScale(entityNode.GetScale());

		for (auto& renderableData : m_attachedRenderables)
			visualEntity->AttachRenderable(renderableData.renderable, renderableData.offset, renderableData.renderOrder);
	}
	
	void LocalLayerEntity::UnregisterVisualEntity(VisualEntity* visualEntity)
	{
		auto it = std::find(m_visualEntities.begin(), m_visualEntities.end(), visualEntity);
		assert(it != m_visualEntities.end());
		m_visualEntities.erase(it);
	}
}
