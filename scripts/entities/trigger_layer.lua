ENTITY.IsNetworked = false

ENTITY.Properties = {
	{ Name = "size", Type = PropertyType.FloatSize, Default = Vec2(1.0, 1.0) },
	{ Name = "target_layer", Type = PropertyType.Integer, Default = 0 },
}

function ENTITY:Initialize()
	assert(self:GetProperty("target_layer") >= 0)

	local size = self:GetProperty("size")
	local colliderSize = size / 2
	self:SetCollider(Rect(-colliderSize, colliderSize))
	self:EnableCollisionCallbacks(true)

	if (EDITOR) then
		self:AddSprite({
			Color = { r = 0, g = 0, b = 0, a = 180 },
			RenderOrder = 2000,
			Size = self:GetProperty("size"),
		})
	end
end

function ENTITY:OnCollisionStart(other)
	if (other.Name == "burger") then
		local owner = other:GetOwner()
		owner:UpdateLayer(self:GetProperty("target_layer"))
	end

	return false
end

