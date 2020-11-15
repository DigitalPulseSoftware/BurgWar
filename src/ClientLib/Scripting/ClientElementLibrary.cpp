// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientElementLibrary.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Components/LayerEntityComponent.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/Scripting/Sound.hpp>
#include <ClientLib/Scripting/Sprite.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <Thirdparty/sol3/sol.hpp>

namespace bw
{
	void ClientElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedElementLibrary::RegisterLibrary(elementMetatable);

		RegisterClientLibrary(elementMetatable);
	}

	void ClientElementLibrary::RegisterClientLibrary(sol::table& elementTable)
	{
		elementTable["AddSprite"] = ExceptToLuaErr([this](const sol::table& entityTable, const sol::table& parameters)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);

			std::string texturePath = parameters.get_or("TexturePath", std::string{});
			int renderOrder = parameters.get_or("RenderOrder", 0);
			Nz::Vector2f offset = parameters.get_or("Offset", Nz::Vector2f(0.f, 0.f));
			Nz::DegreeAnglef rotation = parameters.get_or("Rotation", Nz::DegreeAnglef::Zero());
			Nz::Vector2f origin = parameters.get_or("Origin", Nz::Vector2f(0.5f, 0.5f));
			Nz::Vector2f scale = parameters.get_or("Scale", Nz::Vector2f::Unit());
			Nz::Rectf textureCoords = parameters.get_or("TextureCoords", Nz::Rectf(0.f, 0.f, 1.f, 1.f));

			Nz::Matrix4 transformMatrix = Nz::Matrix4f::Transform(offset, rotation);

			Nz::Color color;
			if (std::optional<Nz::Color> colorParameter = parameters.get_or<std::optional<Nz::Color>>("Color", std::nullopt); colorParameter)
				color = colorParameter.value();
			else
				color = Nz::Color::White;

			Nz::MaterialRef mat = Nz::Material::New("Translucent2D");
			if (!texturePath.empty())
				mat->SetDiffuseMap(m_assetStore.GetTexture(texturePath));

			auto& sampler = mat->GetDiffuseSampler();
			sampler.SetFilterMode(Nz::SamplerFilter_Bilinear);
			//sampler.SetWrapMode(Nz::SamplerWrap_Repeat);

			Nz::SpriteRef sprite = Nz::Sprite::New();
			sprite->SetColor(color);
			sprite->SetMaterial(mat);
			sprite->SetTextureCoords(textureCoords);

			if (std::optional<sol::table> cornerColorTable = parameters.get_or<std::optional<sol::table>>("CornerColor", std::nullopt); cornerColorTable)
			{
				struct Corner
				{
					const char* varName;
					Nz::RectCorner rectCorner;
				};

				std::array<Corner, 4> corners = {
					{
						{
							"TopLeft",
							Nz::RectCorner_LeftTop
						},
						{
							"TopRight",
							Nz::RectCorner_RightTop
						},
						{
							"BottomLeft",
							Nz::RectCorner_LeftBottom
						},
						{
							"BottomRight",
							Nz::RectCorner_RightBottom
						}
					}
				};

				for (const auto& corner : corners)
				{
					if (std::optional<Nz::Color> colorParameter = cornerColorTable->get_or<std::optional<Nz::Color>>(corner.varName, std::nullopt); colorParameter)
						sprite->SetCornerColor(corner.rectCorner, colorParameter.value());
				}
			}

			Nz::Vector2f size = parameters.get_or("Size", sprite->GetSize());

			sprite->SetSize(size * scale);
			sprite->SetOrigin(sprite->GetSize() * origin);

			//FIXME (FIXME (FIXME))
			if (entity->HasComponent<LayerEntityComponent>())
			{
				auto& layerEntityComponent = entity->GetComponent<LayerEntityComponent>();

				Sprite scriptSprite(layerEntityComponent.GetLayerEntity(), sprite, transformMatrix, renderOrder);
				scriptSprite.Show();

				return scriptSprite;
			}
			else
			{
				entity->GetComponent<Ndk::GraphicsComponent>().Attach(sprite, transformMatrix, renderOrder);

				return Sprite({}, sprite, transformMatrix, renderOrder);
			}
		});

		elementTable["AddModel"] = ExceptToLuaErr([this](const sol::table& entityTable, const sol::table& parameters)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);

			std::string modelPath = parameters["ModelPath"];
			int renderOrder = parameters.get_or("RenderOrder", 0);
			Nz::Vector3f offset = parameters.get_or("Offset", Nz::Vector3f::Zero());
			Nz::Vector3f rotation = parameters.get_or("Rotation", Nz::Vector3f::Zero()); //< TODO: Euler angles
			Nz::Vector3f scale = parameters.get_or("Scale", Nz::Vector3f::Unit());

			Nz::ModelRef model = m_assetStore.GetModel(modelPath);
			if (!model)
				return;

			Nz::Matrix4 transformMatrix = Nz::Matrix4f::Transform(offset, Nz::EulerAnglesf(rotation.x, rotation.y, rotation.z), scale);

			if (entity->HasComponent<LayerEntityComponent>())
			{
				auto& layerEntityComponent = entity->GetComponent<LayerEntityComponent>();
				layerEntityComponent.GetLayerEntity()->AttachRenderable(model, transformMatrix, renderOrder);
			}
			else
				entity->GetComponent<Ndk::GraphicsComponent>().Attach(model, transformMatrix, renderOrder);
		});

		auto DealDamage = [](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 /*damage*/, Nz::Rectf damageZone, float pushbackForce = 0.f)
		{
			// Client-side this function only applies push-back forces
			if (Nz::NumberEquals(pushbackForce, 0.f))
				return;

			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
			Ndk::World* world = entity->GetWorld();
			assert(world);

			Ndk::EntityList hitEntities; //< FIXME: RegionQuery hit multiples entities

			world->GetSystem<Ndk::PhysicsSystem2D>().RegionQuery(damageZone, 0, 0xFFFFFFFF, 0xFFFFFFFF, [&](const Ndk::EntityHandle& hitEntity)
			{
				if (hitEntities.Has(hitEntity))
					return;

				hitEntities.Insert(hitEntity);

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitEntityPhys.GetMassCenter(Nz::CoordSys_Global) - origin) * pushbackForce);
				}
			});
		};

		elementTable["DealDamage"] = sol::overload(
			ExceptToLuaErr(DealDamage),
			ExceptToLuaErr([=](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone) { DealDamage(entityTable, origin, damage, damageZone); })
		);

		elementTable["GetLayerIndex"] = ExceptToLuaErr([](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);

			return entity->GetComponent<LocalMatchComponent>().GetLayerIndex();
		});
		
		elementTable["GetProperty"] = ExceptToLuaErr([](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			Ndk::EntityHandle entity = AssertScriptEntity(table);

			auto& entityScript = entity->GetComponent<ScriptComponent>();

			auto propertyVal = entityScript.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const PropertyValue& property = propertyVal.value();

				LocalMatch* match;
				if (entity->HasComponent<LocalMatchComponent>())
					match = &entity->GetComponent<LocalMatchComponent>().GetLocalMatch();
				else
					match = nullptr;

				return TranslatePropertyToLua(match, lua, property);
			}
			else
				return sol::nil;
		});

		elementTable["PlaySound"] = ExceptToLuaErr([this](sol::this_state L, const sol::table& entityTable, const std::string& soundPath, bool isAttachedToEntity, bool isLooping, bool isSpatialized)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
			auto& entityMatch = entity->GetComponent<LocalMatchComponent>();

			const Nz::SoundBufferRef& soundBuffer = m_assetStore.GetSoundBuffer(soundPath);
			if (!soundBuffer)
				TriggerLuaArgError(L, 1, "failed to load " + soundPath);

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
		});
	}

	void ClientElementLibrary::SetScale(const Ndk::EntityHandle& entity, float newScale)
	{
		auto& layerEntityComponent = entity->GetComponent<LayerEntityComponent>();
		layerEntityComponent.GetLayerEntity()->UpdateScale(newScale);

		if (entity->HasComponent<WeaponWielderComponent>())
		{
			auto& wielderComponent = entity->GetComponent<WeaponWielderComponent>();
			for (const Ndk::EntityHandle& weapon : wielderComponent.GetWeapons())
				SetScale(weapon, newScale);
		}
	}
}
