RegisterClientScript("tilemap.lua")

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 1000

ENTITY.Properties = {
	{ Name = "width", Type = PropertyType.Integer, Default = 1, Shared = true },
	{ Name = "height", Type = PropertyType.Integer, Default = 1, Shared = true },
	{ Name = "cellWidth", Type = PropertyType.Float, Default = 1, Shared = true },
	{ Name = "cellHeight", Type = PropertyType.Float, Default = 1, Shared = true },
	--{ Name = "content", Type = PropertyType.Integer, Array = true, Shared = true },
}
