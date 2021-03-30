// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorEntityLibrary.hpp>

namespace bw
{
	inline EditorEntityLibrary::EditorEntityLibrary(EditorWindow& editorWindow, const Logger& logger, ClientAssetStore& assetStore) :
	ClientEntityLibrary(logger, assetStore),
	m_editorWindow(editorWindow)
	{
	}
}
