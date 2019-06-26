#include "defs.h"

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

#ifdef HAVE_DRM_H
# include <drm.h>
#else
# include <drm/drm.h>
#endif

#include DEF_MPERS_TYPE(struct_drm_version)
#include DEF_MPERS_TYPE(struct_drm_unique)
#include DEF_MPERS_TYPE(struct_drm_map)
#include DEF_MPERS_TYPE(struct_drm_client)
#include DEF_MPERS_TYPE(struct_drm_stats)
#include DEF_MPERS_TYPE(struct_drm_buf_desc)
#include DEF_MPERS_TYPE(struct_drm_buf_info)
#include DEF_MPERS_TYPE(struct_drm_buf_map)
#include DEF_MPERS_TYPE(struct_drm_buf_free)
#include DEF_MPERS_TYPE(struct_drm_ctx_priv_map)
#include DEF_MPERS_TYPE(struct_drm_ctx_res)
#include DEF_MPERS_TYPE(struct_drm_agp_mode)
#include DEF_MPERS_TYPE(struct_drm_agp_info)
#include DEF_MPERS_TYPE(struct_drm_agp_buffer)
#include DEF_MPERS_TYPE(struct_drm_agp_binding)
#include DEF_MPERS_TYPE(struct_drm_scatter_gather)
#include DEF_MPERS_TYPE(union_drm_wait_vblank)
#include DEF_MPERS_TYPE(struct_drm_mode_get_connector)
#include DEF_MPERS_TYPE(struct_drm_mode_fb_cmd2)
#include DEF_MPERS_TYPE(struct_drm_mode_get_plane_res)
#include DEF_MPERS_TYPE(struct_drm_mode_obj_get_properties)
#include DEF_MPERS_TYPE(struct_drm_mode_obj_set_property)

typedef struct drm_version struct_drm_version;
typedef struct drm_unique struct_drm_unique;
typedef struct drm_map struct_drm_map;
typedef struct drm_client struct_drm_client;
typedef struct drm_stats struct_drm_stats;
typedef struct drm_buf_desc struct_drm_buf_desc;
typedef struct drm_buf_info struct_drm_buf_info;
typedef struct drm_buf_map struct_drm_buf_map;
typedef struct drm_buf_free struct_drm_buf_free;
typedef struct drm_ctx_priv_map struct_drm_ctx_priv_map;
typedef struct drm_ctx_res struct_drm_ctx_res;
typedef struct drm_agp_mode struct_drm_agp_mode;
typedef struct drm_agp_info struct_drm_agp_info;
typedef struct drm_agp_buffer struct_drm_agp_buffer;
typedef struct drm_agp_binding struct_drm_agp_binding;
typedef struct drm_scatter_gather struct_drm_scatter_gather;
typedef union drm_wait_vblank union_drm_wait_vblank;
typedef struct drm_mode_get_connector struct_drm_mode_get_connector;
typedef struct drm_mode_fb_cmd2 struct_drm_mode_fb_cmd2;
typedef struct drm_mode_get_plane_res struct_drm_mode_get_plane_res;
typedef struct drm_mode_obj_get_properties struct_drm_mode_obj_get_properties;
typedef struct drm_mode_obj_set_property struct_drm_mode_obj_set_property;

#endif /* HAVE_DRM_H || HAVE_DRM_DRM_H */

#include MPERS_DEFS

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

# include "print_fields.h"

static int
drm_version(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_version ver;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");
	if (umove_or_printaddr(tcp, arg, &ver))
		return RVAL_IOCTL_DECODED;
	if (entering(tcp)) {
		PRINT_FIELD_U("{", ver, name_len);
		PRINT_FIELD_U(", ", ver, date_len);
		PRINT_FIELD_U(", ", ver, desc_len);
		tprints("}");

		return 0;
	}

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

static int
drm_unique(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_unique unique;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");
	if (umove_or_printaddr(tcp, arg, &unique))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", unique, unique_len);
	if (entering(tcp)) {
		tprints("}");

		return 0;
	}

	tprints(", unique=");
	printstrn(tcp, ptr_to_kulong(unique.unique), unique.unique_len);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_map_type.h"
# include "xlat/drm_map_flags.h"

static int
drm_get_map(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_map map;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &map))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_X("{", map, offset);
	if (entering(tcp)) {
		tprints("}");

		return 0;
	}

	PRINT_FIELD_U(", ", map, size);
	tprints(", type=");
	printxval(drm_map_type, map.type, "_DRM_???");
	tprints(", flags=");
	printxval(drm_map_flags, map.flags, "_DRM_???");
	PRINT_FIELD_PTR(", ", map, handle);
	PRINT_FIELD_D(", ", map, mtrr);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_get_client(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_client client;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &client))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_D("{", client, idx);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &client)) {
		PRINT_FIELD_D(", ", client, auth);
		PRINT_FIELD_U(", ", client, pid);
		PRINT_FIELD_U(", ", client, uid);
		PRINT_FIELD_U(", ", client, magic);
		PRINT_FIELD_U(", ", client, iocs);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_stat_type.h"

static int
drm_get_stats(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_stats stats;

	if (exiting(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &stats))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", stats, count);
		tprints(", data=[");

		for (unsigned int i = 0; i < 15; i++) {
			tprintf("%s%lu", i > 0 ? ", {value=" : "{value=", (unsigned long) stats.data[i].value);
			tprints(", type=");
			printxval(drm_stat_type, stats.data[i].type, "_DRM_STAT_???");
			tprints("}");
		}
		tprints("]}");

		return RVAL_IOCTL_DECODED;
	}

	return 0;
}

static int
drm_add_map(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_map map;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &map)) {
		PRINT_FIELD_X("{", map, offset);
		PRINT_FIELD_U(", ", map, size);
		tprints(", type=");
		printxval(drm_map_type, map.type, "_DRM_???");
		tprints(", flags");
		printxval(drm_map_flags, map.flags, "_DRM_???");
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_buf_desc_flags.h"

static int
drm_add_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_desc desc;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");
	if (umove_or_printaddr(tcp, arg, &desc))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_D("{", desc, count);
	PRINT_FIELD_D(", ", desc, size);
	if (entering(tcp)) {
		tprints(", flags=");
		printxval(drm_buf_desc_flags, desc.flags, "_DRM_???");
		PRINT_FIELD_X(", ", desc, agp_start);
		tprints("}");

		return 0;
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mark_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_desc desc;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &desc)) {
		PRINT_FIELD_D("{", desc, size);
		PRINT_FIELD_D(", ", desc, low_mark);
		PRINT_FIELD_D(", ", desc, high_mark);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_info_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_info info;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");
	if (umove_or_printaddr(tcp, arg, &info))
		return RVAL_IOCTL_DECODED;

	tprints("{");
	if (entering(tcp)) {
		PRINT_FIELD_PTR("", info, list);
		PRINT_FIELD_D(", ", info, count);
		tprints("}");

		return 0;
	}

	PRINT_FIELD_D("", info, count);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_map_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_map map;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &map))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_D("{", map, count);
		PRINT_FIELD_PTR(", ", map, virtual);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &map)) {
		PRINT_FIELD_PTR(", ", map, list);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_free_bufs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_buf_free free;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &free)) {
		PRINT_FIELD_D("{", free, count);
		PRINT_FIELD_PTR(", ", free, list);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_rm_map(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_map map;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &map)) {
		PRINT_FIELD_PTR("{", map, handle);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_sarea_ctx(struct tcb *const tcp, const kernel_ulong_t arg,
	      const bool is_get)
{
	struct_drm_ctx_priv_map map;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &map))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", map, ctx_id);
		if (!is_get)
			PRINT_FIELD_PTR(", ", map, handle);
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &map)) {
		if (is_get)
			PRINT_FIELD_PTR(", ", map, handle);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_res_ctx(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_ctx_res ctx;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");
	if (umove_or_printaddr(tcp, arg, &ctx))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_D("{", ctx, count);

	if (entering(tcp)) {
		tprints("}");

		return 0;
	}

	PRINT_FIELD_PTR(", ", ctx, contexts);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}


static int
drm_agp_enable(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_agp_mode mode;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &mode)) {
		PRINT_FIELD_U("{", mode, mode);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_agp_info(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_agp_info info;

	if (exiting(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &info))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_D("{", info, agp_version_major);
		PRINT_FIELD_D(", ", info, agp_version_minor);
		PRINT_FIELD_U(", ", info, mode);
		PRINT_FIELD_U(", ", info, aperture_base);
		PRINT_FIELD_U(", ", info, aperture_size);
		PRINT_FIELD_U(", ", info, memory_allowed);
		PRINT_FIELD_U(", ", info, memory_used);
		PRINT_FIELD_U(", ", info, id_vendor);
		PRINT_FIELD_U(", ", info, id_device);
		tprints("}");

		return RVAL_IOCTL_DECODED;
	}

	return 0;
}

static int
drm_agp_alloc(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_agp_buffer buffer;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &buffer))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", buffer, size);
		PRINT_FIELD_U(", ", buffer, type);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &buffer)) {
		PRINT_FIELD_U(", ", buffer, handle);
		PRINT_FIELD_U(", ", buffer, physical);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_agp_free(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_agp_buffer buffer;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &buffer)) {
		PRINT_FIELD_U("{", buffer, handle);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_agp_bind(struct tcb *const tcp, const kernel_ulong_t arg, bool is_bind)
{
	struct_drm_agp_binding binding;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &binding)) {
		PRINT_FIELD_U("{", binding, handle);
		if (is_bind)
			PRINT_FIELD_X(", ", binding, offset);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_scatter_gather(struct tcb *const tcp, const kernel_ulong_t arg,
		   bool is_alloc)
{
	struct_drm_scatter_gather sg;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &sg))
			return RVAL_IOCTL_DECODED;
		if (is_alloc)
			PRINT_FIELD_U("{", sg, size);
		else
			PRINT_FIELD_U("{", sg, handle);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &sg)) {
		if (is_alloc)
			PRINT_FIELD_U(", ", sg, handle);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_vblank_seq_type.h"

static int
drm_wait_vblank(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union_drm_wait_vblank vblank;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &vblank))
			return RVAL_IOCTL_DECODED;

		tprints("{request={type=");
		printxval(drm_vblank_seq_type, vblank.request.type, "_DRM_VBLANK_???");
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

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &con))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", con, connector_id);
	PRINT_FIELD_U(", ", con, count_encoders);

	if (entering(tcp)) {
		tprints("}");

		return 0;
	}

	PRINT_FIELD_PTR(", ", con, encoders_ptr);
	PRINT_FIELD_PTR(", ", con, modes_ptr);
	PRINT_FIELD_PTR(", ", con, props_ptr);
	PRINT_FIELD_PTR(", ", con, prop_values_ptr);
	PRINT_FIELD_U(", ", con, count_modes);
	PRINT_FIELD_U(", ", con, count_props);
	PRINT_FIELD_U(", ", con, encoder_id);
	PRINT_FIELD_U(", ", con, connector_type);
	PRINT_FIELD_U(", ", con, connector_type_id);
	PRINT_FIELD_U(", ", con, connection);
	PRINT_FIELD_U(", ", con, mm_width);
	PRINT_FIELD_U(", ", con, mm_height);
	PRINT_FIELD_U(", ", con, subpixel);

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
			"handles=[%u, %u, %u, %u], "
			"pitches=[%u, %u, %u, %u], "
			"offsets=[%u, %u, %u, %u]",
			cmd.width, cmd.height, cmd.pixel_format, cmd.flags,
			cmd.handles[0], cmd.handles[1], cmd.handles[2],
			cmd.handles[3], cmd.pitches[0], cmd.pitches[1],
			cmd.pitches[2], cmd.pitches[3], cmd.offsets[0],
			cmd.offsets[1], cmd.offsets[2], cmd.offsets[3]);
#ifdef HAVE_STRUCT_DRM_MODE_FB_CMD2_MODIFIER
		tprintf(", modifiers=[%" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 "]",
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

static int
drm_mode_getplaneresources(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_mode_get_plane_res res;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &res))
		return RVAL_IOCTL_DECODED;

	tprints("{");
	if (entering(tcp)) {
		PRINT_FIELD_PTR("", res, plane_id_ptr);
		PRINT_FIELD_U(", ", res, count_planes);
		tprints("}");

		return 0;
	}

	PRINT_FIELD_U("", res, count_planes);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_obj_getproperties(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_mode_obj_get_properties prop;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &prop))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_PTR("{", prop, props_ptr);
		PRINT_FIELD_PTR(", ", prop, prop_values_ptr);
		PRINT_FIELD_U(", ", prop, obj_id);
		PRINT_FIELD_U(", ", prop, obj_type);

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &prop)) {
		PRINT_FIELD_U(", ", prop, count_props);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_mode_obj_setproperty(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_drm_mode_obj_set_property prop;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &prop)) {
		PRINT_FIELD_U("{", prop, value);
		PRINT_FIELD_U(", ", prop, prop_id);
		PRINT_FIELD_U(", ", prop, obj_id);
		PRINT_FIELD_U(", ", prop, obj_type);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, drm_ioctl_mpers, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case DRM_IOCTL_VERSION: /* RW */
		return drm_version(tcp, arg);
	case DRM_IOCTL_GET_UNIQUE: /* RW */
	//case DRM_IOCTL_SET_UNIQUE: /* W */ /* invalid op */
		return drm_unique(tcp, arg);
	case DRM_IOCTL_GET_MAP: /* RW */
		return drm_get_map(tcp, arg);
	case DRM_IOCTL_GET_CLIENT: /* RW */
		return drm_get_client(tcp, arg);
	case DRM_IOCTL_GET_STATS: /* R */
		return drm_get_stats(tcp, arg);
	case DRM_IOCTL_ADD_MAP: /* RW */
		return drm_add_map(tcp, arg);
	case DRM_IOCTL_ADD_BUFS: /* RW */
		return drm_add_bufs(tcp, arg);
	case DRM_IOCTL_MARK_BUFS: /* W */
		return drm_mark_bufs(tcp, arg);
	case DRM_IOCTL_INFO_BUFS: /* RW */
		return drm_info_bufs(tcp, arg);
	case DRM_IOCTL_MAP_BUFS: /* RW */
		return drm_map_bufs(tcp, arg);
	case DRM_IOCTL_FREE_BUFS: /* W */
		return drm_free_bufs(tcp, arg);
	case DRM_IOCTL_RM_MAP: /* W */
		return drm_rm_map(tcp, arg);
	case DRM_IOCTL_SET_SAREA_CTX:
	case DRM_IOCTL_GET_SAREA_CTX:
		return drm_sarea_ctx(tcp, arg, code == DRM_IOCTL_GET_SAREA_CTX);
	case DRM_IOCTL_RES_CTX: /* RW */
		return drm_res_ctx(tcp, arg);
	case DRM_IOCTL_AGP_ENABLE: /* W */
		return drm_agp_enable(tcp, arg);
	case DRM_IOCTL_AGP_INFO: /* R */
		return drm_agp_info(tcp, arg);
	case DRM_IOCTL_AGP_ALLOC: /* RW */
		return drm_agp_alloc(tcp, arg);
	case DRM_IOCTL_AGP_FREE: /* W */
		return drm_agp_free(tcp, arg);
	case DRM_IOCTL_AGP_BIND: /* W */
	case DRM_IOCTL_AGP_UNBIND: /* W */
		return drm_agp_bind(tcp, arg, code == DRM_IOCTL_AGP_BIND);
	case DRM_IOCTL_SG_ALLOC: /* RW */
	case DRM_IOCTL_SG_FREE: /* W */
		return drm_scatter_gather(tcp, arg, code == DRM_IOCTL_SG_ALLOC);
	case DRM_IOCTL_WAIT_VBLANK: /* RW */
		return drm_wait_vblank(tcp, arg);
	case DRM_IOCTL_MODE_ADDFB2: /* RW */
		return drm_mode_add_fb2(tcp, arg);
	}
	/* variable length, so we can't use switch(code) to identify DRM_IOCTL_MODE_GETCONNECTOR */
	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_GETCONNECTOR)) {
		return drm_mode_get_connector(tcp, arg);
	}

	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_GETPLANERESOURCES)) {
		return drm_mode_getplaneresources(tcp, arg);
	}

	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_OBJ_GETPROPERTIES)) {
		return drm_mode_obj_getproperties(tcp, arg);
	}

	if (_IOC_NR(code) == _IOC_NR(DRM_IOCTL_MODE_OBJ_SETPROPERTY)) {
		return drm_mode_obj_setproperty(tcp, arg);
	}

	return RVAL_DECODED;
}

#endif /* HAVE_DRM_H || HAVE_DRM_DRM_H */

