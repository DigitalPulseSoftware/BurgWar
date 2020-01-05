// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Float3SpinBox.hpp>

namespace bw
{
	inline Float3SpinBox::Float3SpinBox(const Nz::Vector3f& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
	Float3SpinBox(labelMode, dir, parent)
	{
		setValue(value);
	}
}