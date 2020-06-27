RegisterClientScript()

ENTITY.Base = "entity_sprite"

ENTITY.Properties = {
	{ Name = "lifetime", Type = PropertyType.Integer, Default = 10, Shared = true },
	{ Name = "disappeartime", Type = PropertyType.Integer, Default = 2, Shared = true },
}

function ENTITY:Initialize()
	self.Base.Initialize(self)

	self:EnableCollisionCallbacks(true)
	self.DisappearTime = match.GetSeconds() + self:GetProperty("lifetime")
end

function ENTITY:OnCollisionStart(entity)
	return entity:GetMass() == 0
end

function ENTITY:OnTick()
	local now = match.GetSeconds()
	if (now >= self.DisappearTime) then
		local alphaFactor = 1 - (now - self.DisappearTime) / self:GetProperty("disappeartime")
		if (alphaFactor > 0) then
			self.Sprite:SetColor({ r = 255, g = 255, b = 255, a = math.floor(alphaFactor * 255) })
		else
			self:Remove()
		end
	end
end