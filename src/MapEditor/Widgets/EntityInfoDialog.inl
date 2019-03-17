// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EntityInfoDialog.hpp>

namespace bw
{
	const EntityInfo& EntityInfoDialog::GetEntityInfo() const
	{
		return m_entityInfo;
	}

	inline const Nz::Vector2f& EntityInfoDialog::GetEntityPosition() const
	{
		return m_entityInfo.position;
	}

	inline const Nz::DegreeAnglef& EntityInfoDialog::GetEntityRotation() const
	{
		return m_entityInfo.rotation;
	}

	const Ndk::EntityHandle& EntityInfoDialog::GetTargetEntity() const
	{
		return m_targetEntity;
	}
}