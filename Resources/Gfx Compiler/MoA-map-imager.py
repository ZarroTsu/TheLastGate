#! /usr/bin/python
# python3

import os
import sys
from threading import Thread
#import console
import math
import time
import re
import json
try:
    from PIL import Image
except:
    try:
        from pillow import Image
    except:
        try:
            import Image
        except:
            print("Failed:")
            print("from PIL import Image")
            print("from pillow import Image")
            print("import Image")

timeKeyboardInterrupt = None
SETTINGS_FILE = "MoA-map-imager.conf"
settings = json.loads("{}")

mapFile = "map.dat"
itemsFile = "item.dat"
charsFile = "char.dat"
tileLength = None
itemLength = None
charLength = None
mapWidth = 2048
mapHeight = 1024
mapHexData = bytearray()
itemsHexData = bytearray()
tilesList = {}
itemsList = {}
surface = None
surfaceAltered = False

# Colour for missing image files
missingColour = (255, 0, 255)

imageColourOverrides = { "1090": (224, 130, 29),   # experience pole
                         "1091": (224, 130, 29),   # experience pole
                         "914": (175, 151, 12),    # shrine   
                         "16900": (206, 180, 132), # lab 9 white candlestick
                         "16510": (168, 84, 45),   # dark metal fire-bowl
                         "1750": (42, 42, 40) }    # tombstone

imageColourIgnore = [ "16972" ] # soulstones

imageColourCache = {}

# Drawing
layers = { "background"  : True, 
           "foreground"  : True,
           "items"       : True,
           "itemsCarried": False,
           "characters"  : False }

def displayImage(image):
    image.show()

def catchDoubleKeyboardInterrupt():
    global timeKeyboardInterrupt
    
    if timeKeyboardInterrupt:
        if time.time() - timeKeyboardInterrupt < 2:
            sys.exit()
    
    timeKeyboardInterrupt = time.time()

def loadSettings():
    global settings
    global mapFile
    global itemsFile
    global charsFile
    global mapWidth
    global mapHeight
    global tileLength
    global itemLength
    global charLength
    global missingColour
    global imageColourOverrides
    global imageColourIgnore
    global layers
    
    if os.path.exists(os.getcwd() + '/' + SETTINGS_FILE):   
        if os.path.isfile(os.getcwd() + '/' + SETTINGS_FILE) and os.access(os.getcwd() + '/' + SETTINGS_FILE, os.X_OK):
            try:        
                with open(os.getcwd() + '/' + SETTINGS_FILE, "r") as file:    
                    settings = json.load(file)
                    
                mapFile = settings["map_file"]
                itemsFile = settings["items_file"]
                charsFile = settings["chars_file"]
                mapWidth = settings["map_width"]
                mapHeight = settings["map_height"]
                tileLength = settings["tile_length"]
                itemLength = settings["item_length"]
                charLength = settings["char_length"]
                missingColour = tuple(settings["missing_colour"])
                for key, value in settings["image_colour_override"].items():
                    imageColourOverrides[key] = tuple(value)
                imageColourIgnore = settings["image_ignore"]
                layers = settings["layer_visibility"]
            except:
                print("Unable to load settings")

def saveSettings():
    global settings
    
    settings["map_file"] = mapFile
    settings["items_file"] = itemsFile
    settings["chars_file"] = charsFile
    settings["map_width"] = mapWidth
    settings["map_height"] = mapHeight
    settings["tile_length"] = tileLength
    settings["item_length"] = itemLength
    settings["char_length"] = charLength
    settings["missing_colour"] = missingColour
    settings["image_colour_override"] = imageColourOverrides
    settings["image_ignore"] = imageColourIgnore
    settings["layer_visibility"] = layers

    try:
        with open(os.getcwd() + '/' + SETTINGS_FILE, "w+") as file:    
            json.dump(settings, file)
    except:
        print("Unable to save settings")

def loadMap():
    global mapHexData
    print("Loading map file ", end='')
    sys.stdout.flush()
    with open(os.getcwd() + '/' + mapFile, "rb") as file:    
        mapHexData = file.read().hex().lower()
    sys.stdout.flush()
    print("[100%]")

def loadItems():
    global itemsHexData
    print("Loading items file ", end='')
    sys.stdout.flush()
    with open(os.getcwd() + '/' + itemsFile, "rb") as file:    
        itemsHexData = file.read().hex().lower()
    sys.stdout.flush()
    print("[100%]")

def calibrateLengths():
    global tileLength 
    global itemLength
    global charLength
    
    # Assumes first two tiles are the same)
    portion = ""
    for h in mapHexData[8:]:
        portion += str(h)
        if portion[-8:] == mapHexData[:8]:
            tileLength = len(portion)
            break
    
    # No calibration yet
    itemLength = 1268
    charLength = 99999
    
    try:
        if tileLength > 0 and itemLength > 0 and charLength > 0:
            return
    except:
        print("Unable to determine data length")
        sys.exit()

def readMap():
    global tilesList
    tileCounter = 0
    hexCounter = 0
    print("Reading map file [0%]\r", end='')
    sys.stdout.flush()
    for h in mapHexData:
        try:
            tilesList[tileCounter] = tilesList[tileCounter] + str(h)
        except:
            tilesList[tileCounter] = ""
            tilesList[tileCounter] = tilesList[tileCounter] + str(h)
        hexCounter += 1
        if hexCounter == tileLength:
            
            tileCounter += 1
            hexCounter = 0
        
        if tileCounter % mapHeight == 0:
            print("Reading map file [{0}%]\r".format(int(tileCounter / (mapWidth * mapHeight) * 100)), end='')
            sys.stdout.flush()
    print("Reading map file [100%]")

def readItems():
    global itemsList
    itemCounter = 0
    hexCounter = 0
    print("Reading items file [0%]\r", end='')
    sys.stdout.flush()
    itemsFileSize = os.stat(os.getcwd() + '/' + itemsFile).st_size
    itemsTotalEstimate = int(math.ceil(itemsFileSize / 634 / 100) * 100)  
    if itemsHexData[:2] == "00":
        offset = 2
    else:
        offset = 0
    for h in itemsHexData[offset:]:
        try:
            itemsList[itemCounter] = itemsList[itemCounter] + str(h)
        except:
            itemsList[itemCounter] = ""
            itemsList[itemCounter] = itemsList[itemCounter] + str(h)
        hexCounter += 1
        if hexCounter == itemLength: 
            itemCounter += 1
            hexCounter = 0
        if itemCounter % (itemsTotalEstimate / 100) == 0:
            percentage = int(itemCounter / itemsTotalEstimate * 100)
            if percentage > 100:
                percentage = 100
            print("Reading items file [{0}%]\r".format(percentage), end='')
            sys.stdout.flush()
    print("Reading items file [100%]")

def drawMap():
    global surface
    try:
        surface = Image.new('RGB', (mapWidth, mapHeight), 0x000000)
    except:
        print("Pillow not working as expected, likely not running Python 3.6+ / Pillow 8.0+")
        sys.exit()
    
    print("Drawing map [0%]\r", end='')
    sys.stdout.flush()
    x = 0
    y = 0
    for key, value in tilesList.items():
        sprite = value[:8]
        background = sprite[:4]
        sprite = sprite[4:]
        
        # Background
        if layers["background"] is True:
            colour = getImageColour(background)
            if colour:
                surface.putpixel((y, x % mapHeight), colour)
            else:
                surface.putpixel((y, x % mapHeight), missingColour)
        # Foreground
        if layers["foreground"] is True:
            colour = getImageColour(sprite)
            if colour:
                surface.putpixel((y, x % mapHeight), colour)
            else:
                if sprite != "0000":
                    print("Image file not found ({0})".format(str(int(imageNumber[2:] + imageNumber[:2], 16)).zfill(5)))
        
        x += 1
        if x % mapHeight == 0:
            y += 1
            if layers["items"] is True:
                # Items are being drawn
                tilesPercentage = int(y / mapWidth * 100 * 0.66)
            else:
                # Items are not being drawn
                tilesPercentage = int(y / mapWidth * 100)
                pass
            if tilesPercentage > 100:
                tilesPercentage = 100
            print("Drawing map [{0}%]\r".format(tilesPercentage), end='')
            sys.stdout.flush()
    # Items
    if layers["items"] is True:
        itemCounter = 0
        for key, value in itemsList.items():
            try:
                x = int(value[1062:1064] + value[1060:1062], 16)
                y = int(value[1058:1060] + value[1056:1058], 16)
                item = value[1072:1076]
                 
                colour = getImageColour(item)
                if colour:
                    surface.putpixel((x, y % mapHeight), colour)
            except Exception as exception:
                print(exception)
                pass
            
            if itemCounter % (int(len(itemsList) / 100)) == 0:
                itemsPercentage = tilesPercentage + int(itemCounter / len(itemsList) * 100 * 0.34)
                if itemsPercentage > 100:
                    itemsPercentage = 100
                print("Drawing map [{0}%]\r".format(itemsPercentage), end='')
                sys.stdout.flush()
            itemCounter += 1
    print("Drawing map [100%]")

def getImageColour(hex):
    imageNumber = str(int(hex[2:] + hex[:2], 16)).zfill(5)

    # Ignore blank
    if imageNumber == "00000":
        return None
    # Check ignores
    if imageNumber.lstrip("0") in imageColourIgnore or imageNumber in imageColourIgnore:
        return None
    # Check overrides
    for key, value in imageColourOverrides.items():
        if key == imageNumber.lstrip("0") or key == imageNumber:
            return value
    # Check cache
    for key, value in imageColourCache.items():
        if key == imageNumber:
            return value
    
    try:
        image = Image.open(os.getcwd() + "/gfx/" + imageNumber + ".png")
    except:
        try:
            image = Image.open(os.getcwd() + "/gfx/" + imageNumber + ".bmp")
        except:
            return None
    
    pixels = list(image.getdata())
    averageRed = 0
    averageGreen = 0
    averageBlue = 0
    pixelCount = 0 
    # Check for transparency
    if image.mode == "RGBA":
        imageDimensions = image.size[0] * image.size[1]
        for channels in pixels:
            if channels[3] != 0: # transparency
                averageRed += channels[0]
                averageGreen += channels[1]
                averageBlue += channels[2]
                pixelCount += 1
        percentageTransparent = 100 / (imageDimensions / pixelCount)
        if percentageTransparent < 10:
            # May still be looking for a colour mask
            image.convert("RGB")
    # Determine average colour, excluding masks
    if image.mode == "RGB":
        for channels in pixels:
            #255, 255, 255 # white mask
            if channels != (255, 0, 255) and channels != (254, 0, 254): # pink mask
                averageRed += channels[0]
                averageGreen += channels[1]
                averageBlue += channels[2]
                pixelCount += 1
            #127, 127, 127 # grey mask
            #96, 96, 96 # grey mask
            #0, 0, 0 # black mask
    
    imageColour = (int(averageRed / pixelCount), int(averageGreen / pixelCount), int(averageBlue / pixelCount))
    imageColourCache[imageNumber] = imageColour
    
    return imageColour

def getLocation(x, y):
    if x < 0 or x > mapWidth or y < 0 or y > mapHeight:
        print("Coordinates outside map bounds")
        return False
    location = ""
    for h in mapHexData[(y + x * mapWidth) * tileLength:(y + x * mapWidth) * tileLength + tileLength]:
        location += str(h)
    try:
        background = int(location[2:4] + location[:2], 16)
    except:
        return False
    if background == 0:
        background = "none"
    foreground = int(location[6:8] + location[4:6], 16)
    if foreground == 0:
        foreground = "none"
    print("{0},{1}: {2}, {3}".format(x, y, background, foreground), end='')
    foundValidName = False
    for key, value in itemsList.items():
        try:
            itemX = int(value[1062:1064] + value[1060:1062], 16)
            itemY = int(value[1058:1060] + value[1056:1058], 16)         
            if itemX == x and itemY == y:
                name = value[:40].rstrip('0')
                if int(value[1066:1068] + value[1064:1066], 16) == 0:
                    # Item not being carried (may be the wrong value)
                    
                    if int(value[1074:1076] + value[1072:1074], 16) != 0:# and int(value[1076:1078], 16) == 0:
                        # Exclude 'items' that represent an active spell effect (icon above player model up top of GUI)
                        nameLength = 80
                        name = value[:nameLength].rstrip('0')
                        while foundValidName is False:
                            # Stops an odd issue with Titanium Two-Handed's name (and maybe others) throwing an exception
                            try:
                                print(", {0} ({1})".format(int(value[1074:1076] + value[1072:1074], 16), bytes.fromhex(name).decode('utf-8')))
                                foundValidName = True
                            except:
                                nameLength -= 2
                                if nameLength > 1:
                                    name = value[:nameLength].rstrip('0')
                                else:
                                    name = "unavailable"                                
        except:
            pass
    if foundValidName is False:
        print(", none")
    
    return True

print("\nMercenaries of Astonia map imager v2.1")
print("by Makadon\n")

loadSettings()

if len(sys.argv) == 1:
    
    loadMap()
    calibrateLengths()
    saveSettings()
    loadItems()
    readMap()
    readItems()
    drawMap()
    
    print("Type \"help\" for commands")
    command = ""
    
    while True:
        try:
            command = input("> ").lower().strip(' ')
        except:
            print("")
            catchDoubleKeyboardInterrupt()
            continue
        
        if command == "exit" or command == "quit" or command == "e" or command == "x" or command == "q":
            sys.exit()
        
        commandRegex = re.search("^([?\/(\-]?\??[a-zA-Z0-9-]*)?( *(,|\.| ) *([a-zA-Z0-9.\"-]*\+?\)?))?( *(,|\.| ) *([a-zA-Z0-9-]*))?( *(,|\.| ) *([a-zA-Z0-9]*))?( *(,|\.| ) *([a-zA-Z0-9]*))?", command, re.IGNORECASE)
        
        try:
            for i in range(14):
                commandRegex[i].lower().strip(' \'`"!@#$^()[]{}<>;:|~=')
        except:
            pass
            
        if commandRegex[1] == "?" or commandRegex[1] == "/?" or commandRegex[1] == "h" or commandRegex[1] == "help" or commandRegex[1] == "-h" or commandRegex[1] == "--h" or commandRegex[1] == "-help" or commandRegex[1] == "--help" or commandRegex[1] == "commands":
            print("Usage:")
            #print("begin number                     --- set starting point for map saving and displaying")
            #print("end number                       --- set stopping point for map saving and displaying")
            print("toggle bg/fg/items/carried/chars --- toggle layer")
            print("display map/number               --- display map or image")
            print("save [filename.dat]              --- save map")
            print("location x, y                    --- list location info, e.g. location x1-x2, y, location x, -y2")
            print("search word                      --- search item names, one word")
            #print("mask r, g, b                     --- add, remove colour mask")
            print("ignore number                    --- ignore image number")
            print("override number r, g, b          --- override image colour")
            #print("missing r, g, b                  --- set missing image colour")
            print("map filename                     --- set map file")
            print("items filename                   --- set items file")
            #print("chars filename                   --- set characters file")
            #print("width number                     --- define map width")
            #print("height number                    --- define map height")
            #print("tilelength number                --- define number of hex values between tiles")
            #print("itemlength number                --- define number of hex values between items")
            #print("charlength number                --- define number of hex values between characters")
            #print("copyminimum                      --- create copies of data files with only necessary information")
            print("defaults                         --- reset settings")
            print("")
            print("Omit parameters to see current")
            print("")
            
        elif commandRegex[1] == "save":
            # Save map
            
            if surfaceAltered is True:
                drawMap()
                surfaceAltered = False
            
            if commandRegex[4]:
                if commandRegex[4][-4:] == ".png" or commandRegex[2][-4:] == ".bmp":  
                    surface.save(commandRegex[4])
                else:
                    print("Filetype must be \"png\" or \"bmp\"")
            else:
                surface.save("map.png")
        elif commandRegex[1] == "display" or commandRegex[1] == "d":
            # Open image
                 
            if commandRegex[4]:
                if surfaceAltered is True:
                    drawMap()
                    surfaceAltered = False
                
                if commandRegex[4] == "map":
                    try:
                        if threadSurface.is_alive() is True:
                            continue    
                    except:
                        pass
                    
                    threadSurface=Thread(target=displayImage,args=(surface,))
                    threadSurface.start()
                else:
                    imageNumberString = re.search("^\d{1,5}$", commandRegex[4].lstrip('0'))
                    if imageNumberString:
                        imageNumber = int(imageNumberString[0]) 
                        if imageNumber > 0 and imageNumber < 65536:
                            try:
                                image = Image.open(os.getcwd() + "/png/" + imageNumberString[0].zfill(5) + ".png")
                            except:
                                try:
                                    image = Image.open(os.getcwd() + "/bmp/" + imageNumberString[0].zfill(5) + ".bmp")
                                except:
                                    print("Image file not found ({0})".format(imageNumberString[0].zfill(5)))
                            if image:
                                try:
                                    if threadImage.is_alive() is True:
                                        continue
                                except:
                                    pass

                                threadImage=Thread(target=displayImage,args=(image,))
                                threadImage.start()
                                #if os.name != "nt": # may be useful for linux still
                                #    pid=os.fork()
                                #    if pid:
                                #        image.show()
                                #    else:
                                #        pass
                            continue
                            
                    print("Image number must be between 1-65535")
            else:
                print("Usage: display map/number")
                
                
        elif commandRegex[1] == "defaults":
            # Reset everything to defaults, confirm command
            mapFile = "map.dat"
            itemsFile = "item.dat"
            charsFile = "char.dat"
            calibrateLengths()
            layers["background"] = True
            layers["foreground"] = True
            layers["items"] = True
            layers["itemsCarried"] = False
            layers["characters"] = False
            imageColourOverrides = {}
            imageColourIgnore = [ "16972" ]
            missingColour = (255, 0, 255)
            
            surfaceAltered = True
            saveSettings()
        
        elif commandRegex[1] == "map":
            # Change map file
            if commandRegex[4]:
                if commandRegex[4][-4:] == ".dat":
                    mapFile = commandRegex[4]
                    loadMap()
                    readMap()
                    surfaceAltered = True
                    saveSettings()
                    continue
            
            print("Current: {0}".format(mapFile))
        elif commandRegex[1] == "items" or commandRegex[1] == "item":
            # Change items file
            if commandRegex[4]:
                if commandRegex[4][-4:] == ".dat":
                    itemsFile = commandRegex[4]
                    loadItems()
                    readItems()
                    surfaceAltered = True
                    saveSettings()
                    continue
            
            print("Current: {0}".format(itemsFile))
        elif commandRegex[1] == "chars" or commandRegex[1] == "character" or commandRegex[1] == "characters":
            pass
        elif commandRegex[1] == "copyminimum" or commandRegex[1] == "copyminimums" or commandRegex[1] == "copymin" or commandRegex[1] == "copymins":
            # Create copies of data files with all unneeded information blanked
            pass
        elif commandRegex[1] == "width":
            pass
        elif commandRegex[1] == "height":
            pass
        elif commandRegex[1] == "tilelength" or commandRegex[1] == "tilelen":
            if commandRegex[4]:
                tileLength = commandRegex[4]
                
                loadMap()
                readMap()
                surfaceAltered = True
                saveSettings()
            else:
                print("Current: {0}".format(str(tileLength)))
        elif commandRegex[1] == "itemlength" or commandRegex[1] == "itemlen":
            if commandRegex[4]:
                itemLength = commandRegex[4]
                
                loadItems()
                readItems()
                surfaceAltered = True
                saveSettings()
            else:
                print("Current: {0}".format(str(itemLength)))
        elif commandRegex[1] == "charlength" or commandRegex[1] == "charlen" or commandRegex[1] == "characterlength" or commandRegex[1] == "characterlen":
            pass
        elif commandRegex[1] == "begin":
            # Start point for map drawing
            pass
        elif commandRegex[1] == "end":
            # Stop point for map drawing
            pass
        elif commandRegex[1] == "toggle" or commandRegex[1] == "t":
            # Toggle layer
            
            if commandRegex[4]:
                if commandRegex[4] == "background" or commandRegex[4] == "bg" or commandRegex[4] == "back":
                    layers["background"] = not layers["background"]
                elif commandRegex[4] == "foreground" or commandRegex[4] == "fg" or commandRegex[4] == "fore":
                    layers["foreground"] = not layers["foreground"]
                elif commandRegex[4] == "items" or commandRegex[4] == "item":
                    layers["items"] = not layers["items"]
                elif commandRegex[4] == "carried" or commandRegex[4] == "itemscarried" or commandRegex[4] == "itemsc" or commandRegex[4] == "itemc":
                    #layers["itemsCarried"] = not layers["itemsCarried"]
                    pass
                elif commandRegex[4] == "characters" or commandRegex[4] == "character" or commandRegex[4] == "chars" or commandRegex[4] == "char":
                    #layers["characters"] = not layers["characters"]
                    pass
                else:
                    print("No such layer")
                    continue
                
                surfaceAltered = True
                saveSettings()
            
            for key, value in layers.items():
                print("{0}: {1}".format(key, str(value).lower()))
        elif commandRegex[1] == "show":
            # Show layer
            
            if commandRegex[4]:
                if commandRegex[4] == "background" or commandRegex[4] == "bg" or commandRegex[4] == "back":
                    if layers["background"] is False:
                        layers["background"] = True
                elif commandRegex[4] == "foreground" or commandRegex[4] == "fg" or commandRegex[4] == "fore":
                    if layers["foreground"] is False:
                        layers["foreground"] = True
                elif commandRegex[4] == "items" or commandRegex[4] == "item":
                    if layers["items"] is False:
                        layers["items"] = True
                elif commandRegex[4] == "carried" or commandRegex[4] == "itemscarried" or commandRegex[4] == "itemsc" or commandRegex[4] == "itemc":
                    #if layers["itemsCarried"] is False:
                    #    layers["itemsCarried"] = True
                    pass
                elif commandRegex[4] == "characters" or commandRegex[4] == "character" or commandRegex[4] == "chars" or commandRegex[4] == "char":
                    #if layers["characters"] is False:
                    #    layers["characters"] = True
                    pass
                else:
                    print("No such layer")
                    continue
                
                surfaceAltered = True
                saveSettings()
            else:
                for key, value in layers.items():
                    print("{0}: {1}".format(key, str(value).lower()))
        elif commandRegex[1] == "hide":
            # Hide layer
            
            if commandRegex[4]:
                if commandRegex[4] == "background" or commandRegex[4] == "bg" or commandRegex[4] == "back":
                    if layers["background"] is True:
                        layers["background"] = False
                elif commandRegex[4] == "foreground" or commandRegex[4] == "fg" or commandRegex[4] == "fore":
                    if layers["foreground"] is True:
                        layers["foreground"] = False
                elif commandRegex[4] == "items" or commandRegex[4] == "item":
                    if layers["items"] is True:
                        layers["items"] = False
                elif commandRegex[4] == "carried" or commandRegex[4] == "itemscarried" or commandRegex[4] == "itemsc" or commandRegex[4] == "itemc":
                    #if layers["itemsCarried"] is True:
                    #    layers["itemsCarried"] = False
                    pass
                elif commandRegex[4] == "characters" or commandRegex[4] == "character" or commandRegex[4] == "chars" or commandRegex[4] == "char":
                    #if layers["characters"] is True:
                    #    layers["characters"] = False
                    pass
                else:
                    print("No such layer")
                    continue
                
                surfaceAltered = True
                saveSettings()
            else:
                for key, value in layers.items():
                    print("{0}: {1}".format(key, str(value).lower()))
        elif commandRegex[1] == "mask" or commandRegex[1] == "masks":
            # List masks
            pass
        elif commandRegex[1] == "ignore" or commandRegex[1] == "i":
            # Ignore image number
            
            if commandRegex[4]:
                imageNumberString = re.search("^\d{1,5}$", commandRegex[4].lstrip('0'))
                if imageNumberString:   
                    imageNumber = int(imageNumberString[0]) 
                    if imageNumber > 0 and imageNumber < 65536:
                        if not imageNumberString[0].zfill(5) in imageColourIgnore or not imageNumberString[0] in imageColourIgnore:
                            imageColourIgnore.append(imageNumberString[0])
                            surfaceAltered = True
                            saveSettings()
                        else:
                            index = len(imageColourIgnore) - 1
                            for item in reversed(imageColourIgnore):
                                if item == imageNumberString[0]:
                                    del imageColourIgnore[index]
                                index -= 1
                            surfaceAltered = True
                            saveSettings()
                            print("{0} no longer ignored".format(imageNumberString[0]))
                        continue
                        
                print("Image number must be between 1-65535")
                continue
            
            sortedIgnores = []
            for item in imageColourIgnore:
                sortedIgnores.append(item.zfill(5))
            for item in sorted(sortedIgnores):
                print(item.lstrip('0'))
            
        elif commandRegex[1] == "override" or commandRegex[1] == "overide" or commandRegex[1] == "o":
            # Override image number's colour
            
            if commandRegex[4]:
                imageNumberString = re.search("^\d{1,5}$", commandRegex[4].lstrip('0'))
                if imageNumberString:   
                    imageNumber = int(imageNumberString[0]) 
                    if imageNumber > 0 and imageNumber < 65536: 
                        if commandRegex[7] and commandRegex[10] and commandRegex[13]:
                            RGBoffset = 7
                            RGB = {}
                            for index in range(3):
                                RGBNumberString = re.search("^\d{1,3}$", commandRegex[RGBoffset])[0]
                                if RGBNumberString:
                                    if RGBNumberString != "0":
                                        RGBNumberString = RGBNumberString.lstrip('0')
                                    RGBNumber = int(RGBNumberString) 
                                    if RGBNumber > -1 and RGBNumber < 256:
                                        RGB[str(RGBoffset)] = RGBNumber
                                    else:
                                        print("Parameters must be between 0-255")
                                        continue
                                else:
                                    print("Usage: override [image number] [red, green, blue]")
                                    continue
                                    
                                RGBoffset += 3
                            
                            imageColourOverrides[imageNumberString[0]] = (RGB["7"], RGB["10"], RGB["13"])
                            surfaceAltered = True
                            saveSettings()
                        else:
                            if imageNumberString[0].zfill(5) in imageColourOverrides:
                                del imageColourOverrides[imageNumberString[0].zfill(5)]
                            elif imageNumberString[0] in imageColourOverrides:
                                del imageColourOverrides[imageNumberString[0]]
                            else:
                                print("Image number has no override")
                                continue
                            surfaceAltered = True
                            saveSettings()
                            print("{0} override removed".format(imageNumberString[0]))
                    else:
                        print("Image number must be between 1-65535")
                
            else:
                sortedOverrides = {}
                for key, value in imageColourOverrides.items():
                    sortedOverrides[key.zfill(5)] = value
                for key, value in sorted(sortedOverrides.items()):
                    print("{0}: {1},{2},{3}".format(key.lstrip('0'), value[0], value[1], value[2]))
        
        elif commandRegex[1] == "missing" or commandRegex[1] == "missingcolour" or commandRegex[1] == "missingcolor":
            # Set missing image file colour
            pass
        elif commandRegex[1] == "search" or commandRegex[1] == "s" or commandRegex[1] == "find" or commandRegex[1] == "f":
            # Search for items
            
            if commandRegex[4]:
                parameter = commandRegex[4].strip('"\'`').lower()
                if len(parameter) < 3:
                    print("Word must be at least three characters long")
                    continue 
                itemCounter = 0
                itemNames = {}
                itemDescriptions = {}
                itemNamesFound = []
                results = {}
                
                print("name (image number): description")
                for key, value in itemsList.items():
                
                    itemNames[key] = bytes.fromhex(value[:80]).decode('utf-8').rstrip('\x00')
                    itemDescriptions[key] = bytes.fromhex(value[160:560]).decode('utf-8').rstrip('\x00')
                    itemDescription = itemDescriptions[key]
                    imageColour = int(value[1074:1076] + value[1072:1074], 16)
                    itemNamesWords = itemNames[key].lower().split()
                    itemDescriptionsWords = itemDescriptions[key].lower().split()
                    
                    # Truncate item description
                    length = 80 - (len(itemNames[key]) + len(str(imageColour)) + 5)
                    if length < 0:
                        length = 0
                    if len(itemDescriptions[key]) > length:
                        itemDescription = ' '.join(itemDescription[:length+1].split(' ')[0:-1]) + "..."
                    
                    if len(itemDescriptions[key]) > 3:
                        if itemDescription[-4:] == "....":
                            itemDescription = itemDescription[:-1]
                        elif itemDescription[-4:] == ",...":
                            itemDescription = itemDescription[:-4]
                            itemDescription += "..."

                    if itemDescription == "...":
                        itemDescription = ""
                    
                    if parameter in itemNamesWords or parameter in itemDescriptionsWords:
                        if itemNames[key] not in itemNamesFound:
                            itemNamesFound.append(itemNames[key])
                            if imageColour in results:
                                results[imageColour] = (*results[imageColour], itemNames[key], itemDescription)
                            else:
                                results[imageColour] = (itemNames[key], itemDescription)
                    else:
                        for word in itemNamesWords:
                            if parameter in word.lower():
                                if itemNames[key] not in itemNamesFound:
                                    itemNamesFound.append(itemNames[key])
                                    results[imageColour] = (itemNames[key], itemDescription)
                        for word in itemDescriptionsWords:
                            if parameter in word.lower():
                                if itemNames[key] not in itemNamesFound:
                                    itemNamesFound.append(itemNames[key])
                                    results[imageColour] = (itemNames[key], itemDescription)
                
                for key, value in sorted(results.items()):
                    try:
                        index = 0
                        while True:
                            print("{0} ({1}): {2}".format(value[index], key, value[index + 1]))
                            index += 2                  
                    except:
                        pass              
            else:
                print("Usage: search word")
            
        elif commandRegex[1] and commandRegex[4]:
            # List location info     
            
            if commandRegex[1] and commandRegex[4]:
                commandCoordinateX = re.search("^(\d{1,4})? *-? *(\d{1,4})?-?$", commandRegex[1])
                commandCoordinateY = re.search("^(\d{1,4})? *-? *(\d{1,4})?-?$", commandRegex[4])
            if commandRegex[4] and commandRegex[7]:
                commandCoordinateXAlt = re.search("^(\d{1,4})? *-? *(\d{1,4})?-?$", commandRegex[4])
                commandCoordinateYAlt = re.search("^(\d{1,4})? *-? *(\d{1,4})?-?$", commandRegex[7])
            
            if (commandRegex[1] and commandCoordinateX and commandCoordinateY) or ((commandRegex[1] == "location" or commandRegex[1] == "loc" or commandRegex[1] == "locate" or commandRegex[1] == "tile") and commandRegex[7] and commandCoordinateXAlt and commandCoordinateYAlt):
                if commandRegex[1] == "location" or commandRegex[1] == "loc" or commandRegex[1] == "locate" or commandRegex[1] == "l" or commandRegex[1] == "tile":
                    commandOffset = 3
                else:
                    commandOffset = 0
                
                x1 = 0
                x2 = mapWidth
                y1 = 0
                y2 = mapHeight
                commandCoordinateXRange = re.search("^(\d{1,4})? *- *(\d{1,4})?$", commandRegex[1 + commandOffset])
                commandCoordinateYRange = re.search("^(\d{1,4})? *- *(\d{1,4})?$", commandRegex[4 + commandOffset])            
                
                if commandCoordinateXRange:
                    if commandCoordinateXRange[1]:
                        x1 = commandCoordinateXRange[1]
                    if commandCoordinateXRange[2]:
                        x2 = commandCoordinateXRange[2]
                if commandCoordinateYRange:
                    if commandCoordinateYRange[1]:
                        y1 = commandCoordinateYRange[1]
                    if commandCoordinateYRange[2]:
                        y2 = commandCoordinateYRange[2]
                
                if int(x2) < int(x1):
                    temp = x1
                    x1 = x2
                    x2 = temp
                if int(y2) < int(y1):
                    temp = y1
                    y1 = y2
                    y2 = temp
                
                if commandCoordinateXRange:
                    if commandCoordinateYRange:
                        print("Range only on one axis at a time")
                    else:
                        if commandOffset == 0:
                            y1 = commandCoordinateY[1]
                        else:
                            y1 = commandCoordinateYAlt[1]
                        
                        print("x,y: background, foreground, item")
                        for x in range(int(x1), int(x2) + 1):
                            if getLocation(x, int(y1)) is False:
                                print("Invalid map data")
                                break
                else:
                    if commandCoordinateYRange:
                        if commandOffset == 0:
                            x1 = commandCoordinateX[1]
                        else:
                            x1 = commandCoordinateXAlt[1]
                        
                        print("x,y: background, foreground, item")
                        for y in range(int(y1), int(y2) + 1):
                            if getLocation(int(x1), y) is False:
                                print("Invalid map data")
                                break
                    else:
                        if commandOffset == 0:
                            x1 = commandCoordinateX[1]
                        else:
                            x1 = commandCoordinateXAlt[1]
                        if commandOffset == 0:
                            y1 = commandCoordinateY[1]
                        else:
                            y1 = commandCoordinateYAlt[1]
                        
                        print("x,y: background, foreground, item")
                        if getLocation(int(x1), int(y1)) is False:
                            print("Invalid map data")
            else:
                print("Usage: location x, y")
        else:
            print("Unknown command")            

elif len(sys.argv) > 1:
    try:
        gettingLocation = False
        for index, arg in enumerate(sys.argv):
            if index == 0:
                continue             
            elif sys.argv[index] == "-m" or sys.argv[index] == "-map":
                if sys.argv[index + 1][-4:] == ".dat": 
                    mapFile = sys.argv[index + 1] 
                else:
                    raise
            elif sys.argv[index] == "-i" or sys.argv[index] == "-items" or sys.argv[index] == "-item":
                if sys.argv[index + 1][-4:] == ".dat": 
                    itemsFile = sys.argv[index + 1]
                else:
                    raise
            elif sys.argv[index] == "-l" or sys.argv[index] == "-location" or sys.argv[index] == "-loc":
                gettingLocation = True
                coords = re.search("(-?\d{1,4})(,|.)(-?\d{1,4})", sys.argv[index + 1])   
        
        loadMap()
        calibrateLengths()
        loadItems()
        readMap()
        readItems()
        
        if gettingLocation is False:
            drawMap()
            surface.save("map.png")
        else:
            print("x,y: background, foreground, item")
            getLocation(int(coords[1]), int(coords[3]))
        
    except Exception as exception:
        print("Usage:")
        print("python MoA-map-imager.py [-map map.dat] [-items item.dat] [-location x,y]")
        sys.exit()