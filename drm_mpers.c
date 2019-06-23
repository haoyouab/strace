#include "defs.h"

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

#ifdef HAVE_DRM_H
# include <drm.h>
#else
# include <drm/drm.h>
#endif

#include DEF_MPERS_TYPE(struct_drm_version)
#include DEF_MPERS_TYPE(struct_drm_unique)
#include DEF_MPERS_TYPE(union_drm_wait_vblank)
#include DEF_MPERS_TYPE(struct_drm_mode_get_connector)
#include DEF_MPERS_TYPE(struct_drm_mode_fb_cmd2)

typedef struct drm_version struct_drm_version;
typedef struct drm_unique struct_drm_unique;
typedef union drm_wait_vblank union_drm_wait_vblank;
typedef struct drm_mode_get_connector struct_drm_mode_get_connector;
typedef struct drm_mode_fb_cmd2 struct_drm_mode_fb_cmd2;

#endif /* HAVE_DRM_H || HAVE_DRM_DRM_H */

#include MPERS_DEFS

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

#include "print_fields.h"

static int
drm_version(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_version ver;

	if (exiting(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &ver))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_D("{", ver, version_major);
		PRINT_FIELD_D(", ", ver, version_minor);
		PRINT_FIELD_D(", ", ver, version_patchlevel);
		PRINT_FIELD_U(", ", ver, name_len);
		tprints(", name=");
		printstrn(tcp, ptr_to_kulong(ver.name), ver.name_len);
		PRINT_FIELD_U(", ", ver, date_len);
		tprints(", date=");
		printstrn(tcp, ptr_to_kulong(ver.date), ver.date_len);
		PRINT_FIELD_U(", ", ver, desc_len);
		tprints(", desc=");
		printstrn(tcp, ptr_to_kulong(ver.desc), ver.desc_len);
		tprints("}");

		return RVAL_IOCTL_DECODED;
	}

	return 0;
}

static int
drm_get_unique(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_unique unique;

	if (exiting(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &unique))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_U("{", unique, unique_len);
		tprints(", unique=");
		printstrn(tcp, ptr_to_kulong(unique.unique), unique.unique_len);
		tprints("}");

		return RVAL_IOCTL_DECODED;
	}

	return 0;
}

static int
drm_wait_vblank(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union_drm_wait_vblank vblank;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &vblank))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_U("{request={", vblank.request, type);
		PRINT_FIELD_U(", ", vblank.request, sequence);
		PRINT_FIELD_U(", ", vblank.request, signal);
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &vblank)) {
		PRINT_FIELD_U(", reply={", vblank.reply, type);
		PRINT_FIELD_U(", ", vblank.reply, sequence);
		PRINT_FIELD_U(", ", vblank.reply, tval_sec);
		PRINT_FIELD_U(", ", vblank.reply, tval_usec);
		tprints("}");
	}
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_get_connector(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_mode_get_connector con;

	/* We could be very verbose here but keep is simple for now */
	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &con))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", con, connector_id);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &con)) {
		PRINT_FIELD_PTR(", ", con, encoders_ptr);
		PRINT_FIELD_PTR(", ", con, modes_ptr);
		PRINT_FIELD_PTR(", ", con, props_ptr);
		PRINT_FIELD_PTR(", ", con, prop_values_ptr);
		PRINT_FIELD_U(", ", con, count_modes);
		PRINT_FIELD_U(", ", con, count_props);
		PRINT_FIELD_U(", ", con, count_encoders);
		PRINT_FIELD_U(", ", con, encoder_id);
		PRINT_FIELD_U(", ", con, connector_type);
		PRINT_FIELD_U(", ", con, connector_type_id);
		PRINT_FIELD_U(", ", con, connection);
		PRINT_FIELD_U(", ", con, mm_width);
		PRINT_FIELD_U(", ", con, mm_height);
		PRINT_FIELD_U(", ", con, subpixel);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_add_fb2(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_mode_fb_cmd2 cmd;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &cmd))
			return RVAL_IOCTL_DECODED;
		tprintf("{width=%u, height=%u, pixel_format=%#x, flags=%u, "
			"handles={%u, %u, %u, %u}, "
			"pitches={%u, %u, %u, %u}, "
			"offsets={%u, %u, %u, %u}",
			cmd.width, cmd.height, cmd.pixel_format, cmd.flags,
			cmd.handles[0], cmd.handles[1], cmd.handles[2],
			cmd.handles[3], cmd.pitches[0], cmd.pitches[1],
			cmd.pitches[2], cmd.pitches[3], cmd.offsets[0],
			cmd.offsets[1], cmd.offsets[2], cmd.offsets[3]);
#ifdef HAVE_STRUCT_DRM_MODE_FB_CMD2_MODIFIER
		tprintf(", modifiers={%" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 "}",
			(uint64_t) cmd.modifier[0], (uint64_t) cmd.modifier[1],
			(uint64_t) cmd.modifier[2], (uint64_t) cmd.modifier[3]);
#endif

			return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &cmd))
		PRINT_FIELD_U(", ", cmd, fb_id);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, drm_ioctl_mpers, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case DRM_IOCTL_VERSION: /* RW */
		return drm_version(tcp, arg);
	case DRM_IOCTL_GET_UNIQUE: /* RW */
		return drm_get_unique(tcp, arg);
	case DRM_IOCTL_WAIT_VBLANK:
		return drm_wait_vblank(tcp, arg);
	case DRM_IOCTL_MODE_ADDFB2:
		return drm_mode_add_fb2(tcp, arg);
	}
	/* variable length, so we can't use switch(code) to identify DRM_IOCTL_MODE_GETCONNECTOR */
	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_GETCONNECTOR)) {
		return drm_mode_get_connector(tcp, arg);
	}

	return RVAL_DECODED;
}

#endif /* HAVE_DRM_H || HAVE_DRM_DRM_H */
