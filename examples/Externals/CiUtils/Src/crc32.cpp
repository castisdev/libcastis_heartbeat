#include "internal_CiUtils.h"
#include "crc32.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#define POLYNOMIAL 0x04c11db7L

static uint32_t crc_table[256];

static void gen_crc_table(void)
{
	int			i, j;
	uint32_t	crc_accum;

	for ( i = 0; i < 256; i++ ) {
		crc_accum = ((uint32_t)i << 24);
		for (j = 0;  j < 8; j++) {
			if (crc_accum & 0x80000000L) {
				crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
			}
			else {
				crc_accum = (crc_accum << 1);
			}
		}
		crc_table[i] = crc_accum;
	}
}

void crc32_initialize()
{
	gen_crc_table();
}

/* update the CRC on the data block one byte at a time */
unsigned long crc32_update_crc(uint32_t crc_accum,
							    char *data_blk_ptr,
								int  data_blk_size)
{
	int i, j;

	for (j = 0; j < data_blk_size; j++) {
		i = ((uint32_t)(crc_accum >> 24) ^ *data_blk_ptr++) & 0xff;
		crc_accum = (crc_accum << 8) ^ crc_table[i];
	}

	return crc_accum;
}

