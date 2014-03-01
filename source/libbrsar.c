/*! @file libbrsar.c
 *  @brief offsetFromString for making stuff easier.
 *  @since 2014-02-14
 *
 *  This file contains the offsetFromString function, which converts the given
 *  filename to its respective offset in the BRSAR.
 */
#include "libbrsar.h"
#include <string.h>

/*! @fn int offsetFromString(char* str)
 *  @brief Converts filename to offset.
 *  @param str Filename to convert.
 *  @since 1.2
 *  @return -1 on failure, otherwise the offset converted from the filename.
 */
int offsetFromString(char* str) {
	if (strcmp("N_BLOCK_F", str)==0)
		return N_BLOCK_F;
	if (strcmp("N_BLOCK_N", str)==0)
		return N_BLOCK_N;
	if (strcmp("N_BOSSMI_32", str)==0)
		return N_BOSSMI_32;
	if (strcmp("N_CASINO_F", str)==0)
		return N_CASINO_F;
	if (strcmp("N_CASINO_N", str)==0)
		return N_CASINO_N;
	if (strcmp("N_CIRCUIT32_F", str)==0)
		return N_CIRCUIT32_F;
	if (strcmp("N_CIRCUIT32_N", str)==0)
		return N_CIRCUIT32_N;

	return -1;
}
