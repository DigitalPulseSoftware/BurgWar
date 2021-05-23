RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "scriptPath", Type = PropertyType.String, Shared = true },
	}
})

if (not EDITOR) then
	entity:On("init", function (self)
		include(self:GetProperty("scriptPath"))
	end)
end
