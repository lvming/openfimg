/*
 * libsgl/eglMem.cpp
 *
 * SAMSUNG S3C6410 FIMG-3DSE (PROPER) EGL IMPLEMENTATION
 *
 * Copyrights:	2010 by Tomasz Figa < tomasz.figa at gmail.com >
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty off
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <linux/android_pmem.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "eglCommon.h"
#include "platform.h"
#include "fglsurface.h"
#include "common.h"
#include "types.h"
#include "state.h"
#include "libfimg/fimg.h"

/*
 * Surfaces
 */

bool FGLSurface::bindContext(FGLContext *ctx)
{
	int ret;

	if (ctx)
		return false;

	if (fd < 0 || !allocate(ctx))
		return false;

	ret = fimgImportGEM(ctx->fimg, fd, &handle);
	if (ret)
		return false;

	vaddr = fimgMapGEM(ctx->fimg, handle, size);
	if (!vaddr)
		return false;

	this->ctx = ctx;

	return true;
}

void FGLSurface::unbindContext(void)
{
	if (!ctx)
		return;

	munmap(vaddr, size);
	fimgDestroyGEMHandle(ctx->fimg, handle);

	ctx = 0;
	handle = 0;
}

FGLLocalSurface::FGLLocalSurface(unsigned long req_size)
{
	size = req_size;
}

FGLLocalSurface::~FGLLocalSurface()
{
	if (!isValid())
		return;

	fimgDestroyGEMHandle(ctx->fimg, handle);
}

bool FGLLocalSurface::allocate(FGLContext *ctx)
{
	int ret;

	ret = fimgCreateGEM(ctx->fimg, size, &handle);
	if (ret < 0)
		return false;

	ret = fimgExportGEM(ctx->fimg, handle);
	if (ret < 0) {
		fimgDestroyGEMHandle(ctx->fimg, handle);
		return false;
	}

	fd = ret;
	return true;
}

int FGLLocalSurface::lock(int usage)
{
	return 0;
}

int FGLLocalSurface::unlock(void)
{
	return 0;
}

void FGLLocalSurface::flush(void)
{

}

FGLExternalSurface::FGLExternalSurface(void *v, uint32_t h, off_t o, size_t s) :
	FGLSurface(h, o, s)
{

}

FGLExternalSurface::~FGLExternalSurface()
{

}

int FGLExternalSurface::lock(int usage)
{
	return 0;
}

int FGLExternalSurface::unlock(void)
{
	return 0;
}

void FGLExternalSurface::flush(void)
{

}
