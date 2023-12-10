#!/usr/bin/python2
# This is a simple Python script that will
# get metadata from an ext2/3/4 filesystem inside
# of an image file.

import sys
import os.path
import subprocess
import struct
import time
from math import log

# these are simple functions to make conversions easier
def getU32(data, offset=0):
    return struct.unpack('<L', data[offset:offset+4])[0]

def getU16(data, offset=0):
    return struct.unpack('<H', data[offset:offset+2])[0]

def getU8(data, offset=0):
    return struct.unpack('B', data[offset:offset+1])[0]

def getU64(data, offset=0):
    return struct.unpack('<Q', data[offset:offset+8])[0]

# this function doesn't unpack the string because it isn't really a number but a UUID
def getU128(data, offset=0):
    return data[offset:offset+16]

def printUuid(data):
    retStr = format(struct.unpack('<Q', data[8:16])[0], 'X').zfill(16) + \
        format(struct.unpack('<Q', data[0:8])[0], 'X').zfill(16)
    return retStr

def getCompatibleFeaturesList(u32):
    retList = []
    if u32 & 0x1:
        retList.append('Directory Preallocate')
    if u32 & 0x2:
        retList.append('Imagic Inodes')
    if u32 & 0x4:
        retList.append('Has Journal')
    if u32 & 0x8:
        retList.append('Extended Attributes')
    if u32 & 0x10:
        retList.append('Resize Inode')
    if u32 & 0x20:
        retList.append('Directory Index')
    if u32 & 0x40:
        retList.append('Lazy Block Groups')
    if u32 & 0x80:
        retList.append('Exclude Inode')
    if u32 & 0x100:
        retList.append('Exclude Bitmap')
    if u32 & 0x200:
        retList.append('Sparse Super 2')
    return retList

def getIncompatibleFeaturesList(u32):
    retList = []
    if u32 & 0x1:
        retList.append('Compression')
    if u32 & 0x2:
        retList.append('Filetype')
    if u32 & 0x4:
        retList.append('Recover')
    if u32 & 0x8:
        retList.append('Journal Device')
    if u32 & 0x10:
        retList.append('Meta Block Groups')
    if u32 & 0x40:
        retList.append('Extents')
    if u32 & 0x80:
        retList.append('64-bit')
    if u32 & 0x100:
        retList.append('Multiple Mount Protection')
    if u32 & 0x200:
        retList.append('Flexible Block Groups')
    if u32 & 0x400:
        retList.append('Extended Attributes in Inodes')
    if u32 & 0x1000:
        retList.append('Directory Data')
    if u32 & 0x2000:
        retList.append('Block Group Metadata Checksum')
    if u32 & 0x4000:
        retList.append('Large Directory')
    if u32 & 0x8000:
        retList.append('Inline Data')
    if u32 & 0x10000:
        retList.append('Encrypted Inodes')
    return retList

def getReadonlyCompatibleFeaturesList(u32):
    retList = []
    if u32 & 0x1:
        retList.append('Sparse Super')
    if u32 & 0x2:
        retList.append('Large File')
    if u32 & 0x4:
        retList.append('Btree Directory')
    if u32 & 0x8:
        retList.append('Huge File')
    if u32 & 0x10:
        retList.append('Group Descriptor Table Checksum')
    if u32 & 0x20:
        retList.append('Directory Nlink')
    if u32 & 0x40:
        retList.append('Extra Isize')
    if u32 & 0x80:
        retList.append('Has Snapshot')
    if u32 & 0x100:
        retList.append('Quota')
    if u32 & 0x200:
        retList.append('Big Alloc')
    if u32 & 0x400:
        retList.append('Metadata Checksum')
    if u32 & 0x800:
        retList.append('Replica')
    if u32 & 0x1000:
        retList.append('Read-only')
    return retList

# This class will parse the data in a superblock
class Superblock():
    def __init__(self, data):
        self.totalInodes = getU32(data)
        self.totalBlocks = getU32(data, 4)
        self.restrictedBlocks = getU32(data, 8)
        self.freeBlocks = getU32(data, 0xc)
        self.freeInodes = getU32(data, 0x10)
        self.firstDataBlock = getU32(data, 0x14) # normally 0 unless block size is <4k
        self.blockSize = pow(2, 10 + getU32(data, 0x18)) # block size is 1024 * 2^(whatever is in this field)
        self.clusterSize = pow(2, getU32(data, 0x1c)) # only used if bigalloc feature enabled
        self.blocksPerGroup = getU32(data, 0x20)
        self.clustersPerGroup = getU32(data, 0x24) # only used if bigalloc feature enabled
        self.inodesPerGroup = getU32(data, 0x28)
        self.mountTime = time.gmtime(getU32(data, 0x2c))
        self.writeTime = time.gmtime(getU32(data, 0x30))
        self.mountCount = getU16(data, 0x34) # mounts since last fsck
        self.maxMountCount = getU16(data, 0x36) # mounts between fsck
        self.magic = getU16(data, 0x38) # should be 0xef53
        self.state = getU16(data, 0x3a) #0001/0002/0004 = cleanly unmounted/errors/orphans
        self.errors = getU16(data, 0x3c) # when errors 1/2/3 continue/read-only/panic
        
        self.minorRevision = getU16(data, 0x3e)
        self.lastCheck = time.gmtime(getU32(data, 0x40)) # last fsck time
        self.checkInterval = getU32(data, 0x44) # seconds between checks
        self.creatorOs = getU32(data, 0x48) # 0/1/2/3/4 Linux/Hurd/Masix/FreeBSD/Lites
        self.revisionLevel = getU32(data, 0x4c) # 0/1 original/v2 with dynamic inode sizes
        self.defaultResUid = getU16(data, 0x50) # UID for reserved blocks
        self.defaultRegGid = getU16(data, 0x52) # GID for reserved blocks
        
        # for Ext4 dynamic revisionLevel superblocks only!
        self.firstInode = getU32(data, 0x54) # first non-reserved inode
        self.inodeSize = getU16(data, 0x58) # inode size in bytes
        self.blockGroupNumber = getU16(data, 0x5a) # block group this superblock is in
        self.compatibleFeatures = getU32(data, 0x5c) # compatible features
        self.compatibleFeaturesList = getCompatibleFeaturesList(self.compatibleFeatures)
        self.incompatibleFeatures = getU32(data, 0x60) #incompatible features
        self.incompatibleFeaturesList = getIncompatibleFeaturesList(self.incompatibleFeatures)
        self.readOnlyCompatibleFeatures = getU32(data, 0x64) # read-only compatible features
        self.readOnlyCompatibleFeaturesList = getReadonlyCompatibleFeaturesList(self.readOnlyCompatibleFeatures)
        self.uuid = getU128(data, 0x68) #UUID for volume left as a packed string    
        self.volumeName = data[0x78:0x88].split("\x00")[0] # volume name - likely empty
        self.lastMounted = data[0x88:0xc8].split("\x00")[0] # directory where last mounted
        self.algorithmUsageBitmap = getU32(data, 0xc8) # used with compression
        self.preallocBlocks = getU8(data, 0xcc) # not used in ext4
        self.preallocDirBlock = getU8(data, 0xcd) #only used with DIR_PREALLOC feature
        self.reservedGdtBlocks = getU16(data, 0xce) # blocks reserved for future expansion
        self.journalUuid = getU128(data, 0xd0) # UUID of journal superblock
        self.journalInode = getU32(data, 0xe0) # inode number of journal file
        self.journalDev = getU32(data, 0xe4) # device number for journal if external journal used
        self.lastOrphan = getU32(data, 0xe8) # start of list of orphaned inodes to delete
        self.hashSeed = []
        self.hashSeed.append(getU32(data, 0xec)) # htree hash seed
        self.hashSeed.append(getU32(data, 0xf0))
        self.hashSeed.append(getU32(data, 0xf4))
        self.hashSeed.append(getU32(data, 0xf8))
        self.hashVersion = getU8(data, 0xfc) # 0/1/2/3/4/5 legacy/half MD4/tea/u-legacy/u-half MD4/u-Tea
        self.journalBackupType = getU8(data, 0xfd) 
        self.descriptorSize = getU16(data, 0xfe) # group descriptor size if 64-bit feature enabled
        self.defaultMountOptions = getU32(data, 0x100) 
        self.firstMetaBlockGroup = getU32(data, 0x104) # only used with meta bg feature
        self.mkfsTime = time.gmtime(getU32(data, 0x108)) # when was the filesystem created
        self.journalBlocks = []
    
        for i in range(0, 17): # backup copy of journal inodes and size in last two elements
            self.journalBlocks.append(getU32(data, 0x10c + i*4))
      
        # for 64-bit mode only
        self.blockCountHi = getU32(data, 0x150)
        self.reservedBlockCountHi = getU32(data, 0x154)
        self.freeBlocksHi = getU32(data, 0x158)
        self.minInodeExtraSize = getU16(data, 0x15c) # all inodes such have at least this much space
        self.wantInodeExtraSize = getU16(data, 0x15e) # new inodes should reserve this many bytes
        self.miscFlags = getU32(data, 0x160) #1/2/4 signed hash/unsigned hash/test code
        self.raidStride = getU16(data, 0x164) # logical blocks read from disk in RAID before moving to next disk
        self.mmpInterval = getU16(data, 0x166) # seconds to wait between multi-mount checks
        self.mmpBlock = getU64(data, 0x168) # block number for MMP data
        self.raidStripeWidth = getU32(data, 0x170) # how many blocks read/write till back on this disk
        self.groupsPerFlex = pow(2, getU8(data, 0x174)) # groups per flex group
        self.metadataChecksumType = getU8(data, 0x175) # should be 1 for crc32
        self.reservedPad = getU16(data, 0x176) # should be zeroes
        self.kilobytesWritten = getU64(data, 0x178) # kilobytes written for all time
        self.snapshotInode = getU32(data, 0x180) # inode of active snapshot
        self.snapshotId = getU32(data, 0x184) # id of the active snapshot
        self.snapshotReservedBlocks = getU64(data, 0x188) # blocks reserved for snapshot
        self.snapshotList = getU32(data, 0x190) # inode number of head of snapshot list
        self.errorCount = getU32(data, 0x194)
        self.firstErrorTime = time.gmtime(getU32(data, 0x198)) # time first error detected
        self.firstErrorInode = getU32(data, 0x19c) # guilty inode
        self.firstErrorBlock = getU64(data, 0x1a0) # guilty block
        self.firstErrorFunction = data[0x1a8:0x1c8].split("\x00")[0] # guilty function
        self.firstErrorLine = getU32(data, 0x1c8) # line number where error occurred
        self.lastErrorTime = time.gmtime(getU32(data, 0x1cc)) # time last error detected
        self.lastErrorInode = getU32(data, 0x1d0) # guilty inode
        self.lastErrorLine = getU32(data, 0x1d4) # line number where error occurred
        self.lastErrorBlock = getU64(data, 0x1d8) # guilty block
        self.lastErrorFunction = data[0x1e0:0x200].split("\x00")[0] # guilty function
        self.mountOptions = data[0x200:0x240].split("\x00")[0] # mount options in null-terminated string
        self.userQuotaInode = getU32(data, 0x240) # inode of user quota file
        self.groupQuotaInode = getU32(data, 0x244) # inode of group quota file
        self.overheadBlocks = getU32(data, 0x248) # should be zero
        self.backupBlockGroups = [getU32(data, 0x24c), getU32(data, 0x250)] # super sparse 2 only
        self.encryptionAlgorithms = []
    
        for i in range(0, 4):
            self.encryptionAlgorithms.append(getU32(data, 0x254 + i*4))
        self.checksum = getU32(data, 0x3fc)

    def blockGroups(self):
        bg = self.totalBlocks / self.blocksPerGroup
        if self.totalBlocks % self.blocksPerGroup != 0:
            bg += 1
        return bg

    def groupDescriptorSize(self):
        if '64-bit' in self.incompatibleFeaturesList:
            return 64
        else:
            return 32

    def printState(self):
        #0001/0002/0004 = cleanly unmounted/errors/orphans
        retVal = "Unknown"
        if self.state == 1:
            retVal = "Cleanly unmounted"
        elif self.state == 2:
            retVal = "Errors detected"
        elif self.state == 4:
            retVal = "Orphans being recovered"
        return retVal

    def printErrorBehavior(self):
        # when errors 1/2/3 continue/read-only/panic
        retVal = "Unknown"
        if self.errors == 1:
            retVal = "Continue"
        elif self.errors == 2:
            retVal = "Remount read-only"
        elif self.errors == 3:
            retVal = "Kernel panic"
        return retVal

    def printCreator(self):
        # 0/1/2/3/4 Linux/Hurd/Masix/FreeBSD/Lites
        retVal = "Unknown"
        if self.creatorOs == 0:
            retVal = "Linux"
        elif self.creatorOs == 1:
            retVal = "Hurd"
        elif self.creatorOs == 2:
            retVal = "Masix"
        elif self.creatorOs == 3:
            retVal = "FreeBSD"
        elif self.creatorOs == 4:
            retVal = "Lites"
        return retVal
 
    def printHashAlgorithm(self):
        # 0/1/2/3/4/5 legacy/half MD4/tea/u-legacy/u-half MD4/u-Tea 
        retVal = "Unknown"   
        if self.hashVersion == 0:  
            retVal = "Legacy"
        elif self.hashVersion == 1:
            retVal = "Half MD4"
        elif self.hashVersion == 2:
            retVal = "Tea"
        elif self.hashVersion == 3:
            retVal = "Unsigned Legacy"
        elif self.hashVersion == 4:
            retVal = "Unsigned Half MD4"
        elif self.hashVersion == 5:
            retVal = "Unsigned Tea"
        return retVal 

    def printEncryptionAlgorithms(self):
        encList = []
        for v in self.encryptionAlgorithms:
            if v == 1:
                encList.append('256-bit AES in XTS mode')
            elif v == 2:
                encList.append('256-bit AES in GCM mode')
            elif v == 3:
                encList.append('256-bit AES in CBC mode')
            elif v == 0:
                pass
            else:
                encList.append('Unknown')
        return encList

    def prettyPrint(self):
        for k, v in sorted(self.__dict__.iteritems()) :
            if k == 'mountTime' or k == 'writeTime' or \
                k == 'lastCheck' or k == 'mkfsTime' or \
                k == 'firstErrorTime' or k == 'lastErrorTime' :
                print k+":", time.asctime(v)
            elif k == 'state':
                print k+":", self.printState()
            elif k == 'errors':
                print k+":", self.printErrorBehavior()
            elif k == 'uuid' or k == 'journalUuid':
                print k+":", printUuid(v)
            elif k == 'creatorOs':
                print k+":", self.printCreator()
            elif k == 'hashVersion':
                print k+":", self.printHashAlgorithm()
            elif k == 'encryptionAlgorithms':
                print k+":", self.printEncryptionAlgorithms()
            else:
                print k+":", v

    def groupStartBlock(self, bgNo):
        return self.blocksPerGroup * bgNo

    def groupEndBlock(self, bgNo):
        return self.groupStartBlock(bgNo + 1) - 1  

    def groupStartInode(self, bgNo):
        return self.inodesPerGroup * bgNo + 1
 
    def groupEndInode(self, bgNo):
        return self.inodesPerGroup * (bgNo + 1)      

    def groupFromBlock(self, blockNo):
        return blockNo / self.blocksPerGroup

    def groupIndexFromBlock(self, blockNo):
        return blockNo % self.blocksPerGroup

    def groupFromInode(self, inodeNo):
        return (inodeNo - 1) / self.inodesPerGroup
                  
    def groupIndexFromInode(self, inodeNo):
        return (inodeNo - 1) % self.inodesPerGroup

    def groupHasSuperblock(self, bgNo):
        # block group zero always has a superblock
        if bgNo == 0:
            return True
        retVal = False
    
        if 'Sparse Super 2' in self.compatibleFeaturesList: 
            # two backup superblocks in self.backupBlockGroups
            if bgNo == self.backupBlockGroups[0] or bgNo == self.backupBlockGroups[1]:
                retVal = True
        elif 'Sparse Super' in self.readOnlyCompatibleFeaturesList:
            # backups in 1, powers of 3, 5, and 7
            retVal = (bgNo == 1) or (bgNo == pow(3, round(log(bgNo) / log(3)))) \
                    or (bgNo == pow(5, round(log(bgNo) / log(5)))) \
                    or (bgNo == pow(7, round(log(bgNo) / log(7)))) 
        if retVal:
            return retVal
        elif 'Meta Block Groups' in self.incompatibleFeaturesList:
            # meta block groups have a sb and gdt in 1st and last two of each meta group
            # meta block group size is blocksize/32
            # only part of filesystem might use this feature
            if bgNo >= self.firstMetaBlockGroup:
                mbgSize = self.blockSize / 32
                retVal = (bgNo % mbgSize == 0) or ((bgNo + 1) % mbgSize == 0) or ((bgNo + 2) % mbgSize == 0)
            else:
                # if we got this far we must have default with every bg having sb and gdt
                retVal = True
        return retVal

class GroupDescriptor():
    def __init__(self, data, wide=False):
        self.wide = wide
        self.blockBitmapLo=getU32(data)	        #/* Blocks bitmap block */
        self.inodeBitmapLo=getU32(data, 4)	    #/* Inodes bitmap block */
        self.inodeTableLo=getU32(data, 8)	    #/* Inodes table block */
        self.freeBlocksCountLo=getU16(data, 0xc)#/* Free blocks count */
        self.freeInodesCountLo=getU16(data, 0xe)#/* Free inodes count */
        self.usedDirsCountLo=getU16(data, 0x10)	#/* Directories count */
        self.flags=getU16(data, 0x12)		    #/* EXT4_BG_flags (INODE_UNINIT, etc) */
        self.flagList = self.printFlagList()
        self.excludeBitmapLo=getU32(data, 0x14)     #/* Exclude bitmap for snapshots */
        self.blockBitmapCsumLo=getU16(data, 0x18)       #/* crc32c(s_uuid+grp_num+bbitmap) LE */
        self.inodeBitmapCsumLo=getU16(data, 0x1a)       #/* crc32c(s_uuid+grp_num+ibitmap) LE */
        self.itableUnusedLo=getU16(data, 0x1c)	        #/* Unused inodes count */
        self.checksum=getU16(data, 0x1e)		        #/* crc16(sb_uuid+group+desc) */
    
        if wide==True:      	
            self.blockBitmapHi=getU32(data, 0x20)	    #/* Blocks bitmap block MSB */
            self.inodeBitmapHi=getU32(data, 0x24)	    #/* Inodes bitmap block MSB */
            self.inodeTableHi=getU32(data, 0x28)	    #/* Inodes table block MSB */
            self.freeBlocksCountHi=getU16(data, 0x2c) #/* Free blocks count MSB */
            self.freeInodesCountHi=getU16(data, 0x2e) #/* Free inodes count MSB */
            self.usedDirsCountHi=getU16(data, 0x30)	#/* Directories count MSB */
            self.itableUnusedHi=getU16(data, 0x32)    #/* Unused inodes count MSB */
            self.excludeBitmapHi=getU32(data, 0x34)   #/* Exclude bitmap block MSB */
            self.blockBitmapCsumHi=getU16(data, 0x38) #/* crc32c(s_uuid+grp_num+bbitmap) BE */
            self.inodeBitmapCsumHi=getU16(data, 0x3a) #/* crc32c(s_uuid+grp_num+ibitmap) BE */
            self.reserved=getU32(data, 0x3c)

    def printFlagList(self):
        flagList = []
        if self.flags & 0x1: #inode table and bitmap are not initialized (EXT4_BG_INODE_UNINIT).
            flagList.append('Inode Uninitialized')
        if self.flags & 0x2: #block bitmap is not initialized (EXT4_BG_BLOCK_UNINIT).
            flagList.append('Block Uninitialized')
        if self.flags & 0x4: #inode table is zeroed (EXT4_BG_INODE_ZEROED).
            flagList.append('Inode Zeroed')
        return flagList

    def prettyPrint(self):
        for k, v in sorted(self.__dict__.iteritems()) :
            print k+":", v

# This class combines informaton from the block group descriptor
# and the superblock to more fully describe the block group
class ExtendedGroupDescriptor():
    def __init__(self, bgd, sb, bgNo):
        self.blockGroup = bgNo
        self.startBlock = sb.groupStartBlock(bgNo)
        self.endBlock = sb.groupEndBlock(bgNo)
        self.startInode = sb.groupStartInode(bgNo)
        self.endInode = sb.groupEndInode(bgNo)
        self.flags = bgd.printFlagList()
        self.freeInodes = bgd.freeInodesCountLo
    
        if bgd.wide:
            self.freeInodes += bgd.freeInodesCountHi * pow(2, 16)
        self.freeBlocks = bgd.freeBlocksCountLo
        if bgd.wide:
            self.freeBlocks += bgd.freeBlocksCountHi * pow(2, 16)
        self.directories = bgd.usedDirsCountLo
        if bgd.wide:
            self.directories += bgd.usedDirsCountHi * pow(2, 16)
        self.checksum = bgd.checksum
        self.blockBitmapChecksum = bgd.blockBitmapCsumLo
        if bgd.wide:
            self.blockBitmapChecksum += bgd.blockBitmapCsumHi * pow(2, 16)
        self.inodeBitmapChecksum = bgd.inodeBitmapCsumLo
        if bgd.wide:
            self.inodeBitmapChecksum += bgd.inodeBitmapCsumHi * pow(2, 16)
            
        # now figure out the layout and store it in a list (with lists inside)
        self.layout = []
        self.nonDataBlocks = 0
        # for flexible block groups must make an adjustment
        fbgAdj = 1
        if 'Flexible Block Groups' in sb.incompatibleFeaturesList:
            if bgNo % sb.groupsPerFlex == 0: # only first group in flex block affected
                fbgAdj = sb.groupsPerFlex   
        if sb.groupHasSuperblock(bgNo):
            self.layout.append(['Superblock', self.startBlock, self.startBlock])
            gdSize = sb.groupDescriptorSize() * sb.blockGroups() / sb.blockSize
            self.layout.append(['Group Descriptor Table', self.startBlock + 1, self.startBlock + gdSize])
            self.nonDataBlocks += gdSize + 1
            if sb.reservedGdtBlocks > 0:
                self.layout.append(['Reserved GDT Blocks', self.startBlock + gdSize + 1, \
                                    self.startBlock + gdSize + sb.reservedGdtBlocks])
            self.nonDataBlocks += sb.reservedGdtBlocks
        
        bbm = bgd.blockBitmapLo
        if bgd.wide:
            bbm += bgd.blockBitmapHi * pow(2, 32)
            
        self.layout.append(['Data Block Bitmap', bbm, bbm])
        # is block bitmap in this group (not flex block group, etc) 
        if sb.groupFromBlock(bbm) == bgNo:
            self.nonDataBlocks += fbgAdj
        ibm = bgd.inodeBitmapLo
        if bgd.wide:
            ibm += bgd.inodeBitmapHi * pow(2, 32)
        self.layout.append(['Inode Bitmap', ibm, ibm])
        # is inode bitmap in this group?
        if sb.groupFromBlock(ibm) == bgNo:
            self.nonDataBlocks += fbgAdj 
        it = bgd.inodeTableLo
    
        if bgd.wide:
            it += bgd.inodeTableHi * pow(2, 32)
        itBlocks = (sb.inodesPerGroup * sb.inodeSize) / sb.blockSize
        self.layout.append(['Inode Table', it, it + itBlocks - 1])
        # is inode table in this group?
        if sb.groupFromBlock(it) == bgNo:
            self.nonDataBlocks += itBlocks * fbgAdj
        self.layout.append(['Data Blocks', self.startBlock + self.nonDataBlocks, self.endBlock])

    def prettyPrint(self):
        print ""
        print 'Block Group: ' + str(self.blockGroup)
        print 'Flags: %r ' % self.flags
        print 'Blocks: %s - %s ' % (self.startBlock, self.endBlock)
        print 'Inodes: %s - %s ' % (self.startInode, self.endInode)
        print 'Layout:'
        for item in self.layout:
            print '   %s %s - %s' % (item[0], item[1], item[2])
        print 'Free Inodes: %u ' % self.freeInodes
        print 'Free Blocks: %u ' % self.freeBlocks
        print 'Directories: %u ' % self.directories
        print 'Checksum: 0x%x ' % self.checksum
        print 'Block Bitmap Checksum: 0x%x ' % self.blockBitmapChecksum
        print 'Inode Bitmap Checksum: 0x%x ' % self.inodeBitmapChecksum

class ExtMetadata():
    def __init__(self, filename, offset):
        # read first sector
        if not os.path.isfile(sys.argv[1]):
            print("File " + str(filename) + " cannot be openned for reading")
            exit(1)
        with open(str(filename), 'rb') as f:
            f.seek(1024 + int(offset) * 512)
            sbRaw = str(f.read(1024))
        self.superblock = Superblock(sbRaw)  

        # read block group descriptors
        self.blockGroups = self.superblock.blockGroups()
        if self.superblock.descriptorSize != 0:
            self.wideBlockGroups = True
            self.blockGroupDescriptorSize = 64
        else:
            self.wideBlockGroups = False
            self.blockGroupDescriptorSize = 32
        # read in group descriptors starting in block 1
        with open(str(filename), 'rb') as f:
            f.seek(int(offset) * 512 + self.superblock.blockSize)
            bgdRaw = str(f.read(self.blockGroups * self.blockGroupDescriptorSize))
  
        self.bgdList = []
        for i in range(0, self.blockGroups):
            bgd = GroupDescriptor(bgdRaw[i * self.blockGroupDescriptorSize:], self.wideBlockGroups)
            ebgd = ExtendedGroupDescriptor(bgd, self.superblock, i) 
            self.bgdList.append(ebgd)

    def prettyPrint(self):
        self.superblock.prettyPrint()
        for bgd in self.bgdList:
            bgd.prettyPrint()
      
def usage():
    print("usage " + sys.argv[0] + " <image file> <offset in sectors>\nReads superblock from an image file")
    exit(1)

def main():
    if len(sys.argv) < 3: 
        usage()

    # read first sector
    if not os.path.isfile(sys.argv[1]):
        print("File " + sys.argv[1] + " cannot be openned for reading")
        exit(1)
    emd = ExtMetadata(sys.argv[1], sys.argv[2])
    emd.prettyPrint()

if __name__ == "__main__":
    main()

