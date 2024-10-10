#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mdadm.h"
#include "jbod.h"

static int MOUNT_STATUS = 0;
int operator(int DiskID, int BlockID, int Command)
{
  int op = DiskID | BlockID << 4 | Command << 12;
  return op;
}

int mdadm_mount(void)
{

  int bit = operator(0, 0, JBOD_MOUNT);

  if (MOUNT_STATUS == 0)
  {
    if (jbod_operation(bit, NULL) == 0)
    {
      MOUNT_STATUS++;
      return 1;
    }
    else
    {
      return -1;
    }
  }
  else
  {
    return -1;
  }
}

int mdadm_unmount(void)
{
  // Complete your code here
  int bit = operator(0, 0, JBOD_UNMOUNT);

  if (MOUNT_STATUS == 1)
  {
    if (jbod_operation(bit, NULL) == 0)
    {
      MOUNT_STATUS--;
      return 1;
    }
    else
    {
      return -1;
    }
  }
  else
  {
    return -1;
  }
}

int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf)
{
  int TotalSize = JBOD_NUM_DISKS * JBOD_BLOCK_SIZE * JBOD_NUM_BLOCKS_PER_DISK;
  uint8_t tmp_buf[JBOD_BLOCK_SIZE];
  uint32_t remaining_read_len = read_len;
  uint32_t TotalLenReadCounter = 0;

  if (start_addr + read_len > TotalSize)
  {
    return -1;
  }
  if (read_len > 1024)
  {
    return -2;
  }
  if (MOUNT_STATUS == 0)
  {
    return -3;
  }
  if (read_len > 0 && read_buf == NULL)
  {
    return -4;
  }

  while (remaining_read_len > 0)
  {
    uint32_t DiskID = start_addr / JBOD_DISK_SIZE;
    uint32_t BlockID = (start_addr % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
    uint32_t Offset = (start_addr % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE;

    int bit = operator(DiskID, 0, JBOD_SEEK_TO_DISK);

    jbod_operation(bit, NULL);

    bit = operator(0, BlockID, JBOD_SEEK_TO_BLOCK);

    jbod_operation(bit, NULL);

    bit = operator(0, 0, JBOD_READ_BLOCK);

    jbod_operation(bit, tmp_buf);

    int size = JBOD_BLOCK_SIZE - Offset;
    int size2 = remaining_read_len;
    int FinalSize;

    if (size2 >= size)
    {
      FinalSize = size;
    }
    if (size >= size2)
    {
      FinalSize = size2;
    }

    memcpy((read_buf + TotalLenReadCounter), (tmp_buf + Offset), FinalSize);

    TotalLenReadCounter += FinalSize;

    start_addr += FinalSize;

    remaining_read_len -= FinalSize;
  }

  return read_len;
}
