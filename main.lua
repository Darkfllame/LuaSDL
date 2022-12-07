function math.clamp(val, min, max)
    if val < min then return min
    elseif val > max then return max end

    return val
end

local width, height = 800, 600

LuaSDL.Start(nil, width, height)
local bgCol = Color.new(0, 0, 0)
LuaSDL.Background.SetColor(bgCol)

local scale = 100
local rotation = 0
local x, y = 0,0

function LuaSDL.ResetWH()
    width, height = LuaSDL.Window.GetSize()
end
function LuaSDL.DrawCircle(x, y, r, thickness)
    x = x or 0
    y = y or 0
    r = r or 10
    thickness = thickness or 1

    LuaSDL.ResetWH()
    for xw=x-r,x+r,1 do
        for yw=y-r,y+r,1 do
            local dist = math.sqrt((xw-x)^2+(yw-y)^2) 
            if dist < r + thickness/2 and dist > r - thickness/2 then
                LuaSDL.Drawing.DrawPixel(xw,yw)
            end
        end
    end
end
function LuaSDL.FillCircle(x, y, r)
    x = x or 0
    y = y or 0
    r = r or 10

    LuaSDL.ResetWH()
    for xw=x-r,x+r,1 do
        for yw=y-r,y+r,1 do
            local dist = math.sqrt((xw-x)^2+(yw-y)^2) 
            if dist < r then
                LuaSDL.Drawing.DrawPixel(xw,yw)
            end
        end
    end
end
function LuaSDL.Lerp(a,b,t)
    return a + (b-a) * t
end
function LuaSDL.DrawLine(x,y,x2,y2)
    ResetWH()
    local dist = math.abs(math.sqrt((x2-x)^2+(y2-y)^2))

    for t=0,1,1/dist do
        local xl = Lerp(x,x2, t)
        local yl = Lerp(y,y2, t)
        LuaSDL.Drawing.DrawPixel(xl,yl)
    end
end

function update(dt)
    
end

function render()
    
end