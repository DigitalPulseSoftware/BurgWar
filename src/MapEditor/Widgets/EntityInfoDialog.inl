// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EntityInfoDialog.hpp>

namespace bw
{
	inline const Nz::Vector2f& EntityInfoDialog::GetPosition() const
	{
		return m_entityInfo.position;
	}

	inline const Nz::DegreeAnglef& EntityInfoDialog::GetRotation() const
	{
		return m_entityInfo.rotation;
	}

	const Ndk::EntityHandle& EntityInfoDialog::GetTargetEntity() const
	{
		return m_targetEntity;
	}
}
