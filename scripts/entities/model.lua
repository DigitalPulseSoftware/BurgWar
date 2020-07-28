RegisterClientScript()

entity.IsNetworked = true
entity.CollisionType = 2
entity.PlayerControlled = false
entity.MaxHealth = 0

entity.Properties = {
	{ Name = "modelPath", Type = PropertyType.String, Default = "", Shared = true },
	{ Name = "offset", Type = PropertyType.FloatPosition3D, Default = Vec3(0, 0, 0), Shared = true },
	{ Name = "renderOrder", Type = PropertyType.Integer, Default = 0, Shared = true },
	{ Name = "rotation", Type = PropertyType.FloatPosition3D, Default = Vec3(0, 0, 0), Shared = true }, --TODO: EulerAngles
	{ Name = "scale", Type = PropertyType.FloatSize3D, Default = Vec3(1, 1, 1), Shared = true },
}

if (CLIENT) then
	function entity:Initialize()
		self:AddModel({
			ModelPath = self:GetProperty("modelPath"),
			Offset = self:GetProperty("offset"),
			RenderOrder = self:GetProperty("renderOrder"),
			Rotation = self:GetProperty("rotation"),
			Scale = self:GetProperty("scale"),
		})
	end
end
