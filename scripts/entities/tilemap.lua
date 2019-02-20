RegisterClientScript("tilemap.lua")

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 1000

ENTITY.Properties = {
	{ Name = "width", Type = PropertyType.Integer, Shared = true },
	{ Name = "height", Type = PropertyType.Integer, Shared = true },
	{ Name = "cellWidth", Type = PropertyType.Float, Shared = true },
	{ Name = "cellHeight", Type = PropertyType.Float, Shared = true },
	{ Name = "content", Type = PropertyType.Integer, Array = true, Shared = true },
}
