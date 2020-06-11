ENTITY.IsMoving = false
ENTITY.IsHopping = false

local oldInit = ENTITY.Initialize
function ENTITY:Initialize()
	oldInit(self)

	if (EDITOR) then
		self:AddSprite({
			RenderOrder = 0,
			Scale = Vec2(0.33, 0.33),
			TexturePath = "burger2.png"
		})
		return
	end

	local cursor = 0
	local maxWidth = 0
	local defaultOrigin = Vec2(0.5, 1)
	local renderOrder = 0
	local faceOffset
	self.Sprites = {}
	self:ForEachElement(function (elementData, randomSprite)
		local elementOffsetScale = elementData.OffsetScale or 1
		local elementOrigin = elementData.Origin or defaultOrigin

		local faceOrigin, origin, offsetScale, texturePath
		if (type(randomSprite) == "table") then
			faceOrigin = randomSprite.FaceOrigin or elementData.FaceOrigin
			origin = randomSprite.Origin or elementOrigin
			offsetScale = randomSprite.OffsetScale or elementOffsetScale
			texturePath = randomSprite.Path
		else
			assert(type(randomSprite) == "string")

			faceOrigin = elementData.FaceOrigin
			origin = defaultOrigin
			offsetScale = elementOffsetScale
			texturePath = randomSprite
		end

		local offset = Vec2(0, cursor)

		local sprite = self:AddSprite({
			Offset = offset,
			Origin = origin,
			RenderOrder = renderOrder,
			Scale = Vec2(self.Scale, self.Scale),
			TexturePath = texturePath
		})

		local spriteSize = sprite:GetSize()

		if (faceOrigin) then
			faceOffset = offset - origin * spriteSize + faceOrigin * spriteSize
		end

		maxWidth = math.max(maxWidth, spriteSize.x)
		cursor = cursor - spriteSize.y * offsetScale
		renderOrder = renderOrder + 1

		table.insert(self.Sprites, {
			Offset = offset,
			Origin = origin,
			Size = spriteSize,
			TexturePath = texturePath
		})
	end)

	for name, texture in pairs(self.Faces) do
		local face = self:AddSprite({
			Offset = faceOffset,
			RenderOrder = renderOrder,
			Scale = Vec2(self.Scale, self.Scale),
			TexturePath = texture
		})
		face:Hide()

		self[name .. "Face"] = face
	end

	self.CurrentFace = self.DefaultFace
	self.DefaultFace:Show()
end

function ENTITY:UpdateFace(face, duration)
	if (self.CurrentFace ~= face) then
		self.CurrentFace:Hide()
		self.CurrentFace = face

		face:Show()
	end

	if (duration) then
		self.FaceExpiration = match.GetSeconds() + duration
	else
		self.FaceExpiration = nil
	end
end

function ENTITY:OnTick()
	if (self.FaceExpiration) then
		if (match.GetSeconds() >= self.FaceExpiration) then
			self.CurrentFace:Hide()
			self.CurrentFace = self.DefaultFace
			self.CurrentFace:Show()

			self.FaceExpiration = nil
		end
	end
end

function ENTITY:OnHealthUpdate(oldHealth, newHealth)
	if (newHealth > 0) then
		if (newHealth > oldHealth) then
			-- Heal
			self:UpdateFace(self.VictoryFace, 2)
		elseif (newHealth < oldHealth) then
			-- Damage
			self:UpdateFace(self.DamageFace, 2)
		end
	else
		self:OnDeath()
	end
end

function ENTITY:OnDeath()
	local layerIndex = self:GetLayerIndex()
	local pos = self:GetPosition()
	local vel = self:GetVelocity()
	local force = vel:Normalize()

	for _, sprite in pairs(self.Sprites) do
		local entity = match.CreateEntity({
			Type = "entity_gibs",
			Position = pos + sprite.Offset - sprite.Origin,
			LayerIndex = layerIndex,
			Properties = {
				mass = 5,
				physical = true,
				size = sprite.Size,
				texture = sprite.TexturePath
			}
		})

		local randVel = vel * (math.random() + 0.5) + Vec2(math.random(), math.random()) * (force + 500)
		entity:SetAngularVelocity(math.random(-180, 180) * math.random() * 10)
		entity:SetVelocity(randVel)
	end
end

function ENTITY:OnInputUpdate(input)
	if (input.isAttacking) then
		if (self.CurrentFace == self.DamageFace) then
			self:UpdateFace(self.RampageFace, 1)
		elseif (self.CurrentFace ~= self.RampageFace) then
			self:UpdateFace(self.AttackFace, 0.5)
		end
	end

	local isMoving = input.isMovingLeft or input.isMovingRight
	if (self.IsMoving ~= isMoving) then
		self.IsMoving = isMoving
		if (isMoving and not input.isJumping) then
			if (not self.IsHopping) then
				self.IsHopping = true
				while (self.IsMoving) do
					if (self:IsValid() and self:IsPlayerOnGround()) then
						animation.PositionByOffset(self, Vec2(0, 0), Vec2(0, -15), 0.15)
						animation.PositionByOffset(self, Vec2(0, -15), Vec2(0, 0), 0.15)
					else
						timer.Sleep(30)
					end
				end
				self.IsHopping = false
			end
		end
	end
end
