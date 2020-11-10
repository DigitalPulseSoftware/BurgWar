// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Tilemap.hpp>

namespace bw
{
	inline Tilemap::Tilemap(LocalLayerEntityHandle entity, Nz::TileMapRef tilemap, const Nz::Matrix4f& transformMatrix, int renderOrder) :
	m_entity(std::move(entity)),
	m_transformMatrix(transformMatrix),
	m_tilemap(std::move(tilemap)),
	m_renderOrder(renderOrder)
	{
	}

	inline const Nz::Vector2ui& Tilemap::GetMapSize() const
	{
		return m_tilemap->GetMapSize();
	}

	inline Nz::Vector2f Tilemap::GetSize() const
	{
		return m_tilemap->GetSize();
	}

	inline const Nz::Vector2f& Tilemap::GetTileSize() const
	{
		return m_tilemap->GetTileSize();
	}

	inline void Tilemap::Hide()
	{
		return Show(false);
	}

	inline bool Tilemap::IsValid() const
	{
		return m_entity;
	}

	inline bool Tilemap::IsVisible() const
	{
		return m_isVisible;
	}

	inline void Tilemap::SetOffset(const Nz::Vector2f& newOffset)
	{
		m_transformMatrix.SetTranslation(newOffset);

		UpdateTransformMatrix();
	}

	inline void Tilemap::SetRotation(const Nz::DegreeAnglef& newRotation)
	{
		m_transformMatrix.SetRotation(newRotation);

		UpdateTransformMatrix();
	}
}
