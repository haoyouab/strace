/*
 * Copyright (c) 2015 Intel Corporation
 * Copyright (c) 2019 Patrik Jakobsson <pjakobsson@suse.de>
 * Copyright (c) 2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

# ifdef HAVE_DRM_H
#  include <drm.h>
# else
#  include <drm/drm.h>
# endif

# include "xlat/drm_capability.h"
# include "xlat/drm_client_capability.h"
# include "xlat/drm_control_func.h"
# include "xlat/drm_crtc_sequence_flags.h"
# include "xlat/drm_ctx_flags.h"
# include "xlat/drm_lock_flags.h"
# include "xlat/drm_modeset_cmd.h"
# include "xlat/drm_mode_encoder_type.h"
# include "xlat/drm_mode_flags.h"
# include "xlat/drm_mode_page_flip_flags.h"
# include "xlat/drm_mode_type.h"
# include "xlat/drm_syncobj_flags.h"
# include "xlat/drm_syncobj_wait_flags.h"

static void
print_drm_iowr(const unsigned int nr, const unsigned int size,
	       const char *str)
{
	tprintf("DRM_IOWR(%#x, %#x) /* %s */", nr, size, str);
}

int
drm_decode_number(struct tcb *const tcp, const unsigned int code)
{
	const unsigned int nr = _IOC_NR(code);

	if (_IOC_DIR(code) == (_IOC_READ | _IOC_WRITE)) {
		switch (nr) {
		case 0xa7:
			/* In Linux commit v3.12-rc7~26^2~2 a u32 padding was */
			/* added to struct drm_mode_get_connector so in old */
			/* kernel headers the size of this structure is 0x4c */
			/* instead of 0x50. That's why we only check _IOC_NR */
			print_drm_iowr(nr, _IOC_SIZE(code),
				       "DRM_IOCTL_MODE_GETCONNECTOR");
			return IOCTL_NUMBER_STOP_LOOKUP;
		}
	}

	return 0;
}

static int
drm_get_magic(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_auth auth;

	if (entering(tcp)) {
		tprints(", ");
		return 0;
	}

	/* exiting-only code */
	if (umove_or_printaddr(tcp, arg, &auth))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", auth, magic);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_irq_busid(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_irq_busid busid;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &busid))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_D("{", busid, busnum);
		PRINT_FIELD_D(", ", busid, devnum);
		PRINT_FIELD_D(", ", busid, funcnum);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &busid)) {
		PRINT_FIELD_D(", ", busid, irq);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_set_version(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_set_version ver;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &ver))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_D("{", ver, drm_di_major);
	PRINT_FIELD_D(", ", ver, drm_di_minor);
	PRINT_FIELD_D(", ", ver, drm_dd_major);
	PRINT_FIELD_D(", ", ver, drm_dd_minor);
	tprints("}");

	if (entering(tcp))
		return 0;


	return RVAL_IOCTL_DECODED;
}


static int
drm_modeset_ctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_modeset_ctl ctl;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &ctl))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", ctl, crtc);
	PRINT_FIELD_XVAL(", ", ctl, cmd, drm_modeset_cmd, "_DRM_???");
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_gem_close(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_gem_close close;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &close))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", close, handle);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_gem_flink(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_gem_flink flink;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &flink))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", flink, handle);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &flink)) {
		PRINT_FIELD_U(", ", flink, name);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_gem_open(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_gem_open open;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &open))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", open, name);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &open)) {
		PRINT_FIELD_U(", ", open, handle);
		PRINT_FIELD_U(", ", open, size);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_get_cap(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_get_cap cap;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &cap))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_XVAL("{", cap, capability, drm_capability,
				 "DRM_CAP_???");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &cap)) {
		PRINT_FIELD_U(", ", cap, value);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_set_client_cap(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_set_client_cap cap;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &cap))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_XVAL("{", cap, capability, drm_client_capability,
			 "DRM_CLIENT_CAP_???");
	PRINT_FIELD_U(", ", cap, value);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_auth_magic(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_auth auth;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &auth))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", auth, magic);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_noop(struct tcb *const tcp, const kernel_ulong_t arg)
{
	/* No-op ioctl */
	tprints(", ");
	printaddr(arg);
	return RVAL_IOCTL_DECODED;
}

static int
drm_control(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_control control;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &control))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_XVAL("{", control, func, drm_control_func,
			 "DRM_???");
	PRINT_FIELD_D(", ", control, irq);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_ctx(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_ctx ctx;

	if (entering(tcp)) {
		tprints(", ");
		return 0;
	}

	/* exiting-only code */
	if (umove_or_printaddr(tcp, arg, &ctx))
		return RVAL_IOCTL_DECODED;
	PRINT_FIELD_U("{", ctx, handle);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_rm_ctx(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_ctx ctx;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &ctx))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", ctx, handle);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_get_ctx(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_ctx ctx;

	if (entering(tcp)) {
		tprints(", ");
		return 0;
	}

	/* exiting-only code */
	if (umove_or_printaddr(tcp, arg, &ctx))
		return RVAL_IOCTL_DECODED;
	PRINT_FIELD_FLAGS("{", ctx, flags, drm_ctx_flags, "_DRM_CONTEXT_???");
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_lock(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_lock lock;

	if (entering(tcp)) {
		tprints(", ");
		return 0;
	}

	/* exiting-only code */
	if (umove_or_printaddr(tcp, arg, &lock))
		return RVAL_IOCTL_DECODED;
	PRINT_FIELD_D("{", lock, context);
	PRINT_FIELD_FLAGS(", ", lock, flags, drm_lock_flags, "_DRM_LOCK_???");
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_prime_handle_to_fd(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_prime_handle handle;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &handle))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", handle, handle);
		PRINT_FIELD_X(", ", handle, flags);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &handle)) {
		PRINT_FIELD_FD(", ", handle, fd, tcp);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_prime_fd_to_handle(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_prime_handle handle;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &handle))
			return RVAL_IOCTL_DECODED;
		/* The printing oder here is different comparing to */
		/* the order of fields in the struture because in */
		/* this ioctl command we have to get handle via fd. */
		PRINT_FIELD_FD("{", handle, fd, tcp);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &handle)) {
		PRINT_FIELD_U(", ", handle, handle);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;

}

# ifdef DRM_IOCTL_CRTC_GET_SEQUENCE
static int
drm_crtc_get_sequence(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_crtc_get_sequence seq;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &seq))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", seq, crtc_id);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &seq)) {
		PRINT_FIELD_U(", ", seq, active);
		PRINT_FIELD_U(", ", seq, sequence);
		PRINT_FIELD_D(", ", seq, sequence_ns);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_CRTC_QUEUE_SEQUENCE
static int
drm_crtc_queue_sequence(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_crtc_queue_sequence seq;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &seq))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		PRINT_FIELD_U("{", seq, crtc_id);
		PRINT_FIELD_FLAGS(", ", seq, flags, drm_crtc_sequence_flags,
				 "DRM_CRTC_SEQUENCE_???");
		PRINT_FIELD_X(", ", seq, user_data);
		PRINT_FIELD_U(", ", seq, sequence);
		tprints("}");

		return 0;
	} else {
		PRINT_FIELD_U("{", seq, sequence);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}
# endif

static int
drm_mode_get_resources(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_card_res res;
	uint32_t u32_val;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &res))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		tprints("{fb_id_ptr=");
		print_array(tcp, res.fb_id_ptr, res.count_fbs,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint32_array_member, 0);
		tprints(", crtc_id_ptr=");
		print_array(tcp, res.crtc_id_ptr, res.count_crtcs,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint32_array_member, 0);
		tprints(", connector_id_ptr=");
		print_array(tcp, res.connector_id_ptr, res.count_connectors,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint32_array_member, 0);
		tprints(", encoder_id_ptr=");
		print_array(tcp, res.encoder_id_ptr, res.count_encoders,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint32_array_member, 0);
		PRINT_FIELD_U(", ", res, count_fbs);
		PRINT_FIELD_U(", ", res, count_crtcs);
		PRINT_FIELD_U(", ", res, count_connectors);
		PRINT_FIELD_U(", ", res, count_encoders);
		tprints("}");

		return 0;
	}

	PRINT_FIELD_U("{", res, count_fbs);
	PRINT_FIELD_U(", ", res, count_crtcs);
	PRINT_FIELD_U(", ", res, count_connectors);
	PRINT_FIELD_U(", ", res, count_encoders);
	PRINT_FIELD_U(", ", res, min_width);
	PRINT_FIELD_U(", ", res, max_width);
	PRINT_FIELD_U(", ", res, min_height);
	PRINT_FIELD_U(", ", res, max_height);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static void
print_drm_mode_modeinfo(struct drm_mode_modeinfo *info)
{
	PRINT_FIELD_U("{", *info, clock);
	PRINT_FIELD_U(", ", *info, hdisplay);
	PRINT_FIELD_U(", ", *info, hsync_start);
	PRINT_FIELD_U(", ", *info, hsync_end);
	PRINT_FIELD_U(", ", *info, htotal);
	PRINT_FIELD_U(", ", *info, hskew);
	PRINT_FIELD_U(", ", *info, vdisplay);
	PRINT_FIELD_U(", ", *info, vsync_start);
	PRINT_FIELD_U(", ", *info, vsync_end);
	PRINT_FIELD_U(", ", *info, vtotal);
	PRINT_FIELD_U(", ", *info, vscan);
	PRINT_FIELD_U(", ", *info, vrefresh);
	PRINT_FIELD_FLAGS(", ", *info, flags, drm_mode_flags,
			 "DRM_MODE_FLAG_PIC_???");
	PRINT_FIELD_XVAL(", ", *info, type, drm_mode_type,
			 "DRM_MODE_TYPE_???");
	PRINT_FIELD_CSTRING(", ", *info, name);
	tprints("}");
}

static void
print_drm_mode_crtc_tail(struct tcb *const tcp, struct drm_mode_crtc *crtc)
{
	uint32_t set_connectors_ptr;

	tprints(", set_connectors_ptr=");
	print_array(tcp, crtc->set_connectors_ptr, crtc->count_connectors,
		    &set_connectors_ptr, sizeof(set_connectors_ptr),
		    tfetch_mem, print_uint32_array_member, 0);
	PRINT_FIELD_U(", ", *crtc, count_connectors);
	PRINT_FIELD_U(", ", *crtc, fb_id);
	PRINT_FIELD_U(", ", *crtc, x);
	PRINT_FIELD_U(", ", *crtc, y);
	PRINT_FIELD_U(", ", *crtc, gamma_size);
	PRINT_FIELD_U(", ", *crtc, mode_valid);
	tprints(", mode=");
	print_drm_mode_modeinfo(&crtc->mode);
}

static int
drm_mode_crtc(struct tcb *const tcp, const kernel_ulong_t arg,
	      bool is_get)
{
	struct drm_mode_crtc crtc;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &crtc))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", crtc, crtc_id);
		if (is_get)
			return 0;
		print_drm_mode_crtc_tail(tcp, &crtc);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &crtc) && is_get) {
		print_drm_mode_crtc_tail(tcp, &crtc);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_cursor(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_cursor cursor;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &cursor))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_X("{", cursor, flags);
		PRINT_FIELD_U(", ", cursor, crtc_id);
		PRINT_FIELD_D(", ", cursor, x);
		PRINT_FIELD_D(", ", cursor, y);
		PRINT_FIELD_U(", ", cursor, width);
		PRINT_FIELD_U(", ", cursor, height);
		PRINT_FIELD_U(", ", cursor, handle);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_gamma(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_crtc_lut lut;
	int array_size;
	uint16_t u32_val;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &lut))
			return RVAL_IOCTL_DECODED;

		array_size = lut.gamma_size * (sizeof(uint16_t));
		PRINT_FIELD_U("{", lut, crtc_id);
		PRINT_FIELD_U(", ", lut, gamma_size);
		tprints(", red=");
		print_array(tcp, lut.red, array_size,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint16_array_member, 0);
		tprints(", green=");
		print_array(tcp, lut.green, array_size,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint16_array_member, 0);
		tprints(", blue=");
		print_array(tcp, lut.blue, array_size,
			    &u32_val, sizeof(u32_val),
			    tfetch_mem, print_uint16_array_member, 0);

		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_get_encoder(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_get_encoder enc;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &enc))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", enc, encoder_id);

	if (entering(tcp)) {
		tprints("}");

		return 0;
	}

	PRINT_FIELD_XVAL(", ", enc, encoder_type, drm_mode_encoder_type,
			 "DRM_MODE_ENCODER_???");
	PRINT_FIELD_U(", ", enc, crtc_id);
	PRINT_FIELD_X(", ", enc, possible_crtcs);
	PRINT_FIELD_X(", ", enc, possible_clones);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static bool
print_drm_mode_property_enum(struct tcb *tcp, void *elem_buf,
			     size_t elem_size, void *data)
{
	const struct drm_mode_property_enum *p = elem_buf;
	PRINT_FIELD_U("{", *p, value);
	PRINT_FIELD_CSTRING(", ", *p, name);
	tprints("}");
	return true;
}

static int
drm_mode_get_property(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_get_property prop;
	uint64_t values_val;
	struct drm_mode_property_enum enum_blob_val;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &prop))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		PRINT_FIELD_U("{", prop, prop_id);
		PRINT_FIELD_U(", ", prop, count_values);
		PRINT_FIELD_U(", ", prop, count_enum_blobs);
		tprints("}");

		return 0;
	}

	tprints("{values_ptr=");
	print_array(tcp, prop.values_ptr, prop.count_values,
		    &values_val, sizeof(values_val),
		    tfetch_mem, print_uint64_array_member, 0);
	tprints(", enum_blob_ptr=");
	print_array(tcp, prop.enum_blob_ptr, prop.count_enum_blobs,
		    &enum_blob_val, sizeof(enum_blob_val),
		    tfetch_mem, print_drm_mode_property_enum, 0);
	PRINT_FIELD_X(", ", prop, flags);
	PRINT_FIELD_CSTRING(", ", prop, name);
	PRINT_FIELD_U(", ", prop, count_values);
	PRINT_FIELD_U(", ", prop, count_enum_blobs);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_set_property(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_connector_set_property prop;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &prop))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", prop, value);
	PRINT_FIELD_U(", ", prop, prop_id);
	PRINT_FIELD_U(", ", prop, connector_id);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_get_prop_blob(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_get_blob blob;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &blob))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", blob, blob_id);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &blob)) {
		PRINT_FIELD_U(", ", blob, length);
		PRINT_FIELD_U(", ", blob, data);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static void
print_drm_mode_fb_cmd_part(struct drm_mode_fb_cmd *cmd)
{
	/* The prefix might differ here */
	/* so we print it in the caller. */
	PRINT_FIELD_U("", *cmd, width);
	PRINT_FIELD_U(", ", *cmd, height);
	PRINT_FIELD_U(", ", *cmd, pitch);
	PRINT_FIELD_U(", ", *cmd, bpp);
	PRINT_FIELD_U(", ", *cmd, depth);
	PRINT_FIELD_U(", ", *cmd, handle);
}

static int
drm_mode_get_fb(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_fb_cmd cmd;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &cmd))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", cmd, fb_id);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &cmd)) {
		tprints(", ");
		print_drm_mode_fb_cmd_part(&cmd);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_add_fb(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_fb_cmd cmd;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &cmd))
			return RVAL_IOCTL_DECODED;
		tprints("{");
		print_drm_mode_fb_cmd_part(&cmd);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &cmd)) {
		PRINT_FIELD_U(", ", cmd, fb_id);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_rm_fb(struct tcb *const tcp, const kernel_ulong_t arg)
{
	unsigned int fb_id;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &fb_id))
		return RVAL_IOCTL_DECODED;

	tprintf("{fb_id=%u}", fb_id);

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_page_flip(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_crtc_page_flip flip;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &flip))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", flip, crtc_id);
	PRINT_FIELD_U(", ", flip, fb_id);
	PRINT_FIELD_FLAGS(", ", flip, flags, drm_mode_page_flip_flags,
			 "DRM_MODE_PAGE_FLIP_???");
	PRINT_FIELD_X(", ", flip, user_data);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static bool
print_drm_clip_rect(struct tcb *tcp, void *elem_buf,
		    size_t elem_size, void *data)
{
	const struct drm_clip_rect *p = elem_buf;
	PRINT_FIELD_U("{", *p, x1);
	PRINT_FIELD_D(", ", *p, y1);
	PRINT_FIELD_D(", ", *p, x2);
	PRINT_FIELD_D(", ", *p, y2);
	tprints("}");
	return true;
}

static int
drm_mode_dirty_fb(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_fb_dirty_cmd cmd;
	struct drm_clip_rect clips_val;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &cmd))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", cmd, fb_id);
	PRINT_FIELD_X(", ", cmd, flags);
	PRINT_FIELD_X(", ", cmd, color);
	PRINT_FIELD_U(", ", cmd, num_clips);
	tprints(", clips_ptr=");
	print_array(tcp, cmd.clips_ptr, cmd. num_clips,
		    &clips_val, sizeof(clips_val),
		    tfetch_mem, print_drm_clip_rect, 0);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_create_dumb(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_create_dumb dumb;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &dumb))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", dumb, width);
		PRINT_FIELD_U(", ", dumb, height);
		PRINT_FIELD_U(", ", dumb, bpp);
		PRINT_FIELD_X(", ", dumb, flags);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &dumb)) {
		PRINT_FIELD_U(", ", dumb, handle);
		PRINT_FIELD_U(", ", dumb, pitch);
		PRINT_FIELD_U(", ", dumb, size);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_map_dumb(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_map_dumb dumb;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &dumb))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", dumb, handle);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &dumb)) {
		PRINT_FIELD_U(", ", dumb, offset);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_destroy_dumb(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_destroy_dumb dumb;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &dumb))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", dumb, handle);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_getplane(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_get_plane plane;
	uint32_t formant_type_val;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &plane))
		return RVAL_IOCTL_DECODED;

	tprints("{");
	if (entering(tcp)) {
		PRINT_FIELD_U("", plane, plane_id);
		PRINT_FIELD_U(", ", plane, count_format_types);
		tprints(", format_type_ptr=");
		print_array(tcp, plane.format_type_ptr, plane.count_format_types,
			    &formant_type_val, sizeof(formant_type_val),
			    tfetch_mem, print_uint32_array_member, 0);

		tprints("}");

		return 0;
	}

	PRINT_FIELD_U("", plane, plane_id);
	PRINT_FIELD_U(", ", plane, crtc_id);
	PRINT_FIELD_U(", ", plane, fb_id);
	PRINT_FIELD_U(", ", plane, possible_crtcs);
	PRINT_FIELD_U(", ", plane, gamma_size);
	PRINT_FIELD_U(", ", plane, count_format_types);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_setplane(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_set_plane plane;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &plane))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", plane, plane_id);
		PRINT_FIELD_U(", ", plane, crtc_id);
		PRINT_FIELD_U(", ", plane, fb_id);
		PRINT_FIELD_X(", ", plane, flags);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &plane)) {
		PRINT_FIELD_D(", ", plane, crtc_x);
		PRINT_FIELD_D(", ", plane, crtc_y);
		PRINT_FIELD_U(", ", plane, crtc_w);
		PRINT_FIELD_U(", ", plane, crtc_h);
		/* The printing format reference is from kernel driver. */
		PRINT_FIELD_U(", ", plane, src_x);
		tprintf_comment("%u.%06u", plane.src_x >> 16,
				((plane.src_x & 0xffff) * 15625) >> 10);
		PRINT_FIELD_U(", ", plane, src_y);
		tprintf_comment("%u.%06u", plane.src_y >> 16,
				((plane.src_y & 0xffff) * 15625) >> 10);
		PRINT_FIELD_U(", ", plane, src_h);
		tprintf_comment("%u.%06u", plane.src_h >> 16,
				((plane.src_h & 0xffff) * 15625) >> 10);
		PRINT_FIELD_U(", ", plane, src_w);
		tprintf_comment("%u.%06u", plane.src_w >> 16,
				((plane.src_w & 0xffff) * 15625) >> 10);

	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_cursor2(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_cursor2 cursor;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &cursor))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_X("{", cursor, flags);
		PRINT_FIELD_U(", ", cursor, crtc_id);
		PRINT_FIELD_D(", ", cursor, x);
		PRINT_FIELD_D(", ", cursor, y);
		PRINT_FIELD_U(", ", cursor, width);
		PRINT_FIELD_U(", ", cursor, height);
		PRINT_FIELD_U(", ", cursor, handle);
		PRINT_FIELD_D(", ", cursor, hot_x);
		PRINT_FIELD_D(", ", cursor, hot_y);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_atomic(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_atomic atomic;
	uint32_t u32_val;
	uint64_t u64_val;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &atomic))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_X("{", atomic, flags);
	PRINT_FIELD_U(", ", atomic, count_objs);
	tprints(", objs_ptr=");
	print_array(tcp, atomic.objs_ptr, atomic.count_objs,
		    &u32_val, sizeof(u32_val),
		    tfetch_mem, print_uint32_array_member, 0);
	tprints(", count_props_ptr=");
	print_array(tcp, atomic.count_props_ptr, atomic.count_objs,
		    &u32_val, sizeof(u32_val),
		    tfetch_mem, print_uint32_array_member, 0);
	tprints(", props_ptr=");
	print_array(tcp, atomic.props_ptr, atomic.count_objs,
		    &u32_val, sizeof(u32_val),
		    tfetch_mem, print_uint32_array_member, 0);
	tprints(", prop_values_ptr=");
	print_array(tcp, atomic.prop_values_ptr, atomic.count_objs,
		    &u64_val, sizeof(u64_val),
		    tfetch_mem, print_uint64_array_member, 0);
	if (atomic.reserved)
		PRINT_FIELD_U(", ", atomic, reserved);
	PRINT_FIELD_X(", ", atomic, user_data);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_createpropblob(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_create_blob blob;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &blob))
			return RVAL_IOCTL_DECODED;

		// TODO: print blob using printstr
		PRINT_FIELD_ADDR64("{", blob, data);
		PRINT_FIELD_U(", ", blob, length);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &blob)) {
		PRINT_FIELD_U(", ", blob, blob_id);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_destroypropblob(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_destroy_blob blob;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &blob))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", blob, blob_id);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# ifdef DRM_IOCTL_SYNCOBJ_CREATE
static int
drm_syncobj_create(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_create create;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &create))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_FLAGS("{", create, flags, drm_syncobj_flags,
				 "DRM_SYNCOBJ_???");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &create)) {
		PRINT_FIELD_U(", ", create, handle);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_DESTROY
static int
drm_syncobj_destroy(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_destroy destroy;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &destroy))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", destroy, handle);
	if (destroy.pad)
		PRINT_FIELD_U(", ", destroy, pad);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD
static int
drm_syncobj_handle_fd(struct tcb *const tcp, const kernel_ulong_t arg,
			 bool is_handle_to_fd)
{
	struct drm_syncobj_handle handle;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &handle))
			return RVAL_IOCTL_DECODED;

		if (is_handle_to_fd)
			PRINT_FIELD_U("{", handle, handle);
		else
			PRINT_FIELD_FD("{", handle, fd, tcp);
		PRINT_FIELD_X(", ", handle, flags);
		if (handle.pad)
			PRINT_FIELD_U(", ", handle, pad);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &handle)) {
		if (is_handle_to_fd)
			PRINT_FIELD_FD(", ", handle, fd, tcp);
		else
			PRINT_FIELD_U(", ", handle, handle);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_WAIT
static int
drm_syncobj_wait(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_wait wait;
	uint32_t handles_val;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &wait))
			return RVAL_IOCTL_DECODED;
		tprints(", handles=");
		print_array(tcp, wait.handles, wait.count_handles,
			    &handles_val, sizeof(handles_val),
			    tfetch_mem, print_uint32_array_member, 0);
		PRINT_FIELD_D(", ", wait, timeout_nsec);
		PRINT_FIELD_U(", ", wait, count_handles);
		PRINT_FIELD_X(", ", wait, flags);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &wait)) {
		PRINT_FIELD_U(", ", wait, first_signaled);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_RESET
static int
drm_syncobj_reset_or_signal(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_array array;
	uint32_t handles_val;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &array))
		return RVAL_IOCTL_DECODED;

	tprints(", handles=");
	print_array(tcp, array.handles, array.count_handles,
		    &handles_val, sizeof(handles_val),
		    tfetch_mem, print_uint32_array_member, 0);
	PRINT_FIELD_U(", ", array, count_handles);
	if (array.pad)
		PRINT_FIELD_U(", ", array, pad);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_MODE_CREATE_LEASE
static int
drm_mode_create_lease(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_create_lease lease;
	uint32_t object_val;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &lease))
			return RVAL_IOCTL_DECODED;

		tprints(", object_ids=");
		print_array(tcp, lease.object_ids, lease.object_count,
			    &object_val, sizeof(object_val),
			    tfetch_mem, print_uint32_array_member, 0);
		PRINT_FIELD_U(", ", lease, object_count);
		PRINT_FIELD_X(", ", lease, flags);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &lease)) {
		PRINT_FIELD_U(", ", lease, lessee_id);
		PRINT_FIELD_FD(", ", lease, fd, tcp);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_MODE_LIST_LESSEES
static int
drm_mode_list_lessees(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_list_lessees lessees;
	uint32_t lessees_val;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &lessees))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		PRINT_FIELD_U("{", lessees, count_lessees);
		if (lessees.pad)
			PRINT_FIELD_U(", ", lessees, pad);
		tprints(", lessees_ptr=");
		print_array(tcp, lessees.lessees_ptr, lessees.count_lessees,
			    &lessees_val, sizeof(lessees_val),
			    tfetch_mem, print_uint32_array_member, 0);

		tprints("}");

		return 0;
	} else {
		PRINT_FIELD_U("{", lessees, count_lessees);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_MODE_GET_LEASE
static int
drm_mode_get_lease(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_get_lease lease;
	uint32_t objects_val;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &lease))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		PRINT_FIELD_U("{", lease, count_objects);
		if (lease.pad)
			PRINT_FIELD_U(", ", lease, pad);
		tprints(", objects_ptr=");
		print_array(tcp, lease.objects_ptr, lease.count_objects,
			    &objects_val, sizeof(objects_val),
			    tfetch_mem, print_uint32_array_member, 0);

		tprints("}");

		return 0;
	} else {
		PRINT_FIELD_U("{", lease, count_objects);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;

}
# endif

# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
static int
drm_mode_revoke_lease(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_revoke_lease lease;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &lease))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", lease, lessee_id);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
static int
drm_syncobj_timeline_wait(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_timeline_wait wait;
	uint32_t handles_val;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &wait))
			return RVAL_IOCTL_DECODED;
		tprints(", handles=");
		print_array(tcp, wait.handles, wait.count_handles,
			    &handles_val, sizeof(handles_val),
			    tfetch_mem, print_uint32_array_member, 0);
		PRINT_FIELD_D(", ", wait, timeout_nsec);
		PRINT_FIELD_U(", ", wait, count_handles);
		PRINT_FIELD_FLAGS(", ", wait, flags, drm_syncobj_wait_flags,
				 "DRM_SYNCOBJ_WAIT_FLAGS_???");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &wait)) {
		PRINT_FIELD_U(", ", wait, first_signaled);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_QUERY
static int
drm_syncobj_query_or_timeline_signal(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_timeline_array array;
	uint32_t handles_val;
	uint64_t points_val;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &array))
		return RVAL_IOCTL_DECODED;

	tprints("{handles=");
	print_array(tcp, array.handles, array.count_handles,
		    &handles_val, sizeof(handles_val),
		    tfetch_mem, print_uint32_array_member, 0);
	print_array(tcp, array.points, array.count_handles,
		    &points_val, sizeof(points_val),
		    tfetch_mem, print_uint64_array_member, 0);

	PRINT_FIELD_U(", ", array, count_handles);
	if (array.pad)
		PRINT_FIELD_U(", ", array, pad);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
static int
drm_syncobj_transfer(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_transfer transfer;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &transfer))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", transfer, src_handle);
	PRINT_FIELD_U(", ", transfer, dst_handle);
	PRINT_FIELD_U(", ", transfer, src_point);
	PRINT_FIELD_U(", ", transfer, dst_point);
	PRINT_FIELD_FLAGS(", ", transfer, flags, drm_syncobj_wait_flags,
			 "DRM_SYNCOBJ_WAIT_FLAGS_???");
	if (transfer.pad)
		PRINT_FIELD_U(", ", transfer, pad);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

int
drm_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	switch (code) {
	case DRM_IOCTL_GET_MAGIC: /* R */
		return drm_get_magic(tcp, arg);
	case DRM_IOCTL_IRQ_BUSID: /* RW */
		return drm_irq_busid(tcp, arg);
	case DRM_IOCTL_SET_VERSION: /* RW */
		return drm_set_version(tcp, arg);
	case DRM_IOCTL_MODESET_CTL: /* W */
		return drm_modeset_ctl(tcp, arg);
	case DRM_IOCTL_GEM_CLOSE: /* W */
		return drm_gem_close(tcp, arg);
	case DRM_IOCTL_GEM_FLINK: /* RW */
		return drm_gem_flink(tcp, arg);
	case DRM_IOCTL_GEM_OPEN: /* RW */
		return drm_gem_open(tcp, arg);
	case DRM_IOCTL_GET_CAP: /* RW */
		return drm_get_cap(tcp, arg);
	case DRM_IOCTL_SET_CLIENT_CAP: /* W */
		return drm_set_client_cap(tcp, arg);
	case DRM_IOCTL_AUTH_MAGIC: /* W */
		return drm_auth_magic(tcp, arg);
	case DRM_IOCTL_BLOCK: /* RW */
	case DRM_IOCTL_UNBLOCK: /* RW */
	case DRM_IOCTL_MOD_CTX: /* W */
	case DRM_IOCTL_ADD_DRAW: /* RW */
	case DRM_IOCTL_RM_DRAW: /* RW */
	case DRM_IOCTL_FINISH: /* W */
	case DRM_IOCTL_MODE_ATTACHMODE: /* RW */
	case DRM_IOCTL_MODE_DETACHMODE: /* RW */
	case DRM_IOCTL_GET_STATS: /* R */
		return drm_noop(tcp, arg);
	case DRM_IOCTL_CONTROL: /* W */
		return drm_control(tcp, arg);
	case DRM_IOCTL_ADD_CTX: /* RW */
	case DRM_IOCTL_SWITCH_CTX: /* W */
	case DRM_IOCTL_NEW_CTX: /* W */
		return drm_ctx(tcp, arg);
	case DRM_IOCTL_RM_CTX: /* RW */
		return drm_rm_ctx(tcp, arg);
	case DRM_IOCTL_GET_CTX: /* RW */
		return drm_get_ctx(tcp, arg);
	case DRM_IOCTL_LOCK: /* W */
	case DRM_IOCTL_UNLOCK: /* W */
		return drm_lock(tcp, arg);
	case DRM_IOCTL_PRIME_HANDLE_TO_FD: /* RW */
		return drm_prime_handle_to_fd(tcp, arg);
	case DRM_IOCTL_PRIME_FD_TO_HANDLE: /* RW */
		return drm_prime_fd_to_handle(tcp, arg);
# ifdef DRM_IOCTL_CRTC_GET_SEQUENCE
	case DRM_IOCTL_CRTC_GET_SEQUENCE: /* RW */
		return drm_crtc_get_sequence(tcp, arg);
# endif
# ifdef DRM_IOCTL_CRTC_QUEUE_SEQUENCE
	case DRM_IOCTL_CRTC_QUEUE_SEQUENCE: /* RW */
		return drm_crtc_queue_sequence(tcp, arg);
# endif
	case DRM_IOCTL_MODE_GETRESOURCES: /* RW */
		return drm_mode_get_resources(tcp, arg);
	case DRM_IOCTL_MODE_GETCRTC: /* RW */
	case DRM_IOCTL_MODE_SETCRTC: /* RW */
		return drm_mode_crtc(tcp, arg, code == DRM_IOCTL_MODE_GETCRTC);
	case DRM_IOCTL_MODE_CURSOR: /* RW */
		return drm_mode_cursor(tcp, arg);
	case DRM_IOCTL_MODE_GETGAMMA: /* RW */
	case DRM_IOCTL_MODE_SETGAMMA: /* RW */
		return drm_mode_gamma(tcp, arg);
	case DRM_IOCTL_MODE_GETENCODER: /* RW */
		return drm_mode_get_encoder(tcp, arg);
	case DRM_IOCTL_MODE_GETPROPERTY: /* RW */
		return drm_mode_get_property(tcp, arg);
	case DRM_IOCTL_MODE_SETPROPERTY: /* RW */
		return drm_mode_set_property(tcp, arg);
	case DRM_IOCTL_MODE_GETPROPBLOB: /* RW */
		return drm_mode_get_prop_blob(tcp, arg);
	case DRM_IOCTL_MODE_GETFB: /* RW */
		return drm_mode_get_fb(tcp, arg);
	case DRM_IOCTL_MODE_ADDFB: /* RW */
		return drm_mode_add_fb(tcp, arg);
	case DRM_IOCTL_MODE_RMFB: /* RW */
		return drm_mode_rm_fb(tcp, arg);
	case DRM_IOCTL_MODE_PAGE_FLIP: /* RW */
		return drm_mode_page_flip(tcp, arg);
	case DRM_IOCTL_MODE_DIRTYFB: /* RW */
		return drm_mode_dirty_fb(tcp, arg);
	case DRM_IOCTL_MODE_CREATE_DUMB: /* RW */
		return drm_mode_create_dumb(tcp, arg);
	case DRM_IOCTL_MODE_MAP_DUMB: /* RW */
		return drm_mode_map_dumb(tcp, arg);
	case DRM_IOCTL_MODE_DESTROY_DUMB: /* RW */
		return drm_mode_destroy_dumb(tcp, arg);
	case DRM_IOCTL_MODE_GETPLANE: /* RW */
		return drm_mode_getplane(tcp, arg);
	case DRM_IOCTL_MODE_SETPLANE: /* RW */
		return drm_mode_setplane(tcp, arg);
	case DRM_IOCTL_MODE_CURSOR2: /* RW */
		return drm_mode_cursor2(tcp, arg);
	case DRM_IOCTL_MODE_ATOMIC: /* RW */
		return drm_mode_atomic(tcp, arg);//
	case DRM_IOCTL_MODE_CREATEPROPBLOB: /* RW */
		return drm_mode_createpropblob(tcp, arg);
	case DRM_IOCTL_MODE_DESTROYPROPBLOB: /* RW */
		return drm_mode_destroypropblob(tcp, arg);
# ifdef DRM_IOCTL_SYNCOBJ_CREATE
	case DRM_IOCTL_SYNCOBJ_CREATE: /* RW */
		return drm_syncobj_create(tcp, arg);
# endif
# ifdef DRM_IOCTL_SYNCOBJ_DESTROY
	case DRM_IOCTL_SYNCOBJ_DESTROY: /* RW */
		return drm_syncobj_destroy(tcp, arg);
# endif
# ifdef DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD
	case DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD: /* RW */
	case DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE: /* RW */
		return drm_syncobj_handle_fd(tcp, arg, code == DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD);
# endif
# ifdef DRM_IOCTL_SYNCOBJ_WAIT
	case DRM_IOCTL_SYNCOBJ_WAIT: /* RW */
		return drm_syncobj_wait(tcp, arg);
# endif
# ifdef DRM_IOCTL_SYNCOBJ_RESET
	case DRM_IOCTL_SYNCOBJ_RESET: /* RW */
	case DRM_IOCTL_SYNCOBJ_SIGNAL: /* RW */
		return drm_syncobj_reset_or_signal(tcp, arg);
# endif
# ifdef DRM_IOCTL_MODE_CREATE_LEASE
	case DRM_IOCTL_MODE_CREATE_LEASE: /* RW */
		return drm_mode_create_lease(tcp, arg);
# endif
# ifdef DRM_IOCTL_MODE_LIST_LESSEES
	case DRM_IOCTL_MODE_LIST_LESSEES: /* RW */
		return drm_mode_list_lessees(tcp, arg);
# endif
# ifdef DRM_IOCTL_MODE_GET_LEASE
	case DRM_IOCTL_MODE_GET_LEASE: /* RW */
		return drm_mode_get_lease(tcp, arg);
# endif
# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
	case DRM_IOCTL_MODE_REVOKE_LEASE: /* RW */
		return drm_mode_revoke_lease(tcp, arg);
# endif
# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
	case DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT: /* RW */
		return drm_syncobj_timeline_wait(tcp, arg);
# endif
# ifdef DRM_IOCTL_SYNCOBJ_QUERY
	case DRM_IOCTL_SYNCOBJ_QUERY: /* RW */
	case DRM_IOCTL_SYNCOBJ_TIMELINE_SIGNAL: /* RW */
		return drm_syncobj_query_or_timeline_signal(tcp, arg);
# endif
# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
	case DRM_IOCTL_SYNCOBJ_TRANSFER: /* RW */
		return drm_syncobj_transfer(tcp, arg);
# endif
	default:
		return drm_ioctl_mpers(tcp, code, arg);
	}
}

#endif /* HAVE_DRM_H || HAVE_DRM_DRM_H */
