/*
 * fimg/global.c
 *
 * SAMSUNG S3C6410 FIMG-3DSE GLOBAL BLOCK RELATED FUNCTIONS
 *
 * Copyrights:	2010 by Tomasz Figa < tomasz.figa at gmail.com >
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "fimg_private.h"
#include <unistd.h>

/*
 * Global hardware
 */

/**
 * Makes sure that any rendering that is in progress finishes and any data
 * in caches is saved to buffers in memory.
 * @param ctx Hardware context.
 */
void fimgFinish(fimgContext *ctx)
{
	fimgWaitForFlush(ctx, FGHI_PIPELINE_ALL);
}
