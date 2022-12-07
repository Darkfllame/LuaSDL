width, height = 800, 600

LuaSDL.StartSDL("Exemple", width, height)

img = LuaSDL.newImage("thing.png")

function LuaSDL.ResetWH()
	width, height = LuaSDL.GetWindowDim()
end
function LuaSDL.DrawCircle(x, y, r, thickness)
    x = x or 0
    y = y or 0
    r = r or 10
    thickness = thickness or 1

    local width, height = LuaSDL.GetWindowDim()
    for xw=0,width,1 do
        for yw=0,height,1 do
            local dist = math.sqrt((xw-x)^2+(yw-y)^2) 
	        if dist < r + thickness/2 and dist > r - thickness/2 then
                LuaSDL.DrawPixel(xw,yw)
            end
        end
    end
end
function LuaSDL.FillCircle(x, y, r)
    x = x or 0
    y = y or 0
    r = r or 10

    local width, height = LuaSDL.GetWindowDim()
    for xw=0,width,1 do
        for yw=0,height,1 do
            local dist = math.sqrt((xw-x)^2+(yw-y)^2) 
	        if dist < r then
                LuaSDL.DrawPixel(xw,yw)
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
        LuaSDL.DrawPixel(xl,yl)
    end
end

function update(dt)
end

function render()
    local col = LuaSDL.newColor(255, 255, 255, 255)
    LuaSDL.SetDrawColor(col)
    
    LuaSDL.ResetWH()
    LuaSDL.DrawImage(img, 0,0)
end