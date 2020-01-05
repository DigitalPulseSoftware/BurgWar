RegisterClientScript()

ENTITY.IsNetworked = true
ENTITY.CollisionType = 2
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 0

ENTITY.Properties = {
	{ Name = "modelPath", Type = PropertyType.String, Default = "", Shared = true },
	{ Name = "offset", Type = PropertyType.FloatPosition3D, Default = Vec3(0, 0, 0), Shared = true },
	{ Name = "renderOrder", Type = PropertyType.Integer, Default = 0, Shared = true },
	{ Name = "rotation", Type = PropertyType.FloatPosition3D, Default = Vec3(0, 0, 0), Shared = true }, --TODO: EulerAngles
	{ Name = "scale", Type = PropertyType.FloatSize3D, Default = Vec3(1, 1, 1), Shared = true },
}

if (CLIENT) then
	function ENTITY:Initialize()
		self:AddModel({
			ModelPath = self:GetProperty("modelPath"),
			Offset = self:GetProperty("offset"),
			RenderOrder = self:GetProperty("renderOrder"),
			Rotation = self:GetProperty("rotation"),
			Scale = self:GetProperty("scale"),
		})
	end
end
