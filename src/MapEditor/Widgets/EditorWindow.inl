// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindow.hpp>

namespace bw
{
	inline const EditorEntityStore& EditorWindow::GetEntityStore() const
	{
		return *m_entityStore;
	}
}
