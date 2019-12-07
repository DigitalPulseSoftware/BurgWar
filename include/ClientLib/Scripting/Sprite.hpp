// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_SPRITE_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_SPRITE_HPP

#include <ClientLib/LocalLayerEntity.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Math/Matrix4.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class Sprite
	{
		public:
			inline Sprite(LocalLayerEntityHandle entity, Nz::SpriteRef sprite, const Nz::Matrix4f& transformMatrix, int renderOrder);
			Sprite(const Sprite&) = default;
			Sprite(Sprite&&) = default;
			~Sprite() = default;

			inline Nz::Vector2f GetOrigin() const;
			inline const Nz::Vector2f& GetSize() const;

			inline void Hide();

			//bool IsVisible() const;

			inline void SetSize(const Nz::Vector2f& newSize);
			void Show(bool show = true);

			Sprite& operator=(const Sprite&) = default;
			Sprite& operator=(Sprite&&) = default;

		private:
			LocalLayerEntityHandle m_entity;
			Nz::Matrix4f m_transformMatrix;
			Nz::SpriteRef m_sprite;
			int m_renderOrder;
	};
}

#include <ClientLib/Scripting/Sprite.inl>

#endif
