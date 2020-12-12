RegisterClientScript()


local entity = ScriptedEntity({
	Base = "entity_sprite",
	IsNetworked = true,
	Properties = {
		{ Name = "lifetime", Type = PropertyType.Integer, Default = 10, Shared = true },
		{ Name = "disappeartime", Type = PropertyType.Integer, Default = 2, Shared = true },
	}
})

entity:On("init", function (self)
	local colliders = self:GetColliders()
	for _, collider in pairs(colliders) do
		collider.ColliderType = ColliderType.Callback
	end
	self:SetColliders(colliders)
	
	self.DisappearTime = match.GetSeconds() + self:GetProperty("lifetime")
end)

entity:On("collisionstart", function (self, entity)
	return entity:GetMass() == 0
end)

entity:On("tick", function (self)
	local now = match.GetSeconds()
	if (now >= self.DisappearTime) then
		local alphaFactor = 1 - (now - self.DisappearTime) / self:GetProperty("disappeartime")
		if (alphaFactor > 0) then
			self.Sprite:SetColor({ r = 255, g = 255, b = 255, a = math.floor(alphaFactor * 255) })
		else
			self:Remove()
		end
	end
end)