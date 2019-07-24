/*
 * Copyright (c) 2019 Patrik Jakobsson <pjakobsson@suse.de>
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

# define DRM_MAX_NAME_LEN 128

static void
print_drm_iowr(const unsigned int nr, const unsigned int size,
	       const char *str)
{
	tprintf("DRM_IOWR(%#x, %#x) /* %s */", nr, size, str);
}

static inline int
drm_is_priv(const unsigned int num)
{
	return (_IOC_NR(num) >= DRM_COMMAND_BASE &&
		_IOC_NR(num) < DRM_COMMAND_END);
}

static char *
drm_get_driver_name(struct tcb *tcp)
{
	char path[PATH_MAX];
	char link[PATH_MAX];
	int ret;

	if (getfdpath(tcp, tcp->u_arg[0], path, PATH_MAX - 1) < 0)
		return NULL;

	if (snprintf(link, PATH_MAX, "/sys/class/drm/%s/device/driver",
	    basename(path)) >= (signed int)sizeof(link))
		return NULL;

	ret = readlink(link, path, PATH_MAX - 1);
	if (ret < 0)
		return NULL;

	path[ret] = '\0';
	return strdup(basename(path));
}

static int
drm_is_driver(struct tcb *tcp, const char *name)
{
	char *priv;

	/*
	 * If no private data is allocated we are detecting the driver name for
	 * the first time and must resolve it.
	 */
	if (tcp->_priv_data == NULL) {
		priv = drm_get_driver_name(tcp);

		if (priv == NULL)
			return 0;

		set_tcb_priv_data(tcp, priv, free);
	}

	return strncmp(name, get_tcb_priv_data(tcp), DRM_MAX_NAME_LEN) == 0;
}

int
drm_decode_number(struct tcb *const tcp, const unsigned int code)
{
	const unsigned int nr = _IOC_NR(code);

	if (drm_is_priv(tcp->u_arg[1])) {
		if (verbose(tcp)) {
# if defined(HAVE_I915_DRM_H) || defined(HAVE_DRM_I915_DRM_H)
			if (drm_is_driver(tcp, "i915"))
				return drm_i915_decode_number(tcp, code);
# endif
# if defined(HAVE_AMDGPU_DRM_H) || defined(HAVE_DRM_AMDGPU_DRM_H)
			if (drm_is_driver(tcp, "amdgpu"))
				return drm_amdgpu_decode_number(tcp, code);
# endif
		}
	}

	if (_IOC_DIR(code) == (_IOC_READ | _IOC_WRITE)) {
		switch (nr) {
			case 0xa7:
				/* In Linux commit v3.12-rc7~26^2~2 a u32 padding was added */
				/* to struct drm_mode_get_connector so in old kernel headers */
				/* the size of this structure is 0x4c instead of 0x50. */
				if (_IOC_SIZE(code) == 0x4c) {
					print_drm_iowr(nr, _IOC_SIZE(code),
						       "DRM_IOCTL_MODE_GETCONNECTOR");
					return IOCTL_NUMBER_STOP_LOOKUP;
				} else {
					return 0;
				}
		}
	}

	return 0;
}

static int
drm_get_magic(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_auth auth;

	if (exiting(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &auth))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_U("{", auth, magic);
		tprints("}");

		return RVAL_IOCTL_DECODED;
	}

	return 0;
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

# include "xlat/drm_modeset_cmd.h"

static int
drm_modeset_ctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_modeset_ctl ctl;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &ctl)) {
		PRINT_FIELD_U("{", ctl, crtc);
		printxval(drm_modeset_cmd, ctl.cmd, "_DRM_???");
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_gem_close(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_gem_close close;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &close)) {
		PRINT_FIELD_U("{", close, handle);
		tprints("}");
	}

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

# include "xlat/drm_capability.h"

static int
drm_get_cap(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_get_cap cap;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &cap))
			return RVAL_IOCTL_DECODED;
		tprints("{capability=");
		printxval(drm_capability, cap.capability, "DRM_CAP_???");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &cap)) {
		PRINT_FIELD_U(", ", cap, value);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_client_capability.h"

static int
drm_set_client_cap(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_set_client_cap cap;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &cap)) {
		tprints("{capability=");
		printxval(drm_client_capability, cap.capability, "DRM_CLIENT_CAP_???");
		PRINT_FIELD_U(", ", cap, value);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_auth_magic(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_auth auth;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &auth)) {
		PRINT_FIELD_U("{", auth, magic);
		tprints("}");
	}

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

# include "xlat/drm_control_func.h"

static int
drm_control(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_control control;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &control)) {
		tprints("{func=");
		printxval(drm_control_func, control.func, "DRM_???");
		PRINT_FIELD_D(", ", control, irq);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_ctx(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_ctx ctx;

	if (exiting(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &ctx))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", ctx, handle);
		tprints("}");

		return RVAL_IOCTL_DECODED;
	}

	return 0;
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

# include "xlat/drm_ctx_flags.h"

static int
drm_get_ctx(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_ctx ctx;

	if (exiting(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &ctx))
			return RVAL_IOCTL_DECODED;
		tprints("{flags=");
		printxval(drm_ctx_flags, ctx.flags, "_DRM_CONTEXT_???");
		tprints("}");

		return RVAL_IOCTL_DECODED;
	}

	return 0;
}

/* TODO
static int
drm_dma(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_dma dma;
}
*/

# include "xlat/drm_lock_flags.h"

static int
drm_lock(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_lock lock;

	if (exiting(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &lock))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_D("{", lock, context);
		tprints(", flags=");
		printxval(drm_lock_flags, lock.flags, "_DRM_LOCK_???");
		tprints("}");

		return RVAL_IOCTL_DECODED;
	}

	return 0;
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
		PRINT_FIELD_U(", ", handle, fd);
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
		PRINT_FIELD_U("{", handle, fd);

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

# include "xlat/drm_crtc_sequence_flags.h"

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

	tprints("{");
	if (entering(tcp)) {
		PRINT_FIELD_U("", seq, crtc_id);
		tprints(", flags=");
		printxval(drm_crtc_sequence_flags, seq.flags, "DRM_CRTC_SEQUENCE_???");
		PRINT_FIELD_U(", ", seq, user_data);
		PRINT_FIELD_U(", ", seq, sequence);
		tprints("}");

		return 0;
	}

	PRINT_FIELD_U("", seq, sequence);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

static int
drm_mode_get_resources(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_card_res res;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &res))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_PTR("{", res, fb_id_ptr);
		PRINT_FIELD_PTR(", ", res, crtc_id_ptr);
		PRINT_FIELD_PTR(", ", res, connector_id_ptr);
		PRINT_FIELD_PTR(", ", res, encoder_id_ptr);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &res)) {
		PRINT_FIELD_U(", ", res, count_fbs);
		PRINT_FIELD_U(", ", res, count_crtcs);
		PRINT_FIELD_U(", ", res, count_connectors);
		PRINT_FIELD_U(", ", res, count_encoders);
		PRINT_FIELD_U(", ", res, min_width);
		PRINT_FIELD_U(", ", res, max_width);
		PRINT_FIELD_U(", ", res, min_height);
		PRINT_FIELD_U(", ", res, max_height);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_mode_type.h"
# include "xlat/drm_mode_flags.h"

static void
drm_mode_print_modeinfo(struct drm_mode_modeinfo *info)
{
	PRINT_FIELD_U("", *info, clock);
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
	tprints(", flags=");
	printxval(drm_mode_flags, info->flags, "DRM_MODE_FLAG_PIC_???");
	tprints(", type=");
	printxval(drm_mode_type, info->type, "DRM_MODE_TYPE_???");
	PRINT_FIELD_CSTRING(", ", *info, name);
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
		PRINT_FIELD_PTR(", ", crtc, set_connectors_ptr);
		PRINT_FIELD_U(", ", crtc, count_connectors);
		PRINT_FIELD_U(", ", crtc, fb_id);
		PRINT_FIELD_U(", ", crtc, x);
		PRINT_FIELD_U(", ", crtc, y);
		PRINT_FIELD_U(", ", crtc, gamma_size);
		PRINT_FIELD_U(", ", crtc, mode_valid);
		tprints(", mode={");

		drm_mode_print_modeinfo(&crtc.mode);
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &crtc)) {
		if (is_get) {
			PRINT_FIELD_PTR(", ", crtc, set_connectors_ptr);
			PRINT_FIELD_U(", ", crtc, count_connectors);
			PRINT_FIELD_U(", ", crtc, fb_id);
			PRINT_FIELD_U(", ", crtc, x);
			PRINT_FIELD_U(", ", crtc, y);
			PRINT_FIELD_U(", ", crtc, gamma_size);
			PRINT_FIELD_U(", ", crtc, mode_valid);
			tprints(", mode={");

			drm_mode_print_modeinfo(&crtc.mode);
			tprints("}");
		}
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

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &lut))
			return RVAL_IOCTL_DECODED;

		/* We don't print the entire table, just the pointers */
		PRINT_FIELD_U("{", lut, crtc_id);
		PRINT_FIELD_U(", ", lut, gamma_size);
		PRINT_FIELD_PTR(", ", lut, red);
		PRINT_FIELD_PTR(", ", lut, green);
		PRINT_FIELD_PTR(", ", lut, blue);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_mode_encoder_type.h"

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

	tprints(", encoder_type=");
	printxval(drm_mode_encoder_type, enc.encoder_type, "DRM_MODE_ENCODER_???");
	PRINT_FIELD_U(", ", enc, crtc_id);
	PRINT_FIELD_X(", ", enc, possible_crtcs);
	PRINT_FIELD_X(", ", enc, possible_clones);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_get_property(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_get_property prop;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &prop))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", prop, prop_id);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &prop)) {
		PRINT_FIELD_PTR(", ", prop, values_ptr);
		PRINT_FIELD_PTR(", ", prop, enum_blob_ptr);
		PRINT_FIELD_X(", ", prop, flags);
		PRINT_FIELD_CSTRING(", ", prop, name);
		PRINT_FIELD_U(", ", prop, count_values);
		PRINT_FIELD_U(", ", prop, count_enum_blobs);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_set_property(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_connector_set_property prop;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &prop)) {
		PRINT_FIELD_U("{", prop, value);
		PRINT_FIELD_U(", ", prop, prop_id);
		PRINT_FIELD_U(", ", prop, connector_id);
		tprints("}");
	}

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
		PRINT_FIELD_U(", ", cmd, width);
		PRINT_FIELD_U(", ", cmd, height);
		PRINT_FIELD_U(", ", cmd, pitch);
		PRINT_FIELD_U(", ", cmd, bpp);
		PRINT_FIELD_U(", ", cmd, depth);
		PRINT_FIELD_U(", ", cmd, handle);
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
		PRINT_FIELD_U("{", cmd, width);
		PRINT_FIELD_U(", ", cmd, height);
		PRINT_FIELD_U(", ", cmd, pitch);
		PRINT_FIELD_U(", ", cmd, bpp);
		PRINT_FIELD_U(", ", cmd, depth);
		PRINT_FIELD_U(", ", cmd, handle);

		return 0;
	}

	if (!syserror(tcp) && !umove_or_printaddr(tcp, arg, &cmd)) {
		PRINT_FIELD_U(", ", cmd, fb_id);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_rm_fb(struct tcb *const tcp, const kernel_ulong_t arg)
{
	unsigned int handle;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &handle))
		tprintf("%u", handle);

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_mode_page_flip_flags.h"

static int
drm_mode_page_flip(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_crtc_page_flip flip;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &flip)) {
		PRINT_FIELD_U("{", flip, crtc_id);
		PRINT_FIELD_U(", ", flip, fb_id);
		tprints(", flags=");
		printxval(drm_mode_page_flip_flags, flip.flags, "DRM_MODE_PAGE_FLIP_???");
		PRINT_FIELD_X(", ", flip, user_data);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_dirty_fb(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_fb_dirty_cmd cmd;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &cmd)) {
		PRINT_FIELD_U("{", cmd, fb_id);
		PRINT_FIELD_X(", ", cmd, flags);
		PRINT_FIELD_X(", ", cmd, color);
		PRINT_FIELD_U(", ", cmd, num_clips);
		PRINT_FIELD_PTR(", ", cmd, clips_ptr);
		tprints("}");
	}

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
	if (!umove_or_printaddr(tcp, arg, &dumb)) {
		PRINT_FIELD_U("{", dumb, handle);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_getplane(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_get_plane plane;

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
		PRINT_FIELD_PTR(", ", plane, format_type_ptr);
		tprints("}");

		return 0;
	}

	PRINT_FIELD_PTR("", plane, format_type_ptr);
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
		PRINT_FIELD_D(", ", plane, crtc_w);
		PRINT_FIELD_D(", ", plane, crtc_h);
		PRINT_FIELD_D(", ", plane, src_x);
		PRINT_FIELD_D(", ", plane, src_y);
		PRINT_FIELD_D(", ", plane, src_h);
		PRINT_FIELD_D(", ", plane, src_w);
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

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &atomic)) {
		PRINT_FIELD_X("{", atomic, flags);
		PRINT_FIELD_U(", ", atomic, count_objs);
		PRINT_FIELD_PTR(", ", atomic, objs_ptr);
		PRINT_FIELD_PTR(", ", atomic, count_props_ptr);
		PRINT_FIELD_PTR(", ", atomic, props_ptr);
		PRINT_FIELD_PTR(", ", atomic, prop_values_ptr);
		PRINT_FIELD_U(", ", atomic, reserved);
		PRINT_FIELD_U(", ", atomic, user_data);
		tprints("}");
	}

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

		PRINT_FIELD_PTR("{", blob, data);
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
	if (!umove_or_printaddr(tcp, arg, &blob)) {
		PRINT_FIELD_U("{", blob, blob_id);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

# ifdef DRM_IOCTL_SYNCOBJ_CREATE

# include "xlat/drm_syncobj_flags.h"

static int
drm_syncobj_create(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_create create;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &create))
			return RVAL_IOCTL_DECODED;

		tprints("{flags=");
		printxval(drm_syncobj_flags, create.flags, "DRM_SYNCOJB_???");

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
	if (!umove_or_printaddr(tcp, arg, &destroy)) {
		PRINT_FIELD_U("{", destroy, handle);
		PRINT_FIELD_U(", ", destroy, pad);
		tprints("}");
	}

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
			PRINT_FIELD_D("{", handle, fd);
		PRINT_FIELD_X(", ", handle, flags);
		PRINT_FIELD_U(", ", handle, pad);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &handle)) {
		if (is_handle_to_fd)
			PRINT_FIELD_D(", ", handle, fd);
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

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &wait))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_PTR("{", wait, handles);
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

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &array)) {
		PRINT_FIELD_PTR("{", array, handles);
		PRINT_FIELD_U(", ", array, count_handles);
		PRINT_FIELD_U(", ", array, pad);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_MODE_CREATE_LEASE
static int
drm_mode_create_lease(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_create_lease lease;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &lease))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_PTR("{", lease, object_ids);
		PRINT_FIELD_U(", ", lease, object_count);
		PRINT_FIELD_X(", ", lease, flags);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &lease)) {
		PRINT_FIELD_U(", ", lease, lessee_id);
		PRINT_FIELD_U(", ", lease, fd);
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

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &lessees))
		return RVAL_IOCTL_DECODED;

	tprints("{");
	if (entering(tcp)) {
		PRINT_FIELD_U("", lessees, pad);
		PRINT_FIELD_PTR(", ",lessees, lessees_ptr);
		PRINT_FIELD_U(", ", lessees, count_lessees);
		tprints("}");

		return 0;
	}

	PRINT_FIELD_U("", lessees, count_lessees);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_MODE_GET_LEASE
static int
drm_mode_get_lease(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_get_lease lease;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &lease))
		return RVAL_IOCTL_DECODED;

	tprints("{");
	if (entering(tcp)) {
		PRINT_FIELD_U("", lease, pad);
		PRINT_FIELD_PTR(", ", lease, objects_ptr);
		PRINT_FIELD_U(", ", lease, count_objects);
		tprints("}");

		return 0;
	}

	PRINT_FIELD_U("", lease, count_objects);

	tprints("}");

	return RVAL_IOCTL_DECODED;

}
# endif

# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
static int
drm_mode_revoke_lease(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_mode_revoke_lease lease;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &lease)) {
		PRINT_FIELD_U("{", lease, lessee_id);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT

# include "xlat/drm_syncobj_wait_flags.h"

static int
drm_syncobj_timeline_wait(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_timeline_wait wait;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &wait))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_PTR("{", wait, handles);
		PRINT_FIELD_D(", ", wait, timeout_nsec);
		PRINT_FIELD_U(", ", wait, count_handles);
		tprints(", flags=");
		printxval(drm_syncobj_wait_flags, wait.flags, "DRM_SYNCOBJ_WAIT_FLAGS_???");

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

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &array)) {
		PRINT_FIELD_PTR("{", array, handles);
		PRINT_FIELD_PTR(", ", array, points);
		PRINT_FIELD_U(", ", array, count_handles);
		PRINT_FIELD_U(", ", array, pad);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
static int
drm_syncobj_transfer(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_syncobj_transfer transfer;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &transfer)) {
		PRINT_FIELD_U("{", transfer, src_handle);
		PRINT_FIELD_U(", ", transfer, dst_handle);
		PRINT_FIELD_U(", ", transfer, src_point);
		PRINT_FIELD_U(", ", transfer, dst_point);
		tprints(", flags=");
		printxval(drm_syncobj_wait_flags, transfer.flags, "DRM_SYNCOBJ_WAIT_FLAGS_???");
		PRINT_FIELD_U(", ", transfer, pad);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}
# endif

int
drm_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	/* Check for device specific ioctls */
	if (drm_is_priv(tcp->u_arg[1])) {
		if (verbose(tcp)) {
# if defined(HAVE_I915_DRM_H) || defined(HAVE_DRM_I915_DRM_H)
			if (drm_is_driver(tcp, "i915"))
				return drm_i915_ioctl(tcp, code, arg);
# endif
# if defined(HAVE_AMDGPU_DRM_H) || defined(HAVE_DRM_AMDGPU_DRM_H)
			if (drm_is_driver(tcp, "amdgpu"))
				return drm_amdgpu_ioctl(tcp, code, arg);
# endif
		}
	}

	switch (code) {
	case DRM_IOCTL_GET_MAGIC: /* R */
		return drm_get_magic(tcp, arg);
	case DRM_IOCTL_IRQ_BUSID: /* RW */
		return drm_irq_busid(tcp, arg);//
	case DRM_IOCTL_SET_VERSION: /* RW */
		return drm_set_version(tcp, arg);
	case DRM_IOCTL_MODESET_CTL: /* W */
		return drm_modeset_ctl(tcp, arg);//
	case DRM_IOCTL_GEM_CLOSE: /* W */
		return drm_gem_close(tcp, arg);
	case DRM_IOCTL_GEM_FLINK: /* RW */
		return drm_gem_flink(tcp, arg);//
	case DRM_IOCTL_GEM_OPEN: /* RW */
		return drm_gem_open(tcp, arg);//
	case DRM_IOCTL_GET_CAP: /* RW */
		return drm_get_cap(tcp, arg);//
	case DRM_IOCTL_SET_CLIENT_CAP: /* W */
		return drm_set_client_cap(tcp, arg);//
	case DRM_IOCTL_AUTH_MAGIC: /* W */
		return drm_auth_magic(tcp, arg);//
	case DRM_IOCTL_BLOCK: /* RW */
	case DRM_IOCTL_UNBLOCK: /* RW */
	case DRM_IOCTL_MOD_CTX: /* W */
	case DRM_IOCTL_ADD_DRAW: /* RW */
	case DRM_IOCTL_RM_DRAW: /* RW */
	case DRM_IOCTL_FINISH: /* W */
	case DRM_IOCTL_UPDATE_DRAW: /* W */
	case DRM_IOCTL_MODE_ATTACHMODE: /* RW */
	case DRM_IOCTL_MODE_DETACHMODE: /* RW */
		return drm_noop(tcp, arg);//
	case DRM_IOCTL_CONTROL: /* W */
		return drm_control(tcp, arg);//
	case DRM_IOCTL_ADD_CTX: /* RW */
	case DRM_IOCTL_SWITCH_CTX: /* W */
	case DRM_IOCTL_NEW_CTX: /* W */
		return drm_ctx(tcp, arg);//
	case DRM_IOCTL_RM_CTX: /* RW */
		return drm_rm_ctx(tcp, arg);
	case DRM_IOCTL_GET_CTX: /* RW */
		return drm_get_ctx(tcp, arg);
	// TODO
	//case DRM_IOCTL_DMA: /* RW */
		//return drm_dma(tcp, arg);//
	case DRM_IOCTL_LOCK: /* W */
	case DRM_IOCTL_UNLOCK: /* W */
		return drm_lock(tcp, arg);//
	case DRM_IOCTL_PRIME_HANDLE_TO_FD: /* RW */
		return drm_prime_handle_to_fd(tcp, arg);//
	case DRM_IOCTL_PRIME_FD_TO_HANDLE: /* RW */
		return drm_prime_fd_to_handle(tcp, arg);//
# ifdef DRM_IOCTL_CRTC_GET_SEQUENCE
	case DRM_IOCTL_CRTC_GET_SEQUENCE: /* RW */
		return drm_crtc_get_sequence(tcp, arg);//
# endif
# ifdef DRM_IOCTL_CRTC_QUEUE_SEQUENCE
	case DRM_IOCTL_CRTC_QUEUE_SEQUENCE: /* RW */
		return drm_crtc_queue_sequence(tcp, arg);//
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
		return drm_mode_getplane(tcp, arg);//
	case DRM_IOCTL_MODE_SETPLANE: /* RW */
		return drm_mode_setplane(tcp, arg);//
	case DRM_IOCTL_MODE_CURSOR2: /* RW */
		return drm_mode_cursor2(tcp, arg);
	case DRM_IOCTL_MODE_ATOMIC: /* RW */
		return drm_mode_atomic(tcp, arg);//
	case DRM_IOCTL_MODE_CREATEPROPBLOB: /* RW */
		return drm_mode_createpropblob(tcp, arg);//
	case DRM_IOCTL_MODE_DESTROYPROPBLOB: /* RW */
		return drm_mode_destroypropblob(tcp, arg);//
# ifdef DRM_IOCTL_SYNCOBJ_CREATE
	case DRM_IOCTL_SYNCOBJ_CREATE: /* RW */
		return drm_syncobj_create(tcp, arg);//
# endif
# ifdef DRM_IOCTL_SYNCOBJ_DESTROY
	case DRM_IOCTL_SYNCOBJ_DESTROY: /* RW */
		return drm_syncobj_destroy(tcp, arg);//
# endif
# ifdef DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD
	case DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD: /* RW */
	case DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE: /* RW */
		return drm_syncobj_handle_fd(tcp, arg, code == DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD);//
# endif
# ifdef DRM_IOCTL_SYNCOBJ_WAIT
	case DRM_IOCTL_SYNCOBJ_WAIT: /* RW */
		return drm_syncobj_wait(tcp, arg);//
# endif
# ifdef DRM_IOCTL_SYNCOBJ_RESET
	case DRM_IOCTL_SYNCOBJ_RESET: /* RW */
	case DRM_IOCTL_SYNCOBJ_SIGNAL: /* RW */
		return drm_syncobj_reset_or_signal(tcp, arg);//
# endif
# ifdef DRM_IOCTL_MODE_CREATE_LEASE
	case DRM_IOCTL_MODE_CREATE_LEASE: /* RW */
		return drm_mode_create_lease(tcp, arg);//
# endif
# ifdef DRM_IOCTL_MODE_LIST_LESSEES
	case DRM_IOCTL_MODE_LIST_LESSEES: /* RW */
		return drm_mode_list_lessees(tcp, arg);//
# endif
# ifdef DRM_IOCTL_MODE_GET_LEASE
	case DRM_IOCTL_MODE_GET_LEASE: /* RW */
		return drm_mode_get_lease(tcp, arg);//
# endif
# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
	case DRM_IOCTL_MODE_REVOKE_LEASE: /* RW */
		return drm_mode_revoke_lease(tcp, arg);//
# endif
# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
	case DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT: /* RW */
		return drm_syncobj_timeline_wait(tcp, arg);//
# endif
# ifdef DRM_IOCTL_SYNCOBJ_QUERY
	case DRM_IOCTL_SYNCOBJ_QUERY: /* RW */
	case DRM_IOCTL_SYNCOBJ_TIMELINE_SIGNAL: /* RW */
		return drm_syncobj_query_or_timeline_signal(tcp, arg);
# endif
# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
	case DRM_IOCTL_SYNCOBJ_TRANSFER: /* RW */
		return drm_syncobj_transfer(tcp, arg);//
# endif
	default:
		return drm_ioctl_mpers(tcp, code, arg);
	}

	return RVAL_DECODED;
}

#endif /* HAVE_DRM_H || HAVE_DRM_DRM_H */
