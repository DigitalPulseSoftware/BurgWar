RegisterClientScript()

local entity = ScriptedEntity({
	Base = "entity_text",
	IsNetworked = true,
	Properties = {
		{ Name = "lifetime", Type = PropertyType.Integer, Default = 1, Shared = true },
		{ Name = "value", Type = PropertyType.Integer, Shared = true },
	}
})

entity:On("Init", function (self)
	local healthDelta = self:GetProperty("value")
	self.Text:SetText(tostring(math.abs(healthDelta)))
	if (healthDelta > 0) then
		self.Color = { r = 0, g = 255, b = 0, a = 255 }
	else
		self.Color = { r = 255, g = 0, b = 0, a = 255 }
	end
	self.Text:SetColor(self.Color)

	self.DisappearTime = match.GetSeconds()
	self.FromPos = self:GetPosition()
	self.ToPos = self:GetPosition() - Vec2(0, 200)
end)

-- TODO: Switch to frame event
entity:On("Tick", function (self)
	local now = match.GetSeconds()
	local lifeFactor = (now - self.DisappearTime) / self:GetProperty("lifetime")

	self:SetPosition(math.lerp(self.FromPos, self.ToPos, lifeFactor))
	local alphaFactor = 1 - lifeFactor
	if (alphaFactor > 0) then
		self.Color.a = math.floor(alphaFactor * 255)
		self.Text:SetColor(self.Color)
	else
		self:Remove()
	end
end)
