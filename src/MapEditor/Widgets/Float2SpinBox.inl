// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Float2SpinBox.hpp>

namespace bw
{
	inline Float2SpinBox::Float2SpinBox(const Nz::Vector2f& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
	Float2SpinBox(labelMode, dir, parent)
	{
		setValue(value);
	}
}
