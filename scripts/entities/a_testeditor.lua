RegisterClientScript("a_testeditor.lua")

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 1000

ENTITY.Properties = {
	{ Name = "bool", Type = PropertyType.Boolean, Array = true, Default = { false }, Shared = true },
	{ Name = "float", Type = PropertyType.Float, Array = true, Default = { 0 }, Shared = true },
	{ Name = "int", Type = PropertyType.Integer, Array = true, Default = { 0 }, Shared = true },
	{ Name = "str", Type = PropertyType.String, Array = true, Default = { "" }, Shared = true },
}
