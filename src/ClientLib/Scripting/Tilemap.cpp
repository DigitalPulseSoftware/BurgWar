// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Tilemap.hpp>
#include <Nazara/Core/Color.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <optional>
#include <stdexcept>

namespace bw
{
	void Tilemap::Show(bool show)
	{
		if (!m_entity)
			throw std::runtime_error("Invalid sprite");

		if (show == m_isVisible)
			return;

		if (show)
			m_entity->AttachRenderable(m_tilemap, m_transformMatrix, m_renderOrder);
		else
			m_entity->DetachRenderable(m_tilemap);

		m_isVisible = show;
	}

	void Tilemap::SetTileColor(unsigned int x, unsigned int y, const Nz::Color& color)
	{
		const Nz::Vector2ui& mapSize = m_tilemap->GetMapSize();
		if (x >= mapSize.x || y >= mapSize.y)
			throw std::runtime_error("tile position out of range");

		Nz::Vector2ui tilePos(x, y);

		const auto& tileData = m_tilemap->GetTile(tilePos);
		if (!tileData.enabled)
			return;

		m_tilemap->EnableTile(tilePos, tileData.textureCoords, color, tileData.layerIndex);
	}

	void Tilemap::UpdateTransformMatrix()
	{
		m_entity->UpdateRenderableMatrix(m_tilemap, m_transformMatrix);
	}
}
