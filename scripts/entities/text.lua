RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "renderOrder", Type = PropertyType.Integer, Default = 0, Shared = true },
		{ Name = "text", Type = PropertyType.String, Default = "", Shared = true },
	}
})

if (CLIENT) then
    entity:On("init", function (self)
		self.Text = self:AddText({
			RenderOrder = self:GetProperty("renderOrder"),
			Text = self:GetProperty("text")
		})
	end)
end
