// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Integer3SpinBox.hpp>

namespace bw
{
	inline Integer3SpinBox::Integer3SpinBox(const Nz::Vector3i64& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
	Integer3SpinBox(labelMode, dir, parent)
	{
		setValue(value);
	}
}