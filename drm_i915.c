/*
 * Copyright (c) 2019 Patrik Jakobsson <pjakobsson@suse.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"

#ifdef HAVE_DRM_H
#include <drm.h>
#include <i915_drm.h>
#else
#include <drm/drm.h>
#include <drm/i915_drm.h>
#endif

#include "xlat/drm_i915_ioctls.h"
#include "xlat/drm_i915_getparams.h"
#include "xlat/drm_i915_setparams.h"

#include DEF_MPERS_TYPE(struct_drm_i915_getparam)

typedef struct drm_i915_getparam struct_drm_i915_getparam;

#include MPERS_DEFS

static int
i915_init(struct tcb *tcp, const unsigned int code, long arg)
{
	struct _drm_i915_init init;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &init)) {
		PRINT_FIELD_X("{", init, func);
		PRINT_FIELD_X(", ", init, mmio_offset);
		PRINT_FIELD_D(", ", init, sarea_priv_offset);
		PRINT_FIELD_U(", ", init, ring_start);
		PRINT_FIELD_U(", ", init, ring_end);
		PRINT_FIELD_U(", ", init, ring_size);
		PRINT_FIELD_X(", ", init, front_offset);
		PRINT_FIELD_X(", ", init, back_offset);
		PRINT_FIELD_X(", ", init, depth_offset);
		PRINT_FIELD_U(", ", init, w);
		PRINT_FIELD_U(", ", init, h);
		PRINT_FIELD_U(", ", init, pitch);
		PRINT_FIELD_U(", ", init, pitch_bits);
		PRINT_FIELD_U(", ", init, back_pitch);
		PRINT_FIELD_U(", ", init, depth_pitch);
		PRINT_FIELD_U(", ", init, cpp);
		PRINT_FIELD_U(", ", init, chipset);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static void
drm_i915_print_clip_rect(struct drm_clip_rect *rect)
{
	PRINT_FIELD_U("", *rect, x1);
	PRINT_FIELD_U(", ", *rect, y1);
	PRINT_FIELD_U(", ", *rect, x2);
	PRINT_FIELD_U(", ", *rect, y2);
}

static int
i915_batchbuffer(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_batchbuffer batchbuffer;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &batchbuffer)) {
		PRINT_FIELD_D("{", batchbuffer, start);
		PRINT_FIELD_D(", ", batchbuffer, used);
		PRINT_FIELD_D(", ", batchbuffer, DR1);
		PRINT_FIELD_D(", ", batchbuffer, DR4);
		PRINT_FIELD_D(", ", batchbuffer, num_cliprects);
		tprints("cliprects={");
		drm_i915_print_clip_rect(batchbuffer.cliprects);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int i915_getparam(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_getparam param;
	int value;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &param))
			return RVAL_IOCTL_DECODED;
		tprints("{param=");
		printxval(drm_i915_getparams, param.param, "I915_PARAM_???");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &param)) {
		if (umove(tcp, (long)param.value, &value))
			return RVAL_IOCTL_DECODED;

		tprints(", value=");
		switch (param.param) {
		case I915_PARAM_CHIPSET_ID:
			tprintf("0x%04x", value);
			break;
		default:
			tprintf("%d", value);
		}
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int i915_setparam(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_setparam param;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &param)) {
		tprints("{param=");
		printxval(drm_i915_setparams, param.param, "I915_PARAM_???");
		tprintf(", value=%d}", param.value);
	}

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_execbuffer2(struct tcb *tcp, const unsigned int code,
				long arg)
{
	struct drm_i915_gem_execbuffer2 eb;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &eb)) {
		PRINT_FIELD_PTR("{", eb, buffers_ptr);
		PRINT_FIELD_U(", ", eb, buffer_count);
		PRINT_FIELD_X(", ", eb, batch_start_offset);
		PRINT_FIELD_U(", ", eb, batch_len);
		PRINT_FIELD_U(", ", eb, DR1);
		PRINT_FIELD_U(", ", eb, DR4);
		PRINT_FIELD_U(", ", eb, num_cliprects);
		PRINT_FIELD_PTR(", ", eb, cliprects_ptr);
		PRINT_FIELD_X(", ", eb, flags);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_busy(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_gem_busy busy;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &busy))
			return RVAL_IOCTL_DECODED;
		tprintf("{handle=%u", busy.handle);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &busy)) {
		tprintf(", busy=%c, ring=%u",
			(busy.busy & 0x1) ? 'Y' : 'N', (busy.busy >> 16));
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_create(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_gem_create create;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &create))
			return RVAL_IOCTL_DECODED;
		tprintf("{size=%Lu", create.size);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &create))
		tprintf(", handle=%u", create.handle);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_pread(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_gem_pread pr;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &pr)) {
		PRINT_FIELD_U("{", pr, handle);
		PRINT_FIELD_U(", ", pr, offset);
		PRINT_FIELD_U(", ", pr, size);
		PRINT_FIELD_PTR(", ", pr, data_ptr);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_pwrite(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_gem_pwrite pw;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &pw)) {
		PRINT_FIELD_U("{", pw, handle);
		PRINT_FIELD_U(", ", pw, offset);
		PRINT_FIELD_U(", ", pw, size);
		PRINT_FIELD_PTR(", ", pw, data_ptr);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_mmap(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_gem_mmap mmap;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &mmap))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", mmap, handle);
		PRINT_FIELD_U(", ", mmap, size);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &mmap)) {
		PRINT_FIELD_U(", ", mmap, offset);
		PRINT_FIELD_PTR(", ", mmap, addr_ptr);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_mmap_gtt(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_gem_mmap_gtt mmap;


	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &mmap))
			return RVAL_IOCTL_DECODED;
		tprintf("{handle=%u", mmap.handle);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &mmap))
		tprintf(", offset=%Lu", mmap.offset);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_set_domain(struct tcb *tcp, const unsigned int code,
			       long arg)
{
	struct drm_i915_gem_set_domain dom;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &dom))
			return RVAL_IOCTL_DECODED;

		tprintf("{handle=%u, read_domains=%x, write_domain=%x}",
			dom.handle, dom.read_domains, dom.write_domain);
	}

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_madvise(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_gem_madvise madv;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &madv))
			return RVAL_IOCTL_DECODED;
		tprintf("{handle=%u, madv=%u", madv.handle, madv.madv);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &madv))
		tprintf(", retained=%u", madv.retained);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_get_tiling(struct tcb *tcp, const unsigned int code,
			       long arg)
{
	struct drm_i915_gem_get_tiling tiling;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &tiling))
			return RVAL_IOCTL_DECODED;
		tprintf("{handle=%u", tiling.handle);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &tiling)) {
		tprintf(", tiling_mode=%u, swizzle_mode=%u",
			tiling.tiling_mode, tiling.swizzle_mode);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_set_tiling(struct tcb *tcp, const unsigned int code,
			       long arg)
{
	struct drm_i915_gem_set_tiling tiling;

	if (entering(tcp)) {
		tprints(", ");
		if (umove(tcp, arg, &tiling))
			return RVAL_IOCTL_DECODED;
		tprintf("{handle=%u, tiling_mode=%u, stride=%u",
			tiling.handle, tiling.tiling_mode, tiling.stride);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &tiling))
		tprintf(", swizzle_mode=%u", tiling.swizzle_mode);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int i915_gem_userptr(struct tcb *tcp, const unsigned int code, long arg)
{
	struct drm_i915_gem_userptr uptr;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &uptr))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_PTR("{", uptr, user_ptr);
		PRINT_FIELD_U(", ", uptr, user_size);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &uptr)) {
		PRINT_FIELD_X(", ", uptr, flags);
		PRINT_FIELD_U(", ", uptr, handle);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, drm_i915_decode_number, struct tcb *tcp, unsigned int arg)
{
	const char *str = xlookup(drm_i915_ioctls, arg);

	if (str) {
		tprintf("%s", str);
		return IOCTL_NUMBER_STOP_LOOKUP;
	}

	return 0;
}

MPERS_PRINTER_DECL(int, drm_i915_ioctl, struct tcb *tcp, const unsigned int code, long arg)
{
	switch (code) {
	case DRM_IOCTL_I915_INIT:
		return i915_init(tcp, code, arg);
	case DRM_IOCTL_I915_BATCHBUFFER:
		return i915_batchbuffer(tcp, code, arg);
	case DRM_IOCTL_I915_GETPARAM:
		return i915_getparam(tcp, code, arg);
	case DRM_IOCTL_I915_SETPARAM:
		return i915_setparam(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_EXECBUFFER2:
		return i915_gem_execbuffer2(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_BUSY:
		return i915_gem_busy(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_CREATE:
		return i915_gem_create(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_PREAD:
		return i915_gem_pread(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_PWRITE:
		return i915_gem_pwrite(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_MMAP:
		return i915_gem_mmap(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_MMAP_GTT:
		return i915_gem_mmap_gtt(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_SET_DOMAIN:
		return i915_gem_set_domain(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_MADVISE:
		return i915_gem_madvise(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_GET_TILING:
		return i915_gem_get_tiling(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_SET_TILING:
		return i915_gem_set_tiling(tcp, code, arg);
	case DRM_IOCTL_I915_GEM_USERPTR:
		return i915_gem_userptr(tcp, code, arg);
	default:
		tprints(", ");
		printaddr(arg);
		return RVAL_IOCTL_DECODED;
	}
}
