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
#include <CoreLib/Utils.hpp>
#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components.hpp>

namespace bw
{
	LocalLayerEntity::LocalLayerEntity(LocalLayer& layer, const Ndk::EntityHandle& entity, Nz::UInt32 serverEntityId) :
	m_entity(entity),
	m_serverEntityId(serverEntityId),
	m_layer(layer)
	{
	}

	LocalLayerEntity::~LocalLayerEntity()
	{
		if (m_ghostEntity)
			m_layer.OnEntityDelete(&m_layer, *m_ghostEntity);
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
		if (it != m_attachedRenderables.end())
		{
			m_attachedRenderables.erase(it);

			for (VisualEntity* visualEntity : m_visualEntities)
				visualEntity->DetachRenderable(renderable);
		}
	}

	void LocalLayerEntity::Enable(bool enable)
	{
		if (m_entity->IsEnabled() == enable)
			return;

		m_entity->Enable(enable);
		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->Enable(enable);
	}

	LocalLayerEntity* LocalLayerEntity::GetGhost()
	{
		if (!m_ghostEntity)
		{
			const Ndk::EntityHandle& ghostEntity = m_entity->GetWorld()->CreateEntity();
			ghostEntity->AddComponent<Ndk::NodeComponent>();

			m_ghostEntity = std::make_unique<LocalLayerEntity>(m_layer, ghostEntity, ClientsideId);

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

					m_ghostEntity->AttachRenderable(clonedRenderable.release(), renderable.offset, -1);
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

	Nz::Vector2f LocalLayerEntity::GetPosition() const
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		return Nz::Vector2f(entityNode.GetPosition() - entityNode.GetInitialPosition()); //< FIXME
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

	void LocalLayerEntity::InitializeName(const std::string& name)
	{
		auto& nameData = m_name.emplace();

		nameData.nameSprite = Nz::TextSprite::New();
		nameData.nameSprite->Update(Nz::SimpleTextDrawer::Draw(name, 24, Nz::TextStyle_Regular, Nz::Color::White, 2.f, Nz::Color::Black));

		Nz::Boxf textBox = nameData.nameSprite->GetBoundingVolume().obb.localBox;

		for (VisualEntity* visualEntity : m_visualEntities)
			ShowName(visualEntity, textBox);
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
		{
			auto& visualNode = visualEntity->GetEntity()->GetComponent<Ndk::NodeComponent>();
			visualEntity->Update(position, rotation, scale);
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
			scriptComponent.ExecuteCallback("OnHealthUpdate", oldHealth, newHealth);
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

	void LocalLayerEntity::UpdateRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix)
	{
		auto it = std::find_if(m_attachedRenderables.begin(), m_attachedRenderables.end(), [&](const RenderableData& renderableData) { return renderableData.renderable == renderable; });
		if (it != m_attachedRenderables.end())
		{
			RenderableData& renderableData = *it;
			renderableData.offset = offsetMatrix;

			for (VisualEntity* visualEntity : m_visualEntities)
				visualEntity->UpdateRenderableMatrix(renderable, offsetMatrix);
		}
	}

	void LocalLayerEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation)
	{
		//FIXME: Handle this in a better way? (what if server entity is physical but client entity is not?)
		assert(!IsPhysical());

		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		entityNode.SetPosition(position);
		entityNode.SetRotation(rotation);
	}

	void LocalLayerEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation, const Nz::Vector2f& linearVel, const Nz::RadianAnglef& angularVel)
	{
		//FIXME: Handle this in a better way? (what if server entity is physical but client entity is not?)
		assert(IsPhysical());

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

	void LocalLayerEntity::HideHealthBar(VisualEntity* visualEntity)
	{
		visualEntity->DetachHoveringRenderables({ m_health->healthSprite, m_health->lostHealthSprite });
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

		if (m_health && m_health->currentHealth != m_health->maxHealth)
			ShowHealthBar(visualEntity);

		if (m_name)
		{
			auto& nameData = m_name.value();
			const Nz::Boxf& textBox = nameData.nameSprite->GetBoundingVolume().obb.localBox;

			ShowName(visualEntity, textBox);
		}
	}

	void LocalLayerEntity::ShowHealthBar(VisualEntity* visualEntity)
	{
		visualEntity->AttachHoveringRenderables({ m_health->healthSprite, m_health->lostHealthSprite }, { Nz::Matrix4f::Identity(), Nz::Matrix4f::Identity() }, 10.f, { 2, 1 });
	}

	void LocalLayerEntity::ShowName(VisualEntity* visualEntity, const Nz::Boxf& textBox)
	{
		visualEntity->AttachHoveringRenderable(m_name->nameSprite, Nz::Matrix4f::Translate(Nz::Vector2f(-textBox.width / 2.f, -textBox.height)), 20.f);
	}

	void LocalLayerEntity::UnregisterVisualEntity(VisualEntity* visualEntity)
	{
		auto it = std::find(m_visualEntities.begin(), m_visualEntities.end(), visualEntity);
		assert(it != m_visualEntities.end());
		m_visualEntities.erase(it);
	}
}
