return {
	Name = "supersmashburger",
	Base = "deathmatch",
	Description = "Super Smash Burger, be the last burger alive",
	Events = {
		{ Name = "RoundStateUpdate" }
	},
	Properties = {
		{ Name = "maxplayercount", Type = PropertyType.Integer, Default = 4 },
		{ Name = "minplayercount", Type = PropertyType.Integer, Default = 2 },
	}
}
