// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientLayerEntity.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CollisionDataComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/ClientLayer.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>

namespace bw
{
	ClientLayerEntity::ClientLayerEntity(ClientLayer& layer, entt::handle entity, Nz::UInt32 serverEntityId, EntityId uniqueId) :
	LayerVisualEntity(entity, layer.GetLayerIndex(), uniqueId),
	m_serverEntityId(serverEntityId),
	m_layer(layer)
	{
		m_layer.GetClientMatch().RegisterEntity(uniqueId, CreateHandle<ClientLayerEntity>());
	}

	ClientLayerEntity::~ClientLayerEntity()
	{
		if (m_ghostEntity)
			m_layer.OnEntityDelete(&m_layer, *m_ghostEntity);

		if (EntityId uniqueId = GetUniqueId(); uniqueId != InvalidEntityId)
			m_layer.GetClientMatch().UnregisterEntity(uniqueId);
	}

	Nz::RadianAnglef ClientLayerEntity::GetAngularVelocity() const
	{
		assert(IsPhysical());

		auto& entityPhys = GetEntity().get<Nz::RigidBody2DComponent>();
		return entityPhys.GetAngularVelocity();
	}

	ClientLayerEntity* ClientLayerEntity::GetGhost()
	{
		if (!m_ghostEntity)
		{
			/*entt::entity ghostEntity = GetEntity()->GetWorld()->CreateEntity();
			ghostEntity->AddComponent<Nz::NodeComponent>();

			m_ghostEntity = std::make_unique<ClientLayerEntity>(m_layer, ghostEntity, ClientsideId, m_layer.GetClientMatch().AllocateClientUniqueId());

			for (auto& renderable : m_attachedRenderables)
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

	LayerIndex ClientLayerEntity::GetLayerIndex() const
	{
		return m_layer.GetLayerIndex();
	}

	Nz::Vector2f ClientLayerEntity::GetLinearVelocity() const
	{
		assert(IsPhysical());

		auto& entityPhys = GetEntity().get<Nz::RigidBody2DComponent>();
		return entityPhys.GetVelocity();
	}

	Nz::Vector2f ClientLayerEntity::GetPhysicalPosition() const
	{
		assert(IsPhysical());
		
		auto& entityPhys = GetEntity().get<Nz::RigidBody2DComponent>();
		return entityPhys.GetPosition();
	}

	Nz::RadianAnglef ClientLayerEntity::GetPhysicalRotation() const
	{
		assert(IsPhysical());

		auto& entityPhys = GetEntity().get<Nz::RigidBody2DComponent>();
		return entityPhys.GetRotation();
	}

	Nz::Vector2f ClientLayerEntity::GetPosition() const
	{
		auto& entityNode = GetEntity().get<Nz::NodeComponent>();
		return Nz::Vector2f(entityNode.GetPosition()); //< FIXME
	}

	Nz::RadianAnglef ClientLayerEntity::GetRotation() const
	{
		auto& entityNode = GetEntity().get<Nz::NodeComponent>();
		return AngleFromQuaternion(entityNode.GetRotation()); //< FIXME
	}

	void ClientLayerEntity::InitializeHealth(Nz::UInt16 maxHealth, Nz::UInt16 currentHealth)
	{
		auto& healthData = m_health.emplace();
		healthData.currentHealth = currentHealth;
		healthData.maxHealth = maxHealth;
		healthData.spriteWidth = GetLocalBounds().width * 0.85f;

		/*std::shared_ptr<Nz::Sprite> lostHealthBar = Nz::Sprite::New();
		lostHealthBar->SetMaterial(Nz::MaterialLibrary::Get("SpriteNoDepth"));
		lostHealthBar->SetSize(healthData.spriteWidth, 10);
		lostHealthBar->SetColor(Nz::Color::Red);
		lostHealthBar->SetOrigin(Nz::Vector2f(healthData.spriteWidth / 2.f, lostHealthBar->GetSize().y));

		std::shared_ptr<Nz::Sprite> healthBar = Nz::Sprite::New();
		healthBar->SetMaterial(Nz::MaterialLibrary::Get("SpriteNoDepth"));
		healthBar->SetSize(healthData.spriteWidth * healthData.currentHealth / healthData.maxHealth, 10);
		healthBar->SetColor(Nz::Color::Green);
		healthBar->SetOrigin(Nz::Vector2f(healthData.spriteWidth / 2.f, healthBar->GetSize().y));

		healthData.lostHealthSprite = lostHealthBar;
		healthData.healthSprite = healthBar;
		*/
		if (currentHealth != maxHealth)
			ShowHealthBar();
	}

	bool ClientLayerEntity::IsFacingRight() const
	{
		auto& entityNode = GetEntity().get<Nz::NodeComponent>();
		return entityNode.GetScale().x > 0.f;
	}

	void ClientLayerEntity::UpdateAnimation(Nz::UInt8 animationId)
	{
		auto& animComponent = GetEntity().get<AnimationComponent>();
		animComponent.Play(animationId, m_layer.GetMatch().GetCurrentTime());
	}

	void ClientLayerEntity::UpdatePlayerMovement(bool isFacingRight)
	{
		auto& playerMovementComponent = GetEntity().get<PlayerMovementComponent>();

		if (playerMovementComponent.UpdateFacingRightState(isFacingRight))
		{
			auto& entityNode = GetEntity().get<Nz::NodeComponent>();
			entityNode.Scale(-1.f, 1.f);
		}
	}

	void ClientLayerEntity::UpdateHealth(Nz::UInt16 newHealth)
	{
		assert(m_health);

		Nz::UInt16 oldHealth = m_health->currentHealth;
		if (newHealth == oldHealth)
			return;

		m_health->currentHealth = newHealth;
		m_health->healthSprite->SetSize(Nz::Vector2f(m_health->spriteWidth * m_health->currentHealth / m_health->maxHealth, 10));

		if (m_health->currentHealth == m_health->maxHealth)
			HideHealthBar();
		else
			ShowHealthBar();

		if (ScriptComponent* scriptComponent = GetEntity().try_get<ScriptComponent>())
		{
			scriptComponent->ExecuteCallback<ElementEvent::HealthUpdate>(oldHealth, newHealth);

			if (newHealth == 0)
			{
				scriptComponent->ExecuteCallback<ElementEvent::Death>();
				scriptComponent->ExecuteCallback<ElementEvent::Died>();
			}
		}
	}

	void ClientLayerEntity::UpdateInputs(const PlayerInputData& inputData)
	{
		GetEntity().get<InputComponent>().UpdateInputs(inputData);
	}

	void ClientLayerEntity::UpdateParent(const ClientLayerEntity* newParent)
	{
		auto& entityNode = GetEntity().get<Nz::NodeComponent>();
		if (newParent)
			entityNode.SetParent(newParent->GetEntity(), true);
		else
			entityNode.SetParent(static_cast<Nz::Node*>(nullptr));
	}

	void ClientLayerEntity::UpdateWeaponEntity(const ClientLayerEntityHandle& entity)
	{
		if (m_weaponEntity)
		{
			auto& entityWeapon = m_weaponEntity->GetEntity().get<WeaponComponent>();
			entityWeapon.SetActive(false);

			m_weaponEntity->Disable();
		}

		m_weaponEntity = entity;
		if (m_weaponEntity)
		{
			auto& entityWeapon = m_weaponEntity->GetEntity().get<WeaponComponent>();
			entityWeapon.SetActive(true);

			m_weaponEntity->Enable();
		}
	}

	void ClientLayerEntity::HideHealthBar()
	{
		DetachHoveringRenderable(m_health->healthSprite);
		DetachHoveringRenderable(m_health->lostHealthSprite);
	}

	void ClientLayerEntity::ShowHealthBar()
	{
		AttachHoveringRenderable(m_health->healthSprite, Nz::Matrix4f::Identity(), 2, 10.f);
		AttachHoveringRenderable(m_health->lostHealthSprite, Nz::Matrix4f::Identity(), 1, 10.f);
	}
}
