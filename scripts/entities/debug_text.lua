RegisterClientScript()

local entity = ScriptedEntity({
    IsNetworked = true,
    Properties = {
        {Name = "text", Type = PropertyType.string , shared = true},
        {Name = "Lifetime", Type = PropertyType.Float , shared = true}
    }
})

if (not EDITOR) then
    entity:On("init", function(self)
        local lifetime = self:GetProperty("Lifetime")
        if (lifetime > 0) then
        self:SetLifeTime(lifetime)
       end
    end)
end


if (CLIENT) then
    entity:On("init", function(self)
        local text = self:GetProperty("text")
        local color = {r=85 , g=15 , b= 15, a = 100 }
        local textWidth = 5
    )
