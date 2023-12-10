#!/usr/bin/python2

import sys
import os.path
import subprocess
import struct

def printGuid(packedString):
    if len(packedString) == 16:
        outstr = format(struct.unpack('<L', packedString[0:4])[0], 'X').zfill(8) + "-" + \
            format(struct.unpack('<H', packedString[4:6])[0], 'X').zfill(4) + "-" + \
            format(struct.unpack('<H', packedString[6:8])[0], 'X').zfill(4) + "-" + \
            format(struct.unpack('>H', packedString[8:10])[0], 'X').zfill(4) + "-" + \
            format(struct.unpack('>Q', "\x00\x00" + packedString[10:16])[0], 'X').zfill(12)
    else:
        outstr = "<invalid>" 
    return outstr

class GptRecord():
    def __init__(self, recs, partno):
        self.partno = partno
        offset = partno * 128
        self.empty = False
        
        # build partition type GUID string
        self.partType = printGuid(recs[offset:offset+16])
        if self.partType == "00000000-0000-0000-0000-000000000000":
            self.empty = True
            
        self.partGUID = printGuid(recs[offset+16:offset+32]) 
        self.firstLBA = struct.unpack('<Q', recs[offset+32:offset+40])[0]
        self.lastLBA = struct.unpack('<Q', recs[offset+40:offset+48])[0]
        self.attr = struct.unpack('<Q', recs[offset+48:offset+56])[0]
        nameIndex = recs[offset+56:offset+128].find('\x00\x00')
        
        if nameIndex != -1:
            self.partName = recs[offset+56:offset+56+nameIndex].encode('utf-8')
        else:
            self.partName = recs[offset+56:offset+128].encode('utf-8')

    def printPart(self):
        if not self.empty:
            outstr = str(self.partno) + " : " + self.partType + " : " + self.partGUID + \
                " : " + str(self.firstLBA) + " : " + str(self.lastLBA) + " : " + \
                        str(self.attr) + " : " + self.partName
            print(outstr)


class MbrRecord():
    def __init__(self, sector, partno):
        self.partno = partno
        offset = 446 + partno * 16
        self.active = False
        if sector[offset] == '\x80':
            self.active = True
        self.type = ord(sector[offset+4])
        self.empty = False
        if self.type == 0:
            self.empty = True
        self.start = ord(sector[offset+8]) + ord(sector[offset+9]) * 256 + \
            ord(sector[offset+10]) * 65536 + ord(sector[offset+11]) * 16777216 
        self.sectors = ord(sector[offset+12]) + ord(sector[offset+13]) * 256 + \
            ord(sector[offset+14]) * 65536 + ord(sector[offset+15]) * 16777216
   
    def printPart(self):
        if self.empty == True:
            print("<empty>")
        else:
            outstr = "" 
            if self.active == True:
                outstr += "Bootable:"
            outstr += "Type " + hex(self.type) + ":"
            outstr += "Start " + str(self.start) + ":"
            outstr += "Total sectors " + str(self.sectors)
            print(outstr)
    
def usage():
    print("usage " + sys.argv[0] + " <image file>\nWrites information about GPT disks.")
    exit(1)

def main():
    if len(sys.argv) < 2: 
        usage()
    
    # read first sector
    if not os.path.isfile(sys.argv[1]):
        print("File " + sys.argv[1] + " cannot be openned for reading")
        exit(1)
        
    with open(sys.argv[1], 'rb') as f:
        sector = str(f.read(512))
        
    if (sector[510] == "\x55" and sector[511] == "\xaa"):
        # if it is an MBR bytes 446, 462, 478, and 494 must be 0x80 or 0x00
        if(sector[446] == "\x00"):
            part = MbrRecord(sector, 0)
            if part.type != 0xee:
                print("Failed protective MBR sanity check")
                exit(1)
                    
            # check the header as another sanity check
            with open(sys.argv[1], 'rb') as f:
                f.seek(512)
                sector = str(f.read(512))
            if sector[0:8] != "EFI PART":
                print("You appear to be missing a GUI header")
                exit(1)
                
            print("Valid protective MBR and GUI partion table header found")
            with open(sys.argv[1], 'rb') as f:
                f.seek(1024)
                partRecs = str(f.read(512 * 32))
            
                for i in range(0, 128):
                    p = GptRecord(partRecs, i)
                    if not p.empty:
                        p.printPart()

if __name__ == "__main__":
    main()


