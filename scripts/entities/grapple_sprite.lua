RegisterClientScript()

ENTITY.IsNetworked = false
ENTITY.CollisionType = 2
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 0

ENTITY.Properties = {
	{ Name = "duration", Type = PropertyType.Float, Default = 0 },
}

function ENTITY:Initialize()
	self.isRetracting = false
	self.startTime = match.GetCurrentTime()
end

--FIXME: This is required because entities cannot be used as properties yet
function ENTITY:GrappleInit()
	if (not self.TargetEntity) then
		self.endPos = self.TargetOffset
	end

	self:UpdatePositions()

	self.sprite = self:AddSprite({
		Color = { r = 255, g = 255, b = 0, a = 220 },
		Size = Vec2(0, 0)
	})

	self:UpdateSprite()
end

function ENTITY:UpdatePositions()
	if (self.SourceEntity:IsValid()) then
		self.startPos = self.SourceEntity:ToGlobalPosition(self.SourceOffset)
	else
		self:Remove()
		return
	end

	if (not self.isRetracting and self.TargetEntity) then
		if (self.TargetEntity:IsValid()) then
			self.endPos = self.TargetEntity:ToGlobalPosition(self.TargetOffset)
		else
			self:Retract()
		end
	end
end

function ENTITY:UpdateSprite()
	local direction, length = (self.endPos - self.startPos):Normalize()
	local rotation = math.atan(direction.y, direction.x) * 180 / math.pi

	local elapsedTime = match.GetCurrentTime() - self.startTime
	if (self.isRetracting) then
		length = length * (1 - elapsedTime / self.retractTime)
		if (length < 0) then
			self:Remove()
			return
		end
	else
		length = math.min(length, length * elapsedTime / self:GetProperty("duration"))
	end

	self:SetPosition(self.startPos)
	self.sprite:SetRotation(rotation)
	self.sprite:SetSize(Vec2(length, 4))
end

function ENTITY:Retract()
	assert(not self.isRetracting)

	local elapsedTime = match.GetCurrentTime() - self.startTime
	if (elapsedTime > 0) then 
		self.retractTime = math.min(elapsedTime, self:GetProperty("duration"))

		self.isRetracting = true
		self.startTime = match.GetCurrentTime()
	else
		self:Remove()
	end
end

if (CLIENT) then
	function ENTITY:OnFrame()
		self:UpdatePositions()
		self:UpdateSprite()
	end
end
