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

  int bit = operator(0, 0, 0);

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
  int bit = operator(0, 0, 1);

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
  // Complete your code here
  return 0;
}
