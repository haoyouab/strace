/*
 * Copyright (c) 2019 Patrik Jakobsson <pjakobsson@suse.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

#ifdef HAVE_DRM_H
# include <drm.h>
#else
# include <drm/drm.h>
#endif

#define DRM_MAX_NAME_LEN 128

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
			if (drm_is_driver(tcp, "i915"))
				return drm_i915_decode_number(tcp, code);
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
drm_set_version(struct tcb *const tcp, long arg)
{
	struct drm_set_version ver;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &ver))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_D("{", ver, drm_di_major);
		PRINT_FIELD_D(", ", ver, drm_di_minor);
		PRINT_FIELD_D(", ", ver, drm_dd_major);
		PRINT_FIELD_D(", ", ver, drm_dd_minor);
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &ver)) {
		PRINT_FIELD_D(" => {", ver, drm_di_major);
		PRINT_FIELD_D(", ", ver, drm_di_minor);
		PRINT_FIELD_D(", ", ver, drm_dd_major);
		PRINT_FIELD_D(", ", ver, drm_dd_minor);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_get_magic(struct tcb *const tcp, long arg)
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
drm_mode_get_resources(struct tcb *const tcp, long arg)
{
	struct drm_mode_card_res res;

	if (exiting(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &res))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_PTR("{", res, fb_id_ptr);
		PRINT_FIELD_PTR(", ", res, crtc_id_ptr);
		PRINT_FIELD_PTR(", ", res, connector_id_ptr);
		PRINT_FIELD_PTR(", ", res, encoder_id_ptr);
		PRINT_FIELD_U(", ", res, count_fbs);
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

	return 0;
}

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
	PRINT_FIELD_X(", ", *info, flags);
	PRINT_FIELD_U(", ", *info, type);
	PRINT_FIELD_CSTRING(", ", *info, name);
}

static int
drm_mode_get_crtc(struct tcb *const tcp, long arg)
{
	struct drm_mode_crtc crtc;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &crtc))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", crtc, crtc_id);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &crtc)) {
		PRINT_FIELD_PTR(", ", crtc, set_connectors_ptr);
		PRINT_FIELD_U(", ", crtc, count_connectors);
		PRINT_FIELD_U(", ", crtc, fb_id);
		PRINT_FIELD_U(", ", crtc, x);
		PRINT_FIELD_U(", ", crtc, y);
		PRINT_FIELD_U(", ", crtc, gamma_size);
		PRINT_FIELD_U(", ", crtc, mode_valid);
		tprints("mode={");

		drm_mode_print_modeinfo(&crtc.mode);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_set_crtc(struct tcb *const tcp, long arg)
{
	struct drm_mode_crtc crtc;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &crtc))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_PTR("{", crtc, set_connectors_ptr);
		PRINT_FIELD_U(", ", crtc, count_connectors);
		PRINT_FIELD_U(", ", crtc, crtc_id);
		PRINT_FIELD_U(", ", crtc, fb_id);
		PRINT_FIELD_U(", ", crtc, x);
		PRINT_FIELD_U(", ", crtc, y);
		PRINT_FIELD_U(", ", crtc, gamma_size);
		PRINT_FIELD_U(", ", crtc, mode_valid);
		tprints("mode={");

		drm_mode_print_modeinfo(&crtc.mode);
		tprints("}}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_cursor(struct tcb *const tcp, long arg)
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
drm_mode_cursor2(struct tcb *const tcp, long arg)
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
drm_mode_get_gamma(struct tcb *const tcp, long arg)
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

static int
drm_mode_set_gamma(struct tcb *const tcp, long arg)
{
	struct drm_mode_crtc_lut lut;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &lut))
			return RVAL_IOCTL_DECODED;

		/* We don't print the entire table, just the rgb pointers */
		PRINT_FIELD_U("{", lut, crtc_id);
		PRINT_FIELD_U(", ", lut, gamma_size);
		PRINT_FIELD_PTR(", ", lut, red);
		PRINT_FIELD_PTR(", ", lut, green);
		PRINT_FIELD_PTR(", ", lut, blue);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_get_encoder(struct tcb *const tcp, long arg)
{
	struct drm_mode_get_encoder enc;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &enc))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", enc, encoder_id);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &enc)) {
		/* TODO: Print name of encoder type */
		PRINT_FIELD_U(", ", enc, encoder_type);
		PRINT_FIELD_U(", ", enc, crtc_id);
		PRINT_FIELD_X(", ", enc, possible_crtcs);
		PRINT_FIELD_X(", ", enc, possible_clones);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_get_property(struct tcb *const tcp, long arg)
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
drm_mode_set_property(struct tcb *const tcp, long arg)
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
drm_mode_get_prop_blob(struct tcb *const tcp, long arg)
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
drm_mode_add_fb(struct tcb *const tcp, long arg)
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
drm_mode_get_fb(struct tcb *const tcp, long arg)
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
drm_mode_rm_fb(struct tcb *const tcp, long arg)
{
	unsigned int handle;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &handle))
		tprintf("%u", handle);

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_page_flip(struct tcb *const tcp, long arg)
{
	struct drm_mode_crtc_page_flip flip;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &flip)) {
		PRINT_FIELD_U("{", flip, crtc_id);
		PRINT_FIELD_U(", ", flip, fb_id);
		PRINT_FIELD_X(", ", flip, flags);
		PRINT_FIELD_X(", ", flip, user_data);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_dirty_fb(struct tcb *const tcp, long arg)
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
drm_mode_create_dumb(struct tcb *const tcp, long arg)
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
drm_mode_map_dumb(struct tcb *const tcp, long arg)
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
drm_mode_destroy_dumb(struct tcb *const tcp, const unsigned int long arg)
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
drm_gem_close(struct tcb *const tcp, const unsigned int long arg)
{
	struct drm_gem_close close;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &close)) {
		PRINT_FIELD_U("{", close, handle);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

int
drm_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	/* Check for device specific ioctls */
	if (drm_is_priv(tcp->u_arg[1])) {
		if (verbose(tcp)) {
			if (drm_is_driver(tcp, "i915"))
				return drm_i915_ioctl(tcp, code, arg);
		}
	}

	switch (code) {
	case DRM_IOCTL_SET_VERSION:
		return drm_set_version(tcp, arg);
	case DRM_IOCTL_GET_MAGIC:
		return drm_get_magic(tcp, arg);
	case DRM_IOCTL_MODE_GETRESOURCES:
		return drm_mode_get_resources(tcp, arg);
	case DRM_IOCTL_MODE_GETCRTC:
		return drm_mode_get_crtc(tcp, arg);
	case DRM_IOCTL_MODE_SETCRTC:
		return drm_mode_set_crtc(tcp, arg);
	case DRM_IOCTL_MODE_CURSOR:
		return drm_mode_cursor(tcp, arg);
	case DRM_IOCTL_MODE_CURSOR2:
		return drm_mode_cursor2(tcp, arg);
	case DRM_IOCTL_MODE_GETGAMMA:
		return drm_mode_get_gamma(tcp, arg);
	case DRM_IOCTL_MODE_SETGAMMA:
		return drm_mode_set_gamma(tcp, arg);
	case DRM_IOCTL_MODE_GETENCODER:
		return drm_mode_get_encoder(tcp, arg);
	case DRM_IOCTL_MODE_GETPROPERTY:
		return drm_mode_get_property(tcp, arg);
	case DRM_IOCTL_MODE_SETPROPERTY:
		return drm_mode_set_property(tcp, arg);
	case DRM_IOCTL_MODE_GETPROPBLOB:
		return drm_mode_get_prop_blob(tcp, arg);
	case DRM_IOCTL_MODE_GETFB:
		return drm_mode_get_fb(tcp, arg);
	case DRM_IOCTL_MODE_ADDFB:
		return drm_mode_add_fb(tcp, arg);
	case DRM_IOCTL_MODE_RMFB:
		return drm_mode_rm_fb(tcp, arg);
	case DRM_IOCTL_MODE_PAGE_FLIP:
		return drm_mode_page_flip(tcp, arg);
	case DRM_IOCTL_MODE_DIRTYFB:
		return drm_mode_dirty_fb(tcp, arg);
	case DRM_IOCTL_MODE_CREATE_DUMB:
		return drm_mode_create_dumb(tcp, arg);
	case DRM_IOCTL_MODE_MAP_DUMB:
		return drm_mode_map_dumb(tcp, arg);
	case DRM_IOCTL_MODE_DESTROY_DUMB:
		return drm_mode_destroy_dumb(tcp, arg);
	case DRM_IOCTL_GEM_CLOSE:
		return drm_gem_close(tcp, arg);
	default:
		return drm_ioctl_mpers(tcp, code, arg);
	}
}

#endif /* HAVE_DRM_H || HAVE_DRM_DRM_H */
