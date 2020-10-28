// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#if !defined(BURGWAR_PROPERTYTYPE)
#error You must define BURGWAR_PROPERTYTYPE before including this file
#endif

#ifndef BURGWAR_PROPERTYTYPE_LAST
#define BURGWAR_PROPERTYTYPE_LAST(V, X, UT) BURGWAR_PROPERTYTYPE(V, X, UT)
#endif

BURGWAR_PROPERTYTYPE( 0, Bool, bool)
BURGWAR_PROPERTYTYPE( 1, Entity, EntityId)
BURGWAR_PROPERTYTYPE( 2, Float, float)
BURGWAR_PROPERTYTYPE( 3, FloatPosition, Nz::Vector2f)
BURGWAR_PROPERTYTYPE( 4, FloatPosition3D, Nz::Vector3f)
BURGWAR_PROPERTYTYPE( 5, FloatRect, Nz::Vector4f)
BURGWAR_PROPERTYTYPE( 6, FloatSize, Nz::Vector2f)
BURGWAR_PROPERTYTYPE( 7, FloatSize3D, Nz::Vector3f)
BURGWAR_PROPERTYTYPE( 8, Integer, Nz::Int64)
BURGWAR_PROPERTYTYPE( 9, IntegerPosition, Nz::Vector2i64)
BURGWAR_PROPERTYTYPE(10, IntegerPosition3D, Nz::Vector3i64)
BURGWAR_PROPERTYTYPE(11, IntegerRect, Nz::Vector4i64)
BURGWAR_PROPERTYTYPE(12, IntegerSize, Nz::Vector2i64)
BURGWAR_PROPERTYTYPE(13, IntegerSize3D, Nz::Vector3i64)
BURGWAR_PROPERTYTYPE(14, Layer, LayerIndex)
BURGWAR_PROPERTYTYPE(15, String, std::string)
BURGWAR_PROPERTYTYPE_LAST(16, Texture, std::string)

#undef BURGWAR_PROPERTYTYPE
#undef BURGWAR_PROPERTYTYPE_LAST
