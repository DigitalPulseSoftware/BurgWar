return {
	Name = "base",
	Description = "Base gamemode",
	Events = {
		{ Name = "PlayerTeamUpdate" }
	},
	Properties = {
		{ Name = "respawntime", Type = PropertyType.Integer, Default = 5, Shared = true },
	}
}
