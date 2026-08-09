local frame = 0

emu.addEventCallback(function()
    frame = frame + 1
    if frame == 200 then
        -- EbState occupies $7e2000-$7e201b; the counter follows it deliberately.
        if emu.read16(0x7e201c, emu.memType.snesMemory) < 40 then emu.stop(3)
        else emu.stop(0) end
    end
end, emu.eventType.endFrame)
