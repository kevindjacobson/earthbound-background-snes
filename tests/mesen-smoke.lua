local frame = 0
local debug_color_count = 0

local function color_count(pixels)
    local colors = {}
    local count = 0
    for index = 1, #pixels do
        local color = pixels[index]
        if not colors[color] then
            colors[color] = true
            count = count + 1
        end
    end
    return count
end

emu.addEventCallback(function()
    local input = {
        a = frame >= 95 and frame < 99,
        r = frame >= 85 and frame < 89,
        select = frame >= 75 and frame < 79,
        start = (frame >= 50 and frame < 54) or (frame >= 110 and frame < 114),
    }
    emu.setInput(input, 0)
end, emu.eventType.inputPolled)

emu.addEventCallback(function()
    frame = frame + 1
    if frame == 70 then
        debug_color_count = color_count(emu.getScreenBuffer())
    end
    if frame == 200 then
        local png = emu.takeScreenshot()
        local pixels = emu.getScreenBuffer()
        local background_color_count = color_count(pixels)
        if debug_color_count <= 1 then emu.stop(3)
        elseif debug_color_count > 16 then emu.stop(4)
        elseif background_color_count <= 16 then emu.stop(5)
        elseif #png <= 100 or string.sub(png, 2, 4) ~= "PNG" then emu.stop(6)
        else emu.stop(0) end
    end
end, emu.eventType.endFrame)
