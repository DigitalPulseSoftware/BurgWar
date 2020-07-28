RegisterClientScript()

entity.IsNetworked = true
entity.CollisionType = 2
entity.PlayerControlled = false
entity.MaxHealth = 0

entity.Properties = {
	{ Name = "duration", Type = PropertyType.Float, Shared = true },
	{ Name = "retracting", Type = PropertyType.Boolean, Default = false, Shared = true },
	{ Name = "source_entity", Type = PropertyType.Entity, Shared = true },
	{ Name = "source_offset", Type = PropertyType.FloatPosition, Shared = true },
	{ Name = "target_entity", Type = PropertyType.Entity, Shared = true },
	{ Name = "target_offset", Type = PropertyType.FloatPosition, Shared = true },
}

function entity:Initialize()
	self.SourceEntity = self:GetProperty("source_entity")
	self.SourceOffset = self:GetProperty("source_offset")
	self.TargetEntity = self:GetProperty("target_entity")
	self.TargetOffset = self:GetProperty("target_offset")

	self.isRetracting = false
	self.startTime = match.GetSeconds()

	if (self.SourceEntity == NoEntity) then
		self:Remove()
		return
	end

	if (self.TargetEntity ~= NoEntity) then
		self.endPos = self.TargetEntity:ToGlobalPosition(self.TargetOffset)
	else
		self.endPos = self.TargetOffset
	end

	if (self:GetProperty("retracting")) then
		self.startTime = self.startTime - self:GetProperty("duration")
		self:Retract()
	end

	self:UpdatePositions()

	if (CLIENT) then
		self.sprite = self:AddSprite({
			Color = { r = 255, g = 255, b = 0, a = 220 },
			RenderOrder = -5,
			Size = Vec2(0, 0)
		})

		self:UpdateSprite()
	end
end

function entity:UpdatePositions()
	if (self.SourceEntity:IsValid()) then
		self.startPos = self.SourceEntity:ToGlobalPosition(self.SourceOffset)
	else
		self:Remove()
		return
	end

	if (not self.isRetracting and self.TargetEntity ~= NoEntity) then
		if (self.TargetEntity:IsValid()) then
			self.endPos = self.TargetEntity:ToGlobalPosition(self.TargetOffset)
		else
			self:Retract()
		end
	end
end

function entity:Retract()
	assert(not self.isRetracting)

	if (SERVER) then
		self:Remove()
	else
		local elapsedTime = match.GetSeconds() - self.startTime
		if (elapsedTime > 0) then 
			self.retractTime = math.min(elapsedTime, self:GetProperty("duration"))

			self.isRetracting = true
			self.startTime = match.GetSeconds()
		else
			self:Remove()
		end
	end
end

if (SERVER) then
	function entity:OnTick()
		if (self.isRetracting) then
			local elapsedTime = match.GetSeconds() - self.startTime
			if (elapsedTime > self.retractTime) then
				self:Remove()
				return
			end
		end
	end
else
	function entity:OnKilled()
		--TODO: Use RPC to prevent creating a second entity
		if (not self:GetProperty("retracting")) then
			local elapsedTime = match.GetSeconds() - self.startTime

			match.CreateEntity({
				Type = self.FullName,
				LayerIndex = self:GetLayerIndex(),
				Properties = {
					duration = math.min(elapsedTime, self:GetProperty("duration")),
					retracting = true,
					source_entity = self.SourceEntity,
					source_offset = self.SourceOffset,
					target_entity = self.TargetEntity,
					target_offset = self.TargetOffset
				}
			})
		end
	end

	function entity:UpdateSprite()
		local direction, length = (self.endPos - self.startPos):GetNormalized()
		local rotation = math.atan(direction.y, direction.x) * 180 / math.pi

		local elapsedTime = match.GetSeconds() - self.startTime
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

	function entity:OnFrame()
		self:UpdatePositions()
		self:UpdateSprite()
	end
end
