#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mdadm.h"
#include "jbod.h"

//Holds the Mount Status, 1 if Mounted, 0 if Unmounted
static int MOUNT_STATUS = 0;

//Helper function to control the jbod operation
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
      MOUNT_STATUS++; //MOUNT Successful
      return 1;
    }
    else
    {
      return -1; //MOUNT Unsuccessful
    }
  }
  else
  {
    return -1; //MOUNT called while already mounted (error)
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
      MOUNT_STATUS--; //UNMOUNT Successful
      return 1; 
    }
    else
    {
      return -1; //UNMOUNT Unsuccessful
    }
  }
  else
  {
    return -1; //UNMOUNT called while already mounted (error)
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
    return -1; //Out of bounds
  }
  if (read_len > 1024)
  {
    return -2; //Exceeds Maximum read length
  }
  if (MOUNT_STATUS == 0)
  {
    return -3; //Unmounted
  }
  if (read_len > 0 && read_buf == NULL)
  {
    return -4; //Invalid Buffer or read_len invalid length
  }
  
  while (remaining_read_len > 0)
  {
    
    uint32_t DiskID = start_addr / JBOD_DISK_SIZE;
    uint32_t BlockID = (start_addr % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
    uint32_t Offset = (start_addr % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE;

    //Seek Correct Disk
    int bit = operator(DiskID, 0, JBOD_SEEK_TO_DISK); // SEEK DISK BIT (update bit)
    if (jbod_operation(bit, NULL) == -1)
    {
      return -4; //Seek Disk Failed
    }

    //Seek Correct Block
    bit = operator(0, BlockID, JBOD_SEEK_TO_BLOCK); // SEEK BLOCK BIT (update bit)
    if (jbod_operation(bit, NULL) == -1)
    {
      return -4; //Seek Block Failed
    }

    //Read Block into tmp_buf
    bit = operator(0, 0, JBOD_READ_BLOCK); // READ BLOCK BIT (update bit)
    if (jbod_operation(bit, tmp_buf) == -1)
    {
      return -4; //Read Block Failed
    }

    //Decide number of bits needed to be read
    int readSize = JBOD_BLOCK_SIZE - Offset;
    int FinalSize;

    if (remaining_read_len >= readSize)
    {
      FinalSize = readSize;
    }
    if (readSize >= remaining_read_len)
    {
      FinalSize = remaining_read_len;
    }

    //copy from tmp_buf to read_buf
    memcpy((read_buf + TotalLenReadCounter), (tmp_buf + Offset), FinalSize);

    TotalLenReadCounter += FinalSize;

    start_addr += FinalSize;

    remaining_read_len -= FinalSize;
  }

  return read_len;
}
