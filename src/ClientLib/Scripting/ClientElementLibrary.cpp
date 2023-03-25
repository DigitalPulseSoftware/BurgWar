// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientElementLibrary.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <ClientLib/Components/ClientMatchComponent.hpp>
#include <ClientLib/Components/ClientOwnerComponent.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <ClientLib/Scripting/Sound.hpp>
#include <ClientLib/Scripting/Sprite.hpp>
#include <ClientLib/Scripting/Text.hpp>
#include <Nazara/Graphics/Components/GraphicsComponent.hpp>
#include <Nazara/Physics2D/Components/RigidBody2DComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <sol/sol.hpp>

namespace bw
{
	void ClientElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedElementLibrary::RegisterLibrary(elementMetatable);

		RegisterClientLibrary(elementMetatable);
	}

	void ClientElementLibrary::RegisterClientLibrary(sol::table& elementTable)
	{
		elementTable["AddModel"] = LuaFunction([this](const sol::table& entityTable, const sol::table& parameters)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			std::string modelPath = parameters["ModelPath"];
			int renderOrder = parameters.get_or("RenderOrder", 0);
			Nz::Vector3f offset = parameters.get_or("Offset", Nz::Vector3f::Zero());
			Nz::Vector3f rotation = parameters.get_or("Rotation", Nz::Vector3f::Zero()); //< TODO: Euler angles
			Nz::Vector3f scale = parameters.get_or("Scale", Nz::Vector3f::Unit());

			std::shared_ptr<Nz::Model> model = m_assetStore.GetModel(modelPath);
			if (!model)
				return;

			Nz::Matrix4 transformMatrix = Nz::Matrix4f::Transform(offset, Nz::EulerAnglesf(rotation.x, rotation.y, rotation.z), scale);

			auto& visualComponent = entity.get<VisualComponent>();
			visualComponent.GetLayerVisual()->AttachRenderable(model, transformMatrix, renderOrder);
		});

		auto DealDamage = [](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 /*damage*/, Nz::Rectf damageZone, float pushbackForce = 0.f)
		{
			// Client-side this function only applies push-back forces
			if (Nz::NumberEquals(pushbackForce, 0.f))
				return;

			entt::handle entity = AssertScriptEntity(entityTable);
			/*entt::registry* world = entity->GetWorld();
			assert(world);

			Ndk::EntityList hitEntities; //< FIXME: RegionQuery hit multiples entities

			world->GetSystem<Ndk::PhysicsSystem2D>().RegionQuery(damageZone, 0, 0xFFFFFFFF, 0xFFFFFFFF, [&](entt::entity hitEntity)
			{
				if (hitEntities.Has(hitEntity))
					return;

				hitEntities.Insert(hitEntity);

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity.get<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitEntityPhys.GetMassCenter(Nz::CoordSys::Global) - origin) * pushbackForce);
				}
			});*/
		};
		
		elementTable["AddSprite"] = LuaFunction([this](const sol::table& entityTable, const sol::table& parameters)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			std::string texturePath = parameters.get_or("TexturePath", std::string{});
			int renderOrder = parameters.get_or("RenderOrder", 0);
			Nz::Vector2f offset = parameters.get_or("Offset", Nz::Vector2f(0.f, 0.f));
			Nz::DegreeAnglef rotation = parameters.get_or("Rotation", Nz::DegreeAnglef::Zero());
			Nz::Vector2f origin = parameters.get_or("Origin", Nz::Vector2f(0.5f, 0.5f));
			Nz::Vector2f scale = parameters.get_or("Scale", Nz::Vector2f::Unit());
			Nz::Rectf textureCoords = parameters.get_or("TextureCoords", Nz::Rectf(0.f, 0.f, 1.f, 1.f));
			bool repeatTexture = parameters.get_or("RepeatTexture", false);

			Nz::Matrix4 transformMatrix = Nz::Matrix4f::Transform(offset, rotation);

			Nz::Color color;
			if (std::optional<Nz::Color> colorParameter = parameters.get_or<std::optional<Nz::Color>>("Color", std::nullopt); colorParameter)
				color = colorParameter.value();
			else
				color = Nz::Color::White();

			//TODO: Don't create a material everytime
			/*Nz::MaterialRef mat = Nz::Material::New("Translucent2D");
			if (!texturePath.empty())
				mat->SetDiffuseMap(m_assetStore.GetTexture(texturePath));

			auto& sampler = mat->GetDiffuseSampler();
			sampler.SetFilterMode(Nz::SamplerFilter_Bilinear);
			if (repeatTexture)
				sampler.SetWrapMode(Nz::SamplerWrap_Repeat);

			std::shared_ptr<Nz::Sprite> sprite = std::make_shared<Nz::Sprite>();
			sprite->SetColor(color);
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
							Nz::RectCorner::LeftTop
						},
						{
							"TopRight",
							Nz::RectCorner::RightTop
						},
						{
							"BottomLeft",
							Nz::RectCorner::LeftBottom
						},
						{
							"BottomRight",
							Nz::RectCorner::RightBottom
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

			auto& visualComponent = entity.get<VisualComponent>();

			Sprite scriptSprite(visualComponent.GetLayerVisual(), sprite, transformMatrix, renderOrder);
			scriptSprite.Show();

			return scriptSprite;*/
		});

		elementTable["AddText"] = LuaFunction([this](const sol::table& entityTable, const sol::table& parameters)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			std::string fontName = parameters.get_or("Font", std::string{});
			int renderOrder = parameters.get_or("RenderOrder", 0);
			Nz::Vector2f offset = parameters.get_or("Offset", Nz::Vector2f(0.f, 0.f));
			Nz::DegreeAnglef rotation = parameters.get_or("Rotation", Nz::DegreeAnglef::Zero());
			std::string text = parameters["Text"];
			bool isHovering = parameters.get_or("Hovering", false);

			Nz::Matrix4 transformMatrix = Nz::Matrix4f::Transform(offset, rotation);

			Nz::Color color;
			if (std::optional<Nz::Color> colorParameter = parameters.get_or<std::optional<Nz::Color>>("Color", std::nullopt); colorParameter)
				color = colorParameter.value();
			else
				color = Nz::Color::White();

			float outlineThickness = parameters.get_or("OutlineThickness", 0.f);

			Nz::Color outlineColor;
			if (std::optional<Nz::Color> colorParameter = parameters.get_or<std::optional<Nz::Color>>("OutlineColor", std::nullopt); colorParameter)
				outlineColor = colorParameter.value();
			else
				outlineColor = Nz::Color::Black();

			/*Nz::FontRef font;
			if (!fontName.empty())
			{
				font = Nz::FontLibrary::Get(fontName);
				if (!font)
					bwLog(GetLogger(), LogLevel::Warning, "unknown font \"{}\"", fontName);
			}*/

			Nz::SimpleTextDrawer drawer;
			drawer.SetColor(color);
			drawer.SetOutlineColor(outlineColor);
			drawer.SetOutlineThickness(outlineThickness);
			drawer.SetText(text);

			//if (font)
			//	drawer.SetFont(font);

			std::shared_ptr<Nz::TextSprite> textSprite = std::make_shared<Nz::TextSprite>();
			textSprite->Update(drawer);

			auto& visualComponent = entity.get<VisualComponent>();

			Text scriptText(visualComponent.GetLayerVisual(), std::move(drawer), std::move(textSprite), transformMatrix, renderOrder, isHovering);
			scriptText.Show();

			return scriptText;
		});

		elementTable["DealDamage"] = sol::overload(
			LuaFunction(DealDamage),
			LuaFunction([=](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone) { DealDamage(entityTable, origin, damage, damageZone); })
		);

		elementTable["GetGlobalBounds"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			auto& visualComponent = entity.get<VisualComponent>();
			
			const auto& layerVisualHandle = visualComponent.GetLayerVisual();
			if (!layerVisualHandle)
			{
				// Fallback on position
				auto& nodeComponent = entity.get<Nz::NodeComponent>();
				Nz::Vector2f position = Nz::Vector2f(nodeComponent.GetPosition(Nz::CoordSys::Global));
				return Nz::Rectf(position.x, position.y, 0.f, 0.f);
			}

			Nz::Boxf globalBounds = layerVisualHandle->GetGlobalBounds();
			return Nz::Rectf(globalBounds.x, globalBounds.y, globalBounds.width, globalBounds.height);
		});

		elementTable["GetLayerIndex"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			return entity.get<ClientMatchComponent>().GetLayerIndex();
		});
		
		elementTable["GetLocalBounds"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			auto& visualComponent = entity.get<VisualComponent>();
			
			const auto& layerVisualHandle = visualComponent.GetLayerVisual();
			if (!layerVisualHandle)
				return Nz::Rectf::Zero();

			Nz::Boxf localBounds = layerVisualHandle->GetLocalBounds();
			return Nz::Rectf(localBounds.x, localBounds.y, localBounds.width, localBounds.height);
		});
		
		elementTable["GetOwner"] = LuaFunction([](sol::this_state s, const sol::table& table) -> sol::object
		{
			entt::handle entity = AssertScriptEntity(table);

			if (ClientOwnerComponent* clientOwner = entity.try_get<ClientOwnerComponent>())
				return sol::make_object(s, clientOwner->GetOwner()->CreateHandle());
			else
				return sol::nil;
		});

		elementTable["GetProperty"] = LuaFunction([](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			entt::handle entity = AssertScriptEntity(table);

			auto& entityScript = entity.get<ScriptComponent>();

			auto propertyVal = entityScript.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const PropertyValue& property = propertyVal.value();

				ClientMatch* match;
				if (ClientMatchComponent* matchComponent = entity.try_get<ClientMatchComponent>())
					match = &matchComponent->GetClientMatch();
				else
					match = nullptr;

				return TranslatePropertyToLua(match, lua, property);
			}
			else
				return sol::nil;
		});

		elementTable["PlaySound"] = LuaFunction([this](sol::this_state L, const sol::table& entityTable, const std::string& soundPath, bool isAttachedToEntity, bool isLooping, bool isSpatialized)
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			auto& entityMatch = entity.get<ClientMatchComponent>();

			const std::shared_ptr<Nz::SoundBuffer>& soundBuffer = m_assetStore.GetSoundBuffer(soundPath);
			if (!soundBuffer)
				TriggerLuaArgError(L, 1, "failed to load " + soundPath);

			auto& entityNode = entity.get<Nz::NodeComponent>();

			auto& layer = entityMatch.GetLayer();

			std::optional<ClientLayerSound> localLayerSound;
			if (isAttachedToEntity)
				localLayerSound.emplace(layer, entityNode);
			else
				localLayerSound.emplace(layer, Nz::Vector2f(entityNode.GetPosition()));

			auto& layerSound = layer.RegisterSound(std::move(localLayerSound.value()));

			std::size_t soundIndex = layerSound.PlaySound(soundBuffer, isLooping, isSpatialized);
			return Sound(layerSound.CreateHandle(), soundIndex);
		});
	}

	void ClientElementLibrary::SetScale(entt::handle entity, float newScale)
	{
		auto& visualComponent = entity.get<VisualComponent>();
		visualComponent.GetLayerVisual()->UpdateScale(newScale);

		if (WeaponWielderComponent* wielderComponent = entity.try_get<WeaponWielderComponent>())
		{
			for (const EntityOwner& weapon : wielderComponent->GetWeapons())
				SetScale(weapon, newScale);
		}
	}
}
