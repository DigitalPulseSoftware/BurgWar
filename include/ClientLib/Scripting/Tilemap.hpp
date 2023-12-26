// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_TILEMAP_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_TILEMAP_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/LayerVisualEntity.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Math/Matrix4.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <entt/entt.hpp>
#include <sol/forward.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API Tilemap
	{
		public:
			inline Tilemap(LayerVisualEntityHandle visualEntity, std::shared_ptr<Nz::Tilemap> tilemap, const Nz::Vector2f& offset, const Nz::DegreeAnglef& rotation);
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
			void UpdateTransform();

			LayerVisualEntityHandle m_visualEntity;
			Nz::DegreeAnglef m_rotation;
			Nz::Vector2f m_offset;
			std::shared_ptr<Nz::Tilemap> m_tilemap;
			bool m_isVisible;
	};
}

#include <ClientLib/Scripting/Tilemap.inl>

#endif
