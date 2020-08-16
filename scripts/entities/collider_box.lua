RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "friction", Type = PropertyType.Float, Default = 1, Shared = true },
		{ Name = "mass", Type = PropertyType.Float, Default = 0, Shared = true },
		{ Name = "size", Type = PropertyType.FloatSize, Shared = true },
	}
})

entity:On("init", function (self)
	local size = self:GetProperty("size")
	local colliderSize = size / 2
	self:SetCollider(Rect(-colliderSize, colliderSize))
	self:InitRigidBody(self:GetProperty("mass"), self:GetProperty("friction"))

	if (EDITOR) then
		self:AddSprite({
			Color = { r = 200, g = 0, b = 0, a = 180 },
			RenderOrder = 2000,
			Size = self:GetProperty("size"),
		})
	end
end)
