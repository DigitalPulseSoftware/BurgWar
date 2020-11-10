// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_TILEMAP_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_TILEMAP_HPP

#include <ClientLib/LocalLayerEntity.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Math/Matrix4.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <NDK/Entity.hpp>
#include <Thirdparty/sol3/forward.hpp>

namespace bw
{
	class Tilemap
	{
		public:
			inline Tilemap(LocalLayerEntityHandle entity, Nz::TileMapRef tilemap, const Nz::Matrix4f& transformMatrix, int renderOrder);
			Tilemap(const Tilemap&) = delete;
			Tilemap(Tilemap&&) noexcept = default;
			~Tilemap() = default;

			inline const Nz::Vector2ui& GetMapSize() const;
			inline Nz::Vector2f GetSize() const;
			inline const Nz::Vector2f& GetTileSize() const;

			inline void Hide();

			inline bool IsValid() const;
			inline bool IsVisible() const;

			inline void SetOffset(const Nz::Vector2f& newOffset);
			inline void SetRotation(const Nz::DegreeAnglef& newRotation);
			void SetTileColor(unsigned int x, unsigned int y, const Nz::Color& color);
			void Show(bool show = true);

			Tilemap& operator=(const Tilemap&) = delete;
			Tilemap& operator=(Tilemap&&) noexcept = default;

		private:
			void UpdateTransformMatrix();

			LocalLayerEntityHandle m_entity;
			Nz::Matrix4f m_transformMatrix;
			Nz::TileMapRef m_tilemap;
			int m_renderOrder;
			bool m_isVisible;
	};
}

#include <ClientLib/Scripting/Tilemap.inl>

#endif
