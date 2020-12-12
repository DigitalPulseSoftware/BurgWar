local entity = ScriptedEntity({
	Properties = {
		{ Name = "size", Type = PropertyType.FloatSize },
	}
})

entity.Passthrough = true

entity:On("init", function (self)
	local size = self:GetProperty("size")
	local colliderSize = size / 2
	self:SetColliders({
		Collider = Rect(-colliderSize, colliderSize),
		ColliderType = ColliderType.Callback
	})

	if (EDITOR) then
		self:AddSprite({
			Color = { r = 0, g = 0, b = 0, a = 180 },
			RenderOrder = 2000,
			Size = self:GetProperty("size"),
		})
	end
end)

entity:On("collisionstart", function (self, other)
	other:Damage(other:GetHealth())
	return true
end)
