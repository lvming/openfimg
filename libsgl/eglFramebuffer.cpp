/*
 * libsgl/eglFramebuffer.cpp
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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <linux/fb.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "eglCommon.h"
#include "platform.h"
#include "fglrendersurface.h"
#include "fglimage.h"
#include "common.h"
#include "types.h"
#include "libfimg/fimg.h"
#include "fglsurface.h"

/*
 * Configurations available for direct rendering into frame buffer
 */

/*
 * In the lists below, attributes names MUST be sorted.
 * Additionally, all configs must be sorted according to
 * the EGL specification.
 */

/*
 * These configs can override the base attribute list
 * NOTE: when adding a config here, don't forget to update eglCreate*Surface()
 */

/* RGB 565 configs */

/** RGB565, no depth, no stencil configuration */
static const FGLConfigPair configAttributes0[] = {
	{ EGL_BUFFER_SIZE,     16 },
	{ EGL_ALPHA_SIZE,       0 },
	{ EGL_BLUE_SIZE,        5 },
	{ EGL_GREEN_SIZE,       6 },
	{ EGL_RED_SIZE,         5 },
	{ EGL_DEPTH_SIZE,       0 },
	{ EGL_STENCIL_SIZE,     0 },
};

/** RGB565, no depth, 8-bit stencil configuration */
static const FGLConfigPair configAttributes1[] = {
	{ EGL_BUFFER_SIZE,     16 },
	{ EGL_ALPHA_SIZE,       0 },
	{ EGL_BLUE_SIZE,        5 },
	{ EGL_GREEN_SIZE,       6 },
	{ EGL_RED_SIZE,         5 },
	{ EGL_DEPTH_SIZE,       0 },
	{ EGL_STENCIL_SIZE,     8 },
};

/** RGB565, 24-bit depth, no stencil configuration */
static const FGLConfigPair configAttributes2[] = {
	{ EGL_BUFFER_SIZE,     16 },
	{ EGL_ALPHA_SIZE,       0 },
	{ EGL_BLUE_SIZE,        5 },
	{ EGL_GREEN_SIZE,       6 },
	{ EGL_RED_SIZE,         5 },
	{ EGL_DEPTH_SIZE,      24 },
	{ EGL_STENCIL_SIZE,     0 },
};

/** RGB565, 24-bit depth, 8-bit stencil configuration */
static const FGLConfigPair configAttributes3[] = {
	{ EGL_BUFFER_SIZE,     16 },
	{ EGL_ALPHA_SIZE,       0 },
	{ EGL_BLUE_SIZE,        5 },
	{ EGL_GREEN_SIZE,       6 },
	{ EGL_RED_SIZE,         5 },
	{ EGL_DEPTH_SIZE,      24 },
	{ EGL_STENCIL_SIZE,     8 },
};

/* RGB 888 configs */

/** XRGB8888, no depth, no stencil configuration */
static const FGLConfigPair configAttributes4[] = {
	{ EGL_BUFFER_SIZE,     32 },
	{ EGL_ALPHA_SIZE,       0 },
	{ EGL_BLUE_SIZE,        8 },
	{ EGL_GREEN_SIZE,       8 },
	{ EGL_RED_SIZE,         8 },
	{ EGL_DEPTH_SIZE,       0 },
	{ EGL_STENCIL_SIZE,     0 },
};

/** XRGB8888, no depth, 8-bit stencil configuration */
static const FGLConfigPair configAttributes5[] = {
	{ EGL_BUFFER_SIZE,     32 },
	{ EGL_ALPHA_SIZE,       0 },
	{ EGL_BLUE_SIZE,        8 },
	{ EGL_GREEN_SIZE,       8 },
	{ EGL_RED_SIZE,         8 },
	{ EGL_DEPTH_SIZE,       0 },
	{ EGL_STENCIL_SIZE,     8 },
};

/** XRGB8888, 24-bit depth, no stencil configuration */
static const FGLConfigPair configAttributes6[] = {
	{ EGL_BUFFER_SIZE,     32 },
	{ EGL_ALPHA_SIZE,       0 },
	{ EGL_BLUE_SIZE,        8 },
	{ EGL_GREEN_SIZE,       8 },
	{ EGL_RED_SIZE,         8 },
	{ EGL_DEPTH_SIZE,      24 },
	{ EGL_STENCIL_SIZE,     0 },
};

/** XRGB8888, 24-bit depth, 8-bit stencil configuration */
static const FGLConfigPair configAttributes7[] = {
	{ EGL_BUFFER_SIZE,     32 },
	{ EGL_ALPHA_SIZE,       0 },
	{ EGL_BLUE_SIZE,        8 },
	{ EGL_GREEN_SIZE,       8 },
	{ EGL_RED_SIZE,         8 },
	{ EGL_DEPTH_SIZE,      24 },
	{ EGL_STENCIL_SIZE,     8 },
};

/* ARGB 8888 configs */

/** ARGB8888, no depth, no stencil configuration */
static const FGLConfigPair configAttributes8[] = {
	{ EGL_BUFFER_SIZE,     32 },
	{ EGL_ALPHA_SIZE,       8 },
	{ EGL_BLUE_SIZE,        8 },
	{ EGL_GREEN_SIZE,       8 },
	{ EGL_RED_SIZE,         8 },
	{ EGL_DEPTH_SIZE,       0 },
	{ EGL_STENCIL_SIZE,     0 },
};

/** ARGB8888, no depth, 8-bit stencil configuration */
static const FGLConfigPair configAttributes9[] = {
	{ EGL_BUFFER_SIZE,     32 },
	{ EGL_ALPHA_SIZE,       8 },
	{ EGL_BLUE_SIZE,        8 },
	{ EGL_GREEN_SIZE,       8 },
	{ EGL_RED_SIZE,         8 },
	{ EGL_DEPTH_SIZE,       0 },
	{ EGL_STENCIL_SIZE,     8 },
};

/** ARGB8888, 24-bit depth, no stencil configuration */
static const FGLConfigPair configAttributes10[] = {
	{ EGL_BUFFER_SIZE,     32 },
	{ EGL_ALPHA_SIZE,       8 },
	{ EGL_BLUE_SIZE,        8 },
	{ EGL_GREEN_SIZE,       8 },
	{ EGL_RED_SIZE,         8 },
	{ EGL_DEPTH_SIZE,      24 },
	{ EGL_STENCIL_SIZE,     0 },
};

/** ARGB8888, 24-bit depth, 8-bit stencil configuration */
static const FGLConfigPair configAttributes11[] = {
	{ EGL_BUFFER_SIZE,     32 },
	{ EGL_ALPHA_SIZE,       8 },
	{ EGL_BLUE_SIZE,        8 },
	{ EGL_GREEN_SIZE,       8 },
	{ EGL_RED_SIZE,         8 },
	{ EGL_DEPTH_SIZE,      24 },
	{ EGL_STENCIL_SIZE,     8 },
};

/**
 * EGL configurations supported by direct framebuffer access.
 * Exported to platform-independent EGL code.
 */
const FGLConfigs gPlatformConfigs[] = {
	{ configAttributes0, NELEM(configAttributes0)   },
	{ configAttributes1, NELEM(configAttributes1)   },
	{ configAttributes2, NELEM(configAttributes2)   },
	{ configAttributes3, NELEM(configAttributes3)   },
	{ configAttributes4, NELEM(configAttributes4)   },
	{ configAttributes5, NELEM(configAttributes5)   },
	{ configAttributes6, NELEM(configAttributes6)   },
	{ configAttributes7, NELEM(configAttributes7)   },
	{ configAttributes8, NELEM(configAttributes8)   },
	{ configAttributes9, NELEM(configAttributes9)   },
	{ configAttributes10, NELEM(configAttributes10) },
	{ configAttributes11, NELEM(configAttributes11) },
};

/** Number of exported EGL configurations */
const int gPlatformConfigsNum = NELEM(gPlatformConfigs);

/**
 * Framebuffer window render surface.
 * Provides framebuffer for rendering operations directly from Linux
 * framebuffer device.
 */
class FGLFramebufferWindowSurface : public FGLRenderSurface {
	int	bytesPerPixel;
	int	fd;
	FGLLocalSurface *buffers;

	void		*vbase;
	unsigned long	vlen;

public:
	/**
	 * Class constructor.
	 * Constructs a window surface from Linux framebuffer device.
	 * @param dpy EGL display to which the surface shall belong.
	 * @param config Index of EGL configuration to use.
	 * @param pixelFormat Requested color format. (See ::FGLPixelFormatEnum)
	 * @param depthFormat Requested depth format.
	 * @param fileDesc File descriptor of opened framebuffer device.
	 */
	FGLFramebufferWindowSurface(EGLDisplay dpy, uint32_t config,
				uint32_t pixelFormat, uint32_t depthFormat,
				int fileDesc) :
		FGLRenderSurface(dpy, config, pixelFormat, depthFormat),
		bytesPerPixel(0),
		fd(fileDesc)
	{
	}

	/** Class destructor. */
	~FGLFramebufferWindowSurface()
	{
	}

	virtual bool swapBuffers()
	{
		/* TODO: Swap buffers */

		return true;
	}

	virtual bool allocate(FGLContext *fgl)
	{
		if (depthFormat) {
			unsigned int size = width * height * 4;

			delete depth;
			depth = new FGLLocalSurface(size);
			if (!depth || !depth->isValid()) {
				setError(EGL_BAD_ALLOC);
				return false;
			}
		}

		/* TODO: Allocate backing storage for color buffer */

		return true;
	}

	virtual void free()
	{
		/* TODO: ree backing storage */
	}

	virtual bool initCheck() const
	{
		return vbase != NULL;
	}

	virtual EGLint getSwapBehavior() const
	{
		return EGL_BUFFER_DESTROYED;
	}
};

/** 32-bit pixel formats supported by framebuffer. */
static uint32_t framebufferFormats32bpp[] = {
	FGL_PIXFMT_XRGB8888,
	FGL_PIXFMT_ARGB8888,
	FGL_PIXFMT_XBGR8888,
	FGL_PIXFMT_ABGR8888
};

/** 16-bit pixel formats supported by framebuffer. */
static uint32_t framebufferFormats16bpp[] = {
	FGL_PIXFMT_XRGB1555,
	FGL_PIXFMT_RGB565,
	FGL_PIXFMT_ARGB4444,
	FGL_PIXFMT_ARGB1555
};

/**
 * Finds pixel format compatible with given framebuffer configuration.
 * @param vinfo Framebuffer configuration.
 * @param fglFormat Pointer pointing where to store the found format.
 * @param formats Arrays of formats to check for compatibility.
 * @param count Count of formats in formats array.
 * @return True if appropriate format was found, otherwise false.
 */
static bool fglFindCompatiblePixelFormat(const fb_var_screeninfo *vinfo,
		uint32_t *fglFormat, const uint32_t *formats, uint32_t count)
{
	while (count--) {
		uint32_t fmt = *(formats++);
		const FGLPixelFormat *pix = FGLPixelFormat::get(fmt);

		if (vinfo->red.offset != pix->comp[FGL_COMP_RED].pos)
			continue;
		if (vinfo->red.length != pix->comp[FGL_COMP_RED].size)
			continue;
		if (vinfo->green.offset != pix->comp[FGL_COMP_GREEN].pos)
			continue;
		if (vinfo->green.length != pix->comp[FGL_COMP_GREEN].size)
			continue;
		if (vinfo->blue.offset != pix->comp[FGL_COMP_BLUE].pos)
			continue;
		if (vinfo->blue.length != pix->comp[FGL_COMP_BLUE].size)
			continue;
		if (vinfo->transp.offset != pix->comp[FGL_COMP_ALPHA].pos)
			continue;
		if (vinfo->transp.length != pix->comp[FGL_COMP_ALPHA].size)
			continue;

		*fglFormat = fmt;
		return true;
	}

	return false;
}

/**
 * Finds internal pixel format corresponding to framebuffer pixel format.
 * @param vinfo Framebuffer configuration.
 * @param fglFormat Pointer to variable where internal pixel format shall
 * be stored.
 * @return True if corresponding format was found, otherwise false.
 */
static bool fglNativeToFGLPixelFormat(const fb_var_screeninfo *vinfo,
							uint32_t *fglFormat)
{
	switch(vinfo->bits_per_pixel) {
	case 32:
		return fglFindCompatiblePixelFormat(vinfo,
					fglFormat, framebufferFormats32bpp,
					NELEM(framebufferFormats32bpp));
	case 16:
		return fglFindCompatiblePixelFormat(vinfo,
					fglFormat, framebufferFormats16bpp,
					NELEM(framebufferFormats16bpp));
	default:
		break;
	}

	return false;
}

/**
 * Creates native window surface based on user parameters.
 * This function is a glue between generic and platform-specific EGL parts.
 * @param dpy EGL display to which the surface shall belong.
 * @param config EGL configuration which shall be used.
 * @param pixelFormat Preferred pixel format.
 * @param depthFormat Requested depth format.
 * @param window EGL native window backing the surface.
 * @return Created render surface or NULL on error.
 */
FGLRenderSurface *platformCreateWindowSurface(EGLDisplay dpy,
		uint32_t config, uint32_t pixelFormat, uint32_t depthFormat,
		EGLNativeWindowType window)
{
	fb_var_screeninfo vinfo;
	int fd = (int)window;

	/* TODO: Get screen information */

	if (!fglNativeToFGLPixelFormat(&vinfo, &pixelFormat)) {
		setError(EGL_BAD_MATCH);
		return NULL;
	}

	if (!fglEGLValidatePixelFormat(config, pixelFormat)) {
		setError(EGL_BAD_MATCH);
		return NULL;
	}

	return new FGLFramebufferWindowSurface(dpy, config, pixelFormat, depthFormat, fd);
}

#define EGLFunc	__eglMustCastToProperFunctionPointerType

/** List of platform-specific EGL functions */
const FGLExtensionMap gPlatformExtensionMap[] = {
	{ NULL, NULL },
};
