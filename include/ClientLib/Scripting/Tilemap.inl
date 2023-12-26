// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Tilemap.hpp>

namespace bw
{
	inline Tilemap::Tilemap(LayerVisualEntityHandle visualEntity, std::shared_ptr<Nz::Tilemap> tilemap, const Nz::Vector2f& offset, const Nz::DegreeAnglef& rotation) :
	m_visualEntity(std::move(visualEntity)),
	m_rotation(rotation),
	m_offset(offset),
	m_tilemap(std::move(tilemap)),
	m_isVisible(false)
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
		return m_visualEntity;
	}

	inline bool Tilemap::IsVisible() const
	{
		return m_isVisible;
	}

	inline void Tilemap::SetOffset(const Nz::Vector2f& newOffset)
	{
		m_offset = newOffset;

		UpdateTransform();
	}

	inline void Tilemap::SetRotation(const Nz::DegreeAnglef& newRotation)
	{
		m_rotation = newRotation;

		UpdateTransform();
	}
}
