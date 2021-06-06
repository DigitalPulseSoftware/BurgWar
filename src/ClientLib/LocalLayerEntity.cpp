// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayerEntity.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CollisionDataComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components.hpp>

namespace bw
{
	LocalLayerEntity::LocalLayerEntity(LocalLayer& layer, const Ndk::EntityHandle& entity, Nz::UInt32 serverEntityId, EntityId uniqueId) :
	LayerVisualEntity(entity, layer.GetLayerIndex(), uniqueId),
	m_serverEntityId(serverEntityId),
	m_layer(layer)
	{
		m_layer.GetLocalMatch().RegisterEntity(uniqueId, CreateHandle<LocalLayerEntity>());
	}

	LocalLayerEntity::~LocalLayerEntity()
	{
		if (m_ghostEntity)
			m_layer.OnEntityDelete(&m_layer, *m_ghostEntity);

		if (EntityId uniqueId = GetUniqueId(); uniqueId != InvalidEntityId)
		{
			// Don't trigger the Destroyed event on disabling layers
			if (m_layer.IsEnabled() && GetEntity()->HasComponent<ScriptComponent>())
			{
				auto& scriptComponent = GetEntity()->GetComponent<ScriptComponent>();
				scriptComponent.ExecuteCallback<ElementEvent::Destroyed>();
			}

			m_layer.GetLocalMatch().UnregisterEntity(uniqueId);
		}
	}

	Nz::RadianAnglef LocalLayerEntity::GetAngularVelocity() const
	{
		assert(IsPhysical());

		auto& entityPhys = GetEntity()->GetComponent<Ndk::PhysicsComponent2D>();
		return entityPhys.GetAngularVelocity();
	}

	LocalLayerEntity* LocalLayerEntity::GetGhost()
	{
		if (!m_ghostEntity)
		{
			const Ndk::EntityHandle& ghostEntity = GetEntity()->GetWorld()->CreateEntity();
			ghostEntity->AddComponent<Ndk::NodeComponent>();

			m_ghostEntity = std::make_unique<LocalLayerEntity>(m_layer, ghostEntity, ClientsideId, m_layer.GetLocalMatch().AllocateClientUniqueId());

			/*for (auto& renderable : m_attachedRenderables)
			{
				if (std::unique_ptr<Nz::InstancedRenderable> clonedRenderable = renderable.renderable->Clone())
				{
					std::size_t materialCount = clonedRenderable->GetMaterialCount();
					for (std::size_t i = 0; i < materialCount; ++i)
					{
						Nz::MaterialRef ghostMaterial = Nz::Material::New(*clonedRenderable->GetMaterial(i));
						ghostMaterial->Configure("Translucent2D");
						ghostMaterial->SetDiffuseColor(Nz::Color(255, 255, 255, 160));
						ghostMaterial->SetDiffuseMap(ghostMaterial->GetDiffuseMap());

						clonedRenderable->SetMaterial(i, ghostMaterial);
					}

					m_ghostEntity->AttachRenderable(clonedRenderable.release(), renderable.offsetMatrix, -1);
				}
			}*/

			m_layer.OnEntityCreated(&m_layer, *m_ghostEntity);
		}

		return m_ghostEntity.get();
	}

	LayerIndex LocalLayerEntity::GetLayerIndex() const
	{
		return m_layer.GetLayerIndex();
	}

	Nz::Vector2f LocalLayerEntity::GetLinearVelocity() const
	{
		assert(IsPhysical());

		auto& entityPhys = GetEntity()->GetComponent<Ndk::PhysicsComponent2D>();
		return entityPhys.GetVelocity();
	}

	Nz::Vector2f LocalLayerEntity::GetPhysicalPosition() const
	{
		assert(IsPhysical());
		
		auto& entityPhys = GetEntity()->GetComponent<Ndk::PhysicsComponent2D>();
		return entityPhys.GetPosition();
	}

	Nz::RadianAnglef LocalLayerEntity::GetPhysicalRotation() const
	{
		assert(IsPhysical());

		auto& entityPhys = GetEntity()->GetComponent<Ndk::PhysicsComponent2D>();
		return entityPhys.GetRotation();
	}

	Nz::Vector2f LocalLayerEntity::GetPosition() const
	{
		auto& entityNode = GetEntity()->GetComponent<Ndk::NodeComponent>();
		return Nz::Vector2f(entityNode.GetPosition()); //< FIXME
	}

	Nz::RadianAnglef LocalLayerEntity::GetRotation() const
	{
		auto& entityNode = GetEntity()->GetComponent<Ndk::NodeComponent>();
		return AngleFromQuaternion(entityNode.GetRotation()); //< FIXME
	}

	void LocalLayerEntity::InitializeHealth(Nz::UInt16 maxHealth, Nz::UInt16 currentHealth)
	{
		auto& healthData = m_health.emplace();
		healthData.currentHealth = currentHealth;
		healthData.maxHealth = maxHealth;
		healthData.spriteWidth = GetLocalBounds().width * 0.85f;

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

		if (currentHealth != maxHealth)
			ShowHealthBar();
	}

	bool LocalLayerEntity::IsFacingRight() const
	{
		auto& entityNode = GetEntity()->GetComponent<Ndk::NodeComponent>();
		return entityNode.GetScale().x > 0.f;
	}

	void LocalLayerEntity::UpdateAnimation(Nz::UInt8 animationId)
	{
		auto& animComponent = GetEntity()->GetComponent<AnimationComponent>();
		animComponent.Play(animationId, m_layer.GetMatch().GetCurrentTime());
	}

	void LocalLayerEntity::UpdatePlayerMovement(bool isFacingRight)
	{
		auto& playerMovementComponent = GetEntity()->GetComponent<PlayerMovementComponent>();

		if (playerMovementComponent.UpdateFacingRightState(isFacingRight))
		{
			auto& entityNode = GetEntity()->GetComponent<Ndk::NodeComponent>();
			entityNode.Scale(-1.f, 1.f);
		}
	}

	void LocalLayerEntity::UpdateHealth(Nz::UInt16 newHealth)
	{
		assert(m_health);

		Nz::UInt16 oldHealth = m_health->currentHealth;
		if (newHealth == oldHealth)
			return;

		m_health->currentHealth = newHealth;
		m_health->healthSprite->SetSize(m_health->spriteWidth * m_health->currentHealth / m_health->maxHealth, 10);

		if (m_health->currentHealth == m_health->maxHealth)
			HideHealthBar();
		else
			ShowHealthBar();

		if (GetEntity()->HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = GetEntity()->GetComponent<ScriptComponent>();
			scriptComponent.ExecuteCallback<ElementEvent::HealthUpdate>(oldHealth, newHealth);

			if (newHealth == 0)
			{
				scriptComponent.ExecuteCallback<ElementEvent::Death>();
				scriptComponent.ExecuteCallback<ElementEvent::Died>();
			}
		}
	}

	void LocalLayerEntity::UpdateInputs(const PlayerInputData& inputData)
	{
		GetEntity()->GetComponent<InputComponent>().UpdateInputs(inputData);
	}

	void LocalLayerEntity::UpdateParent(const LocalLayerEntity* newParent)
	{
		auto& entityNode = GetEntity()->GetComponent<Ndk::NodeComponent>();
		if (newParent)
			entityNode.SetParent(newParent->GetEntity(), true);
		else
			entityNode.SetParent(static_cast<Nz::Node*>(nullptr));
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

	void LocalLayerEntity::HideHealthBar()
	{
		DetachHoveringRenderable(m_health->healthSprite);
		DetachHoveringRenderable(m_health->lostHealthSprite);
	}

	void LocalLayerEntity::ShowHealthBar()
	{
		AttachHoveringRenderable(m_health->healthSprite, Nz::Matrix4f::Identity(), 2, 10.f);
		AttachHoveringRenderable(m_health->lostHealthSprite, Nz::Matrix4f::Identity(), 1, 10.f);
	}
}
