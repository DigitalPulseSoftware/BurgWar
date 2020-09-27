// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindowPrefabs.hpp>
#include <cassert>

namespace bw
{
	inline EditorWindowPrefabs::EditorWindowPrefabs(EditorWindow* parent) :
	m_parent(parent)
	{
		assert(parent);
	}
}
