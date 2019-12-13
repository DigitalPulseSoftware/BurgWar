RegisterClientScript()

WEAPON.Cooldown = 0.5
WEAPON.Scale = 0.75
WEAPON.Sprite = "placeholder/baguette.png"
WEAPON.SpriteOrigin = Vec2(40, 20) * WEAPON.Scale
WEAPON.WeaponOffset = Vec2(20, -60) -- This should not be here
WEAPON.Animations = {
	{ "grapple", 2 }
}

RegisterClientAssets(WEAPON.Sprite)

local maxDist = 1000

function WEAPON:OnAttack()
	if (self.GrapplePull or self.GrappleSprite) then
		self:Release()
	else
		self:Launch()
	end
end

function WEAPON:Launch()
	local startOffset = Vec2(85 * self.Scale, 0)

	local startPos = self:ToGlobalPosition(startOffset)
	local endPos = startPos + self:GetDirection() * maxDist

	local nearestFraction = math.huge
	local nearestResult

	local traceResult = physics.TraceMultiple(0, startPos, endPos, function (result)
		if (result.fraction < nearestFraction) then
			if (result.hitEntity) then
				-- Ignore player
				if (result.hitEntity == self:GetOwnerEntity()) then
					return
				end
			end

			nearestResult = result
			nearestFraction = result.fraction
		end
	end)

	local targetEntity
	local targetOffset
	local duration = 0.5
	local hitPos

	if (nearestResult) then
		targetEntity = nearestResult.hitEntity
		targetOffset = targetEntity:ToLocalPosition(nearestResult.hitPos)
		duration = duration * nearestResult.fraction
		hitPos = nearestResult.hitPos
	else
		hitPos = endPos
		targetOffset = endPos
	end

	local direction = self:GetDirection()

	if (CLIENT) then
		local grappleSprite = match.CreateEntity({
			Type = "entity_grapple_sprite",
			LayerIndex = self:GetLayerIndex(),
			Properties = {
				duration = duration
			}
		})

		grappleSprite.SourceEntity = self
		grappleSprite.SourceOffset = startOffset
		grappleSprite.TargetEntity = targetEntity
		grappleSprite.TargetOffset = targetOffset

		grappleSprite:GrappleInit() -- FIXME: Waiting for entities in properties to fix this

		self.GrappleSprite = grappleSprite
	end

	timer.Sleep(math.floor(duration * 1000))

	if (targetEntity and targetEntity:IsValid()) then
		--FIXME: Create on both client and server for now (because entities cannot be shared as properties yet)
		local grapplePull = match.CreateEntity({
			Type = "entity_grapple_pull",
			LayerIndex = self:GetLayerIndex(),
			Properties = {
				duration = duration
			}
		})

		grapplePull.SourceEntity = self:GetOwnerEntity()
		grapplePull.SourceOffset = startOffset

		grapplePull.TargetEntity = targetEntity
		grapplePull.TargetOffset = targetOffset

		grapplePull:GrappleInit() -- FIXME: Waiting for entities in properties to fix this

		self.GrapplePull = grapplePull
	else
		self:Release()
	end
end

function WEAPON:Release()
	if (self.GrapplePull and self.GrapplePull:IsValid()) then
		self.GrapplePull:Remove()
	end
	self.GrapplePull = nil

	if (self.GrappleSprite and self.GrappleSprite:IsValid()) then
		self.GrappleSprite:Retract()
	end
	self.GrappleSprite = nil
end

if (CLIENT) then
	function WEAPON:OnAnimationStart(animationId)
	end
end
