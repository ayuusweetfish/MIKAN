#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "common.h"

DSTATUS disk_status(BYTE pdrv)
{
    return 0;
}

DSTATUS disk_initialize(BYTE pdrv)
{
    // Initialization could also be done here
    // instead of in kernel_main()
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    int32_t ret = sdTransferBlocks((uint64_t)sector * FF_MIN_SS, count, buff, 0);
    return (ret == 0 ? RES_OK : RES_ERROR);
}
