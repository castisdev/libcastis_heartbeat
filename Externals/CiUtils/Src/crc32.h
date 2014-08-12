#ifndef __CRC32_H_
#define __CRC32_H_

void crc32_initialize();

/* update the CRC on the data block one byte at a time */
unsigned long crc32_update_crc(uint32_t crc_accum,
							    char *data_blk_ptr,
								int  data_blk_size);

#endif // __CRC32_H_
