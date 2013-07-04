/*
 * fimg/system.c
 *
 * SAMSUNG S3C6410 FIMG-3DSE SYSTEM-DEVICE INTERFACE
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "fimg_private.h"

/**
 * Opens G3D device and maps GPU registers into application address space.
 * @param ctx Hardware context.
 * @return 0 on success, negative on error.
 */
int fimgDeviceOpen(fimgContext *ctx)
{
	ctx->fd = open("/dev/dri/card0", O_RDWR | O_SYNC, 0);
	if(ctx->fd < 0) {
		LOGE("Couldn't open /dev/dri/card0 (%s).", strerror(errno));
		return -errno;
	}

	LOGD("Opened /dev/dri/card0 (%d).", ctx->fd);

	return 0;
}

/**
 * Unmaps GPU registers and closes G3D device.
 * @param ctx Hardware context.
 */
void fimgDeviceClose(fimgContext *ctx)
{
	close(ctx->fd);

	LOGD("fimg3D: Closed /dev/dri/card0 (%d).", ctx->fd);
}

/**
	Context management
*/

/**
 * Creates a hardware context.
 * @return A pointer to hardware context struct or NULL on error.
 */
fimgContext *fimgCreateContext(void)
{
	fimgContext *ctx;
	uint32_t *queue;

	if ((ctx = malloc(sizeof(*ctx))) == NULL)
		return NULL;

	if ((queue = malloc(2*(FIMG_MAX_QUEUE_LEN + 1)*sizeof(uint32_t))) == NULL) {
		free(ctx);
		return NULL;
	}

	memset(ctx, 0, sizeof(fimgContext));

	if(fimgDeviceOpen(ctx)) {
		free(queue);
		free(ctx);
		return NULL;
	}

	fimgCreateGlobalContext(ctx);
	fimgCreateHostContext(ctx);
	fimgCreatePrimitiveContext(ctx);
	fimgCreateRasterizerContext(ctx);
	fimgCreateFragmentContext(ctx);
#ifdef FIMG_FIXED_PIPELINE
	fimgCreateCompatContext(ctx);
#endif

	ctx->queue = queue;
	ctx->queue[0] = 0;
	ctx->queueStart = queue;

	return ctx;
}

/**
 * Destroys a hardware context.
 * @param ctx Hardware context.
 */
void fimgDestroyContext(fimgContext *ctx)
{
	fimgDeviceClose(ctx);
	free(ctx->queueStart);
	free(ctx->vertexData);
#ifdef FIMG_FIXED_PIPELINE
	free(ctx->compat.vshaderBuf);
	free(ctx->compat.pshaderBuf);
#endif
	free(ctx);
}

/**
	Power management
*/

/**
 * Waits for hardware to flush graphics pipeline.
 * @param ctx Hardware context.
 * @param target Bit mask of pipeline parts to be flushed.
 * @return 0 on success, negative on error.
 */
int fimgWaitForFlush(fimgContext *ctx, uint32_t target)
{
#warning Unimplemented

	return 0;
}

/* Register queue */
void fimgQueueFlush(fimgContext *ctx)
{
	struct drm_exynos_g3d_submit submit;
	struct drm_exynos_g3d_request req;
	int ret;

	if (!ctx->queueLen)
		return;

	submit.requests = &req;
	submit.nr_requests = 1;

	/* Above the maximum length it's more effective to restore the whole
	 * context than just the changed registers */
	if (ctx->queueLen == FIMG_MAX_QUEUE_LEN) {
		req.type = G3D_REQUEST_STATE_INIT;
		req.data = &ctx->hw;
		req.length = sizeof(ctx->hw);

		ret = ioctl(ctx->fd, DRM_IOCTL_EXYNOS_G3D_SUBMIT, &submit);
		if (ret < 0)
			LOGE("G3D_REQUEST_STATE_INIT failed (%d)", ret);

		ctx->queueLen = 0;
		ctx->queue = ctx->queueStart;
		ctx->queue[0] = 0;

		return;
	}

	req.type = G3D_REQUEST_STATE_BUFFER;
	req.data = ctx->queueStart;
	req.length = ctx->queueLen * sizeof(*ctx->queueStart) * 2;

	ret = ioctl(ctx->fd, DRM_IOCTL_EXYNOS_G3D_SUBMIT, &submit);
	if (ret < 0)
		LOGE("G3D_REQUEST_STATE_INIT failed (%d)", ret);

	ctx->queueLen = 0;
	ctx->queue = ctx->queueStart;
	ctx->queue[0] = 0;
}

void fimgFlushContext(fimgContext *ctx)
{
	fimgQueueFlush(ctx);
#ifdef FIMG_FIXED_PIPELINE
	fimgCompatFlush(ctx);
#endif
}
