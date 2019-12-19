ENTITY.IsMoving = false
ENTITY.IsHopping = false

local oldInit = ENTITY.Initialize
function ENTITY:Initialize()
	oldInit(self)

	local mainSprite = self:AddSprite({
		Origin = Vec2(0.5, 1),
		Scale = Vec2(self.Scale, self.Scale),
		TexturePath = self.Sprite
	})

	local faceOrigin = mainSprite:GetSize() * Vec2(0.65, 0.31) - mainSprite:GetOrigin() * mainSprite:GetSize()

	for name, texture in pairs(self.Faces) do
		local face = self:AddSprite({
			Offset = faceOrigin,
			RenderOrder = 1,
			Scale = Vec2(self.Scale, self.Scale),
			TexturePath = texture
		})
		face:Hide()

		self[name .. "Face"] = face
	end

	if (self.Hat) then
		self:AddSprite({
			Offset = mainSprite:GetSize() * Vec2(0.4512, 0.2272) - mainSprite:GetOrigin() * mainSprite:GetSize(),
			Origin = Vec2(0.75, 0.95),
			RenderOrder = 2,
			Rotation = -15,
			Scale = Vec2(self.Scale, self.Scale) * 0.8,
			TexturePath = self.Hat
		})
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
	if (newHealth > oldHealth) then
		-- Heal
		self:UpdateFace(self.VictoryFace, 2)
	elseif (newHealth < oldHealth) then
		-- Damage
		self:UpdateFace(self.DamageFace, 2)
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
