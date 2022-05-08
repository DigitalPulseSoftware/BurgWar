// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EntityInfoDialog.hpp>

namespace bw
{
	inline LayerIndex EntityInfoDialog::GetLayerIndex() const
	{
		return m_entityLayer;
	}

	inline const Nz::Vector2f& EntityInfoDialog::GetPosition() const
	{
		return m_entityInfo.position;
	}

	inline const Nz::DegreeAnglef& EntityInfoDialog::GetRotation() const
	{
		return m_entityInfo.rotation;
	}

	entt::entity EntityInfoDialog::GetTargetEntity() const
	{
		return m_targetEntity;
	}
}
