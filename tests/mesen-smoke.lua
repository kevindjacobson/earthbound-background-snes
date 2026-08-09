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
        a = frame >= 75 and frame < 78,
        r = frame >= 70 and frame < 73,
        select = frame >= 55 and frame < 58,
        start = (frame >= 30 and frame < 34) or (frame >= 100 and frame < 104),
    }
    emu.setInput(input, 0)
end, emu.eventType.inputPolled)

emu.addEventCallback(function()
    frame = frame + 1
    if frame == 50 then
        debug_color_count = color_count(emu.getScreenBuffer())
    end
    if frame == 150 then
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
