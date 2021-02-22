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
	m_entity(entity),
	m_uniqueId(uniqueId),
	m_serverEntityId(serverEntityId),
	m_layer(layer)
	{
		assert(m_entity);
		m_layer.GetLocalMatch().RegisterEntity(m_uniqueId, CreateHandle());
	}

	LocalLayerEntity::LocalLayerEntity(LocalLayerEntity&& entity) noexcept :
	HandledObject(std::move(entity)),
	m_ghostEntity(std::move(entity.m_ghostEntity)),
	m_entityId(std::move(entity.m_entityId)),
	m_health(std::move(entity.m_health)),
	m_attachedHoveringRenderables(std::move(entity.m_attachedHoveringRenderables)),
	m_attachedRenderables(std::move(entity.m_attachedRenderables)),
	m_visualEntities(std::move(entity.m_visualEntities)),
	m_entity(std::move(entity.m_entity)),
	m_uniqueId(entity.m_uniqueId),
	m_serverEntityId(entity.m_serverEntityId),
	m_weaponEntity(std::move(entity.m_weaponEntity)),
	m_layer(entity.m_layer)
	{
		entity.m_uniqueId = InvalidEntityId;
	}

	LocalLayerEntity::~LocalLayerEntity()
	{
		if (m_ghostEntity)
			m_layer.OnEntityDelete(&m_layer, *m_ghostEntity);

		if (m_uniqueId != InvalidEntityId)
			m_layer.GetLocalMatch().UnregisterEntity(m_uniqueId);
	}

	void LocalLayerEntity::AttachHoveringRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder, float hoveringHeight)
	{
		auto& renderableData = m_attachedHoveringRenderables.emplace_back();
		renderableData.hoveringHeight = hoveringHeight;
		renderableData.data.offsetMatrix = offsetMatrix;
		renderableData.data.renderable = std::move(renderable);
		renderableData.data.renderOrder = renderOrder;

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->AttachHoveringRenderable(renderableData.data.renderable, renderableData.data.offsetMatrix, renderableData.data.renderOrder, hoveringHeight);
	}

	void LocalLayerEntity::AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder)
	{
		auto& renderableData = m_attachedRenderables.emplace_back();
		renderableData.offsetMatrix = offsetMatrix;
		renderableData.renderable = std::move(renderable);
		renderableData.renderOrder = renderOrder;

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->AttachRenderable(renderableData.renderable, renderableData.offsetMatrix, renderableData.renderOrder);
	}

	void LocalLayerEntity::DetachHoveringRenderable(const Nz::InstancedRenderableRef& renderable)
	{
		for (auto it = m_attachedHoveringRenderables.begin(); it != m_attachedHoveringRenderables.end(); ++it)
		{
			auto& hoveringRenderable = *it;
			if (hoveringRenderable.data.renderable == renderable)
			{
				for (VisualEntity* visualEntity : m_visualEntities)
					visualEntity->DetachHoveringRenderable(renderable);

				m_attachedHoveringRenderables.erase(it);
				break;
			}
		}
	}

	void LocalLayerEntity::DetachRenderable(const Nz::InstancedRenderableRef& renderable)
	{
		auto it = std::find_if(m_attachedRenderables.begin(), m_attachedRenderables.end(), [&](const RenderableData& renderableData) { return renderableData.renderable == renderable; });
		if (it != m_attachedRenderables.end())
		{
			m_attachedRenderables.erase(it);

			for (VisualEntity* visualEntity : m_visualEntities)
				visualEntity->DetachRenderable(renderable);
		}
	}

	void LocalLayerEntity::Enable(bool enable)
	{
		if (IsEnabled() == enable)
			return;

		m_entity->Enable(enable);
		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->Enable(enable);
	}

	Nz::RadianAnglef LocalLayerEntity::GetAngularVelocity() const
	{
		assert(IsPhysical());

		auto& entityPhys = m_entity->GetComponent<Ndk::PhysicsComponent2D>();
		return entityPhys.GetAngularVelocity();
	}

	LocalLayerEntity* LocalLayerEntity::GetGhost()
	{
		if (!m_ghostEntity)
		{
			const Ndk::EntityHandle& ghostEntity = m_entity->GetWorld()->CreateEntity();
			ghostEntity->AddComponent<Ndk::NodeComponent>();

			m_ghostEntity = std::make_unique<LocalLayerEntity>(m_layer, ghostEntity, ClientsideId, m_layer.GetLocalMatch().AllocateClientUniqueId());

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
			}

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

		auto& entityPhys = m_entity->GetComponent<Ndk::PhysicsComponent2D>();
		return entityPhys.GetVelocity();
	}

	Nz::Vector2f LocalLayerEntity::GetPhysicalPosition() const
	{
		assert(IsPhysical());
		
		auto& entityPhys = m_entity->GetComponent<Ndk::PhysicsComponent2D>();
		return entityPhys.GetPosition();
	}

	Nz::RadianAnglef LocalLayerEntity::GetPhysicalRotation() const
	{
		assert(IsPhysical());

		auto& entityPhys = m_entity->GetComponent<Ndk::PhysicsComponent2D>();
		return entityPhys.GetRotation();
	}

	Nz::Vector2f LocalLayerEntity::GetPosition() const
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		return Nz::Vector2f(entityNode.GetPosition()); //< FIXME
	}

	Nz::RadianAnglef LocalLayerEntity::GetRotation() const
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		return AngleFromQuaternion(entityNode.GetRotation()); //< FIXME
	}

	void LocalLayerEntity::InitializeHealth(Nz::UInt16 maxHealth, Nz::UInt16 currentHealth)
	{
		auto& healthData = m_health.emplace();
		healthData.currentHealth = currentHealth;
		healthData.maxHealth = maxHealth;

		float spriteWidth = 0.f;
		for (const auto& renderableData : m_attachedRenderables)
		{
			const auto& localBox = renderableData.renderable->GetBoundingVolume().obb.localBox;
			spriteWidth = std::max({ spriteWidth, localBox.width, localBox.height });
		}

		healthData.spriteWidth = spriteWidth * 0.85f;

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
		{
			for (VisualEntity* visualEntity : m_visualEntities)
				ShowHealthBar(visualEntity);
		}
	}

	bool LocalLayerEntity::IsFacingRight() const
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		return entityNode.GetScale().x > 0.f;
	}

	bool LocalLayerEntity::IsPhysical() const
	{
		return m_entity->HasComponent<Ndk::PhysicsComponent2D>(); //< TODO: Cache this?
	}

	void LocalLayerEntity::SyncVisuals()
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();

		Nz::Vector2f position = Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys_Global));
		Nz::Vector2f scale = Nz::Vector2f(entityNode.GetScale(Nz::CoordSys_Global));
		Nz::Quaternionf rotation = entityNode.GetRotation(Nz::CoordSys_Global);

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->Update(position, rotation, scale);
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
		if (newHealth == oldHealth)
			return;

		m_health->currentHealth = newHealth;
		m_health->healthSprite->SetSize(m_health->spriteWidth * m_health->currentHealth / m_health->maxHealth, 10);

		if (m_health->currentHealth == m_health->maxHealth)
		{
			for (VisualEntity* visualEntity : m_visualEntities)
				HideHealthBar(visualEntity);
		}
		else
		{
			for (VisualEntity* visualEntity : m_visualEntities)
				ShowHealthBar(visualEntity);
		}

		if (m_entity->HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = m_entity->GetComponent<ScriptComponent>();
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
		m_entity->GetComponent<InputComponent>().UpdateInputs(inputData);
	}

	void LocalLayerEntity::UpdateParent(const LocalLayerEntity* newParent)
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		if (newParent)
			entityNode.SetParent(newParent->GetEntity(), true);
		else
			entityNode.SetParent(static_cast<Nz::Node*>(nullptr));
	}

	void LocalLayerEntity::UpdateHoveringRenderableHoveringHeight(const Nz::InstancedRenderableRef& renderable, float newHoveringHeight)
	{
		for (auto& hoveringRenderable : m_attachedHoveringRenderables)
		{
			if (hoveringRenderable.data.renderable == renderable)
			{
				for (VisualEntity* visualEntity : m_visualEntities)
					visualEntity->UpdateHoveringRenderableHoveringHeight(renderable, newHoveringHeight);

				hoveringRenderable.hoveringHeight = newHoveringHeight;
				break;
			}
		}
	}

	void LocalLayerEntity::UpdateHoveringRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix)
	{
		for (auto& hoveringRenderable : m_attachedHoveringRenderables)
		{
			if (hoveringRenderable.data.renderable == renderable)
			{
				for (VisualEntity* visualEntity : m_visualEntities)
					visualEntity->UpdateHoveringRenderableMatrix(renderable, offsetMatrix);

				hoveringRenderable.data.offsetMatrix = offsetMatrix;
				break;
			}
		}
	}

	void LocalLayerEntity::UpdateRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix)
	{
		auto it = std::find_if(m_attachedRenderables.begin(), m_attachedRenderables.end(), [&](const RenderableData& renderableData) { return renderableData.renderable == renderable; });
		if (it != m_attachedRenderables.end())
		{
			RenderableData& renderableData = *it;
			renderableData.offsetMatrix = offsetMatrix;

			for (VisualEntity* visualEntity : m_visualEntities)
				visualEntity->UpdateRenderableMatrix(renderable, offsetMatrix);
		}
	}

	void LocalLayerEntity::UpdateScale(float newScale)
	{
		if (m_entity->HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = m_entity->GetComponent<ScriptComponent>();
			scriptComponent.ExecuteCallback<ElementEvent::ScaleUpdate>(newScale);
		}

		auto& node = m_entity->GetComponent<Ndk::NodeComponent>();
		Nz::Vector2f scale = Nz::Vector2f(node.GetScale());
		scale.x = std::copysign(newScale, scale.x);
		scale.y = std::copysign(newScale, scale.y);

		node.SetScale(scale, Nz::CoordSys_Local);

		if (m_entity->HasComponent<CollisionDataComponent>())
		{
			auto& entityCollData = m_entity->GetComponent<CollisionDataComponent>();
			auto& entityCollider = m_entity->GetComponent<Ndk::CollisionComponent2D>();

			entityCollider.SetGeom(entityCollData.BuildCollider(newScale), false, false);
		}
	}

	void LocalLayerEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation)
	{
		if (IsPhysical())
		{
			auto& entityPhys = m_entity->GetComponent<Ndk::PhysicsComponent2D>();
			entityPhys.SetPosition(position);
			entityPhys.SetRotation(rotation);
		}
		else
		{
			auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
			entityNode.SetPosition(position);
			entityNode.SetRotation(rotation);
		}
	}

	void LocalLayerEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation, const Nz::Vector2f& linearVel, const Nz::RadianAnglef& angularVel)
	{
		if (IsPhysical())
		{
			auto& entityPhys = m_entity->GetComponent<Ndk::PhysicsComponent2D>();
			entityPhys.SetAngularVelocity(angularVel);
			entityPhys.SetPosition(position);
			entityPhys.SetRotation(rotation);
			entityPhys.SetVelocity(linearVel);
		}
		else
		{
			auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
			entityNode.SetPosition(position);
			entityNode.SetRotation(rotation);
		}
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

	void LocalLayerEntity::HideHealthBar(VisualEntity* visualEntity)
	{
		visualEntity->DetachHoveringRenderable(m_health->healthSprite);
		visualEntity->DetachHoveringRenderable(m_health->lostHealthSprite);
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
		Nz::Vector2f position = Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys_Global));
		Nz::Vector2f scale = Nz::Vector2f(entityNode.GetScale(Nz::CoordSys_Global));
		Nz::Quaternionf rotation = entityNode.GetRotation(Nz::CoordSys_Global);

		visualEntity->Update(position, rotation, scale);

		for (auto& renderableData : m_attachedRenderables)
			visualEntity->AttachRenderable(renderableData.renderable, renderableData.offsetMatrix, renderableData.renderOrder);

		for (auto& hoveringRenderableData : m_attachedHoveringRenderables)
			visualEntity->AttachHoveringRenderable(hoveringRenderableData.data.renderable, hoveringRenderableData.data.offsetMatrix, hoveringRenderableData.data.renderOrder, hoveringRenderableData.hoveringHeight);

		if (m_health && m_health->currentHealth != m_health->maxHealth)
			ShowHealthBar(visualEntity);
	}

	void LocalLayerEntity::ShowHealthBar(VisualEntity* visualEntity)
	{
		visualEntity->AttachHoveringRenderable(m_health->healthSprite, Nz::Matrix4f::Identity(), 2, 10.f);
		visualEntity->AttachHoveringRenderable(m_health->lostHealthSprite, Nz::Matrix4f::Identity(), 1, 10.f);
	}

	void LocalLayerEntity::UnregisterVisualEntity(VisualEntity* visualEntity)
	{
		auto it = std::find(m_visualEntities.begin(), m_visualEntities.end(), visualEntity);
		assert(it != m_visualEntities.end());
		m_visualEntities.erase(it);
	}
}
