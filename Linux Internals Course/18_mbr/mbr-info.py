#!/usr/bin/python2

import sys

PARTITION_TYPES = {0: 'Empty', 5: 'Extended', 15: 'Extended Int13', 7: 'NTFS', 11: 'FAT32', 131: 'Linux', 130: 'Linux Swap'}


class PriPart:
    def __init__(self, mbr_data, pid, base_lba=0):
        self.pid = pid
        self.mbr_data = mbr_data
        # this value is in number of sectors
        self.base_lba = base_lba
        self.type = 0
        self.lba = 0
        self.size = 0
        # this one is used for extended partitions
        self.relative_lba = 0
        self.ready = False

    def __calculate_value(self, data, offset):
        return ord(self.mbr_data[offset]) + 256 * ord(self.mbr_data[offset + 1]) \
               + 256 * 256 * ord(self.mbr_data[offset + 2]) + 256 * 256 * 256 * ord(self.mbr_data[offset + 3])

    def extract_information(self):
        if self.ready :
            return
        start_offset = 16 * self.pid + 446
        # making sure we don't exceed data boundary
        if start_offset + 15 >= len(self.mbr_data):
            return

        self.active = 'yes' if ord(self.mbr_data[start_offset])==128 else 'no'
        self.type = ord(self.mbr_data[start_offset + 4])
        # this part is for extended partitions and show lba from previous logical partition
        self.relative_lba = self.__calculate_value(self.mbr_data, start_offset + 8)
        # getting lba in number of sectors
        self.lba = self.base_lba + self.relative_lba
        # getting partition size in number of sectors
        self.size = self.__calculate_value(self.mbr_data, start_offset + 12)
        self.ready = True

    def print_information(self):
        self.extract_information()
        print("Partition Type Value: {0}".format(self.type))
        print("Partition Type Name: {0} ".format(PARTITION_TYPES[self.type]))
        print("Partition start sector: {0}".format(self.lba))
        print("Partition size in sectors: {0}".format(self.size))
        print ("Active partition: {0}\n".format(self.active))


def parse_extended_partitions(file, extended_lba):
    while True:
        # going at the beginning of EBR
        file.seek(extended_lba * 512)
        ebr = file.read(512)
        current_partition = PriPart(ebr, 0, extended_lba)
        next_partition = PriPart(ebr, 1, extended_lba)

        current_partition.print_information()
        next_partition.extract_information()

        # if the second partition info is empty it means we reached end of the linked list
        if next_partition.type == 0:
            break
        extended_lba += next_partition.relative_lba


if __name__ == '__main__':
    with open(sys.argv[1], 'rb') as f:
        data = f.read(512)
        for i in range(0, 4):
            partition = PriPart(data, i)
            partition.extract_information()

            if partition.type != 0:
                partition.print_information()
            # checking extended partition
            if partition.type == 15 or partition.type == 5:
                parse_extended_partitions(f, partition.lba)


