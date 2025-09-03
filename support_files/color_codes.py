# print a list of the 256-color red/green/blue values used by xterm.
#
# modified from https://en.wikipedia.org/wiki/ANSI_escape_code
#
# reference:
# https://github.com/ThomasDickey/ncurses-snapshots/blob/master/test/xterm-16color.dat
# https://github.com/ThomasDickey/xterm-snapshots/blob/master/XTerm-col.ad
# https://github.com/ThomasDickey/xterm-snapshots/blob/master/256colres.pl

def rgb565(r, g, b):
    return ((r & 0x00F8) << 8) | ((g & 0x00fc) << 3) | ((b >> 3) & 0x001F)  ;
    #return (((g&0b00011100)<<3)+((r&0b11111000)>>3)<<8)+(b&0b11111000)+((g&0b11100000)>>5)
    #return ((((r&0b00011100)<<3)+((g&0b11111000)>>3))<<8)+(b&0b11111000)+((r&0b11100000)>>5)
    #return ((((r&0b00011100)<<3)+((g&0b11111000)>>3))<<8)+(b&0b11111000)+((r&0b11100000)>>5)

print("colors 0-16 correspond to the ANSI and aixterm naming")
for code in range(0, 16):
    if code > 8:
        level = 255
    elif code == 7:
        level = 229
    else:
        level = 205
    r = 127 if code == 8 else level if (code & 1) != 0 else 92 if code == 12 else 0
    g = 127 if code == 8 else level if (code & 2) != 0 else 92 if code == 12 else 0
    b = 127 if code == 8 else 238 if code == 4 else level if (code & 4) != 0 else 0
    rgb = rgb565(r, g, b) 
    print(f"0x{rgb:04X}", end=',')
    # print(f"{code:3d}: {r:02X} {g:02X} {b:02X}")

print("")

print("colors 16-231 are a 6x6x6 color cube")
for red in range(0, 6):
    for green in range(0, 6):
        for blue in range(0, 6):
            code = 16 + (red * 36) + (green * 6) + blue
            r = red   * 40 + 55 if red   != 0 else 0
            g = green * 40 + 55 if green != 0 else 0
            b = blue  * 40 + 55 if blue  != 0 else 0
            rgb = rgb565(r, g, b) 
            print(f"0x{rgb:04X}", end=',')
            if (code & 0xf)==0xf :
                print()
            # print(f"{code:3d}: {r:02X} {g:02X} {b:02X}")
print('')

print("colors 232-255 are a grayscale ramp, intentionally leaving out black and white")
code = 232
for gray in range(0, 24):
    level = gray * 10 + 8
    code = 232 + gray
    rgb = rgb565(level, level, level) 
    print(f"0x{rgb:04X}", end=',')
    # print(f"{code:3d}: {level:02X} {level:02X} {level:02X}")

print('')
