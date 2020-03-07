// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_EDITORCONFIGFILE_HPP
#define BURGWAR_EDITORCONFIGFILE_HPP

#include <CoreLib/SharedConfigFile.hpp>

namespace bw
{
	class EditorWindow;

	class EditorConfigFile : public SharedConfigFile
	{
		public:
			EditorConfigFile(EditorWindow& app);
			~EditorConfigFile() = default;
	};
}

#include <MapEditor/EditorConfigFile.inl>

#endif
