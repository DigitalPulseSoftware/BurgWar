RegisterClientScript()

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 1000

ENTITY.Properties = {
	{ Name = "bool", Type = PropertyType.Boolean, Default = false, Shared = true },
	{ Name = "bool_array", Type = PropertyType.Boolean, Array = true, Default = { false }, Shared = true },
	{ Name = "entity", Type = PropertyType.Entity, Default = NoEntity, Shared = true },
	{ Name = "entity_array", Type = PropertyType.Entity, Array = true, Default = { NoEntity }, Shared = true },
	{ Name = "float", Type = PropertyType.Float, Default = 0, Shared = true },
	{ Name = "float_array", Type = PropertyType.Float, Array = true, Default = { 0 }, Shared = true },
	{ Name = "float_position", Type = PropertyType.FloatPosition, Default = Vec2(0, 0), Shared = true },
	{ Name = "float_position_array", Type = PropertyType.FloatPosition, Array = true, Default = { Vec2(0, 0) }, Shared = true },
	{ Name = "float_position3d", Type = PropertyType.FloatPosition3D, Default = Vec3(0, 0, 0), Shared = true },
	{ Name = "float_position3d_array", Type = PropertyType.FloatPosition3D, Array = true, Default = { Vec3(0, 0, 0) }, Shared = true },
	{ Name = "float_rect", Type = PropertyType.FloatRect, Default = Rect(Vec2(0, 0), Vec2(1, 1)), Shared = true },
	{ Name = "float_rect_array", Type = PropertyType.FloatRect, Array = true, Default = { Rect(Vec2(0, 0), Vec2(1, 1)) }, Shared = true },
	{ Name = "float_size", Type = PropertyType.FloatSize, Default = Vec2(0, 0), Shared = true },
	{ Name = "float_size_array", Type = PropertyType.FloatSize, Array = true, Default = { Vec2(0, 0) }, Shared = true },
	{ Name = "float_size3d", Type = PropertyType.FloatSize3D, Default = Vec3(0, 0, 0), Shared = true },
	{ Name = "float_size3d_array", Type = PropertyType.FloatSize3D, Array = true, Default = { Vec3(0, 0, 0) }, Shared = true },
	{ Name = "int", Type = PropertyType.Integer, Default = 0, Shared = true },
	{ Name = "int_array", Type = PropertyType.Integer, Array = true, Default = { 0 }, Shared = true },
	{ Name = "int_position", Type = PropertyType.IntegerPosition, Default = Vec2(0, 0), Shared = true },
	{ Name = "int_position_array", Type = PropertyType.IntegerPosition, Array = true, Default = { Vec2(0, 0) }, Shared = true },
	{ Name = "int_position3d", Type = PropertyType.IntegerPosition3D, Default = Vec3(0, 0, 0), Shared = true },
	{ Name = "int_position3d_array", Type = PropertyType.IntegerPosition3D, Array = true, Default = { Vec3(0, 0, 0) }, Shared = true },
	{ Name = "int_rect", Type = PropertyType.IntegerRect, Default = Rect(Vec2(0, 0), Vec2(1, 1)), Shared = true },
	{ Name = "int_rect_array", Type = PropertyType.IntegerRect, Array = true, Default = { Rect(Vec2(0, 0), Vec2(1, 1)) }, Shared = true },
	{ Name = "int_size", Type = PropertyType.IntegerSize, Default = Vec2(0, 0), Shared = true },
	{ Name = "int_size_array", Type = PropertyType.IntegerSize, Array = true, Default = { Vec2(0, 0) }, Shared = true },
	{ Name = "int_size3d", Type = PropertyType.IntegerSize3D, Default = Vec3(0, 0, 0), Shared = true },
	{ Name = "int_size3d_array", Type = PropertyType.IntegerSize3D, Array = true, Default = { Vec3(0, 0, 0) }, Shared = true },
	{ Name = "layer", Type = PropertyType.Layer, Default = NoLayer, Shared = true },
	{ Name = "layer_array", Type = PropertyType.Layer, Array = true, Default = { NoLayer }, Shared = true },
	{ Name = "str", Type = PropertyType.String, Default = "", Shared = true },
	{ Name = "str_array", Type = PropertyType.String, Array = true, Default = { "" }, Shared = true },
	{ Name = "texture", Type = PropertyType.Texture, Default = "", Shared = true },
	{ Name = "texture_array", Type = PropertyType.Texture, Array = true, Default = { "" }, Shared = true },
}
