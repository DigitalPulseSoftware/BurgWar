// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Float4SpinBox.hpp>

namespace bw
{
	inline Float4SpinBox::Float4SpinBox(const Nz::Vector4f& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
	Float4SpinBox(labelMode, dir, parent)
	{
		setValue(value);
	}
}
