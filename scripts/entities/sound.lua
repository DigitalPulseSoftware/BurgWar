RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "soundPath", Type = PropertyType.String, Shared = true }, -- TODO: Improve using networked string
		{ Name = "isLooping", Type = PropertyType.Boolean, Default = false, Shared = true },
		{ Name = "isSpatialized", Type = PropertyType.Boolean, Default = true, Shared = true },
	}
})

if (CLIENT) then
    entity:OnAsync("Init", function (self)
		self.Sound = self:PlaySound(self:GetProperty("soundPath"), true, self:GetProperty("isLooping"), self:GetProperty("isSpatialized"))
	end)

	entity:On("Destroyed", function (self)
		self.Sound:Stop()
	end)
end
