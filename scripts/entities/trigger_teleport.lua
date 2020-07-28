entity.IsNetworked = false

entity.Properties = {
	{ Name = "size", Type = PropertyType.FloatSize },
	{ Name = "target", Type = PropertyType.Entity },
}

function entity:Initialize()
	local size = self:GetProperty("size")
	local colliderSize = size / 2
	self:SetCollider(Rect(-colliderSize, colliderSize), true)
	self:EnableCollisionCallbacks(true)

	if (EDITOR) then
		self:AddSprite({
			Color = { r = 0, g = 0, b = 0, a = 180 },
			RenderOrder = 2000,
			Size = self:GetProperty("size"),
		})
	end
end

function entity:OnCollisionStart(other)
	local targetEntity = self:GetProperty("target")
	if (targetEntity == NoEntity) then
		return false
	end

	local targetLayer = targetEntity:GetLayerIndex()
	local targetPosition = targetEntity:GetPosition()

	if (other.Name == "burger") then
		local playerOwner = other:GetOwner()
		other:SetPosition(targetPosition)
		playerOwner:MoveToLayer(targetLayer)
	end

	return false
end

