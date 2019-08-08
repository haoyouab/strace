#include "tests.h"

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

# include <errno.h>
# include <fcntl.h>
# include <inttypes.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/ioctl.h>
# include "print_fields.h"

# ifdef HAVE_DRM_H
#  include <drm.h>
# else
#  include <drm/drm.h>
# endif

# include "xlat.h"
# include "xlat/drm_buf_desc_flags.h"
# include "xlat/drm_capability.h"
# include "xlat/drm_client_capability.h"
# include "xlat/drm_control_func.h"
# include "xlat/drm_crtc_sequence_flags.h"
# include "xlat/drm_ctx_flags.h"
# include "xlat/drm_lock_flags.h"
# include "xlat/drm_map_flags.h"
# include "xlat/drm_map_type.h"
# include "xlat/drm_modeset_cmd.h"
# include "xlat/drm_mode_atomic_flags.h"
# include "xlat/drm_mode_create_lease_flags.h"
# include "xlat/drm_mode_cursor_flags.h"
# include "xlat/drm_mode_encoder_type.h"
# include "xlat/drm_mode_fb_cmd2_flags.h"
# include "xlat/drm_mode_fb_dirty_cmd_flags.h"
# include "xlat/drm_mode_flags.h"
# include "xlat/drm_mode_get_property_flags.h"
# include "xlat/drm_mode_page_flip_flags.h"
# include "xlat/drm_mode_set_plane_flags.h"
# include "xlat/drm_mode_type.h"
# include "xlat/drm_prime_handle_flags.h"
# include "xlat/drm_syncobj_fd_to_handle_flags.h"
# include "xlat/drm_syncobj_flags.h"
# include "xlat/drm_syncobj_handle_to_fd_flags.h"
# include "xlat/drm_syncobj_wait_flags.h"
# include "xlat/drm_vblank_seq_type.h"
# include "xlat/drm_vblank_seq_type_flags.h"


static const unsigned int magic = 0xdeadbeef;
static const unsigned long lmagic = (unsigned long) 0xdeadbeefbadc0dedULL;
static const char *errstr;

struct drm_check {
	unsigned long cmd;
	const char *cmd_str;
	void *arg_ptr;
	void (*print_arg)(long rc, void *ptr, void *arg);
};

struct drm_mode_rm_fb_wrap {
	unsigned int fb_id;
};

static void
printaddr64(uint64_t addr)
{
	if (!addr)
		printf("NULL");
	else
		printf("%#" PRIx64, addr);
}

static long
invoke_test_syscall(unsigned long cmd, void *p)
{
	long rc = ioctl(-1, cmd, p);
	errstr = sprintrc(rc);
	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
	return rc;
}

static void
test_drm(struct drm_check *check, void *arg)
{

	long rc = invoke_test_syscall(check->cmd, check->arg_ptr);
	printf("ioctl(-1, %s, ", check->cmd_str);
	if (check->print_arg)
		check->print_arg(rc, check->arg_ptr, arg);
	else
		printf("%p", check->arg_ptr);
	printf(") = %s\n", errstr);
}

static void
print_drm_version(long rc, void *ptr, void *arg)
{
	struct drm_version *ver = ptr;

	if (rc < 0) {
		printf("%p", ver);
		return;
	}
	PRINT_FIELD_U("{", *ver, name_len);
	PRINT_FIELD_U(", ", *ver, date_len);
	PRINT_FIELD_U(", ", *ver, desc_len);
	PRINT_FIELD_D("} => {", *ver, version_major);
	PRINT_FIELD_D(", ", *ver, version_minor);
	PRINT_FIELD_D(", ", *ver, version_patchlevel);
	PRINT_FIELD_U(", ", *ver, name_len);
	printf(", name=\"%s\"", ver->name);
	PRINT_FIELD_U(", ", *ver, date_len);
	printf(", date=\"%s\"", ver->date);
	PRINT_FIELD_U(", ", *ver, desc_len);
	printf(", desc=\"%s\"", ver->desc);
	printf("}");
}

static void
print_drm_get_unique(long rc, void *ptr, void *arg)
{
	struct drm_unique *unique = ptr;

	if (rc < 0) {
		printf("%p", unique);
		return;
	}
	PRINT_FIELD_U("{", *unique, unique_len);
	PRINT_FIELD_U("} => {", *unique, unique_len);
	printf(", unique=\"%s\"", unique->unique);
	printf("}");
}

static void
print_drm_get_magic(long rc, void *ptr, void *arg)
{
	struct drm_auth *auth = ptr;

	if (rc < 0) {
		printf("%p", auth);
		return;
	}
	PRINT_FIELD_U("{", *auth, magic);
	printf("}");
}

static void
print_drm_irq_busid(long rc, void *ptr, void *arg)
{
	struct drm_irq_busid *busid = ptr;

	if (rc < 0) {
		printf("%p", busid);
		return;
	}
	PRINT_FIELD_D("{", *busid, busnum);
	PRINT_FIELD_D(", ", *busid, devnum);
	PRINT_FIELD_D(", ", *busid, funcnum);
	PRINT_FIELD_D(", ", *busid, irq);
	printf("}");
}

static void
print_drm_get_map(long rc, void *ptr, void *arg)
{
	struct drm_map *map = ptr;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	PRINT_FIELD_X("{", *map, offset);
	PRINT_FIELD_X("} => {", *map, offset);
	PRINT_FIELD_U(", ", *map, size);
	printf(", type=");
	printxval(drm_map_type, map->type, "_DRM_???");
	printf(", flags=");
	printflags(drm_map_flags, map->flags, "_DRM_???");
	printf(", handle=%p", map->handle);
	PRINT_FIELD_D(", ", *map, mtrr);
	printf("}");
}

static void
print_drm_get_client(long rc, void *ptr, void *arg)
{
	struct drm_client *client = ptr;

	if (rc < 0) {
		printf("%p", client);
		return;
	}
	PRINT_FIELD_D("{", *client, idx);
	PRINT_FIELD_D(", ", *client, auth);
	PRINT_FIELD_U(", ", *client, pid);
	PRINT_FIELD_UID(", ", *client, uid);
	PRINT_FIELD_U(", ", *client, magic);
	PRINT_FIELD_U(", ", *client, iocs);
	printf("}");
}

static void
print_drm_set_version(long rc, void *ptr, void *arg)
{
	struct drm_set_version *ver = ptr;

	if (rc < 0) {
		printf("%p", ver);
		return;
	}
	PRINT_FIELD_D("{", *ver, drm_di_major);
	PRINT_FIELD_D(", ", *ver, drm_di_minor);
	PRINT_FIELD_D(", ", *ver, drm_dd_major);
	PRINT_FIELD_D(", ", *ver, drm_dd_minor);
	PRINT_FIELD_D("} => {", *ver, drm_di_major);
	PRINT_FIELD_D(", ", *ver, drm_di_minor);
	PRINT_FIELD_D(", ", *ver, drm_dd_major);
	PRINT_FIELD_D(", ", *ver, drm_dd_minor);
	printf("}");

}

static void
print_drm_modeset_ctl(long rc, void *ptr, void *arg)
{
	struct drm_modeset_ctl *ctl = ptr;

	if (rc < 0) {
		printf("%p", ctl);
		return;
	}
	PRINT_FIELD_U("{", *ctl, crtc);
	printf(", cmd=");
	printxval(drm_modeset_cmd, ctl->cmd, "_DRM_???");
	printf("}");
}

static void
print_drm_gem_close(long rc, void *ptr, void *arg)
{
	struct drm_gem_close *close = ptr;

	if (rc < 0) {
		printf("%p", close);
		return;
	}
	PRINT_FIELD_U("{", *close, handle);
	printf("}");
}

static void
print_drm_gem_flink(long rc, void *ptr, void *arg)
{
	struct drm_gem_flink *flink = ptr;

	if (rc < 0) {
		printf("%p", flink);
		return;
	}
	PRINT_FIELD_U("{", *flink, handle);
	PRINT_FIELD_U(", ", *flink, name);
	printf("}");
}

static void
print_drm_gem_open(long rc, void *ptr, void *arg)
{
	struct drm_gem_open *open = ptr;

	if (rc < 0) {
		printf("%p", open);
		return;
	}
	PRINT_FIELD_U("{", *open, name);
	PRINT_FIELD_U(", ", *open, handle);
	PRINT_FIELD_U(", ", *open, size);
	printf("}");
}

static void
print_drm_get_cap(long rc, void *ptr, void *arg)
{
	struct drm_get_cap *cap = ptr;

	if (rc < 0) {
		printf("%p", cap);
		return;
	}
	printf("{capability=");
	printxval(drm_capability, cap->capability, "DRM_CAP_???");
	PRINT_FIELD_U(", ", *cap, value);
	printf("}");
}

static void
print_drm_set_client_cap(long rc, void *ptr, void *arg)
{
	struct drm_set_client_cap *cap = ptr;

	if (rc < 0) {
		printf("%p", cap);
		return;
	}
	printf("{capability=");
	printxval(drm_client_capability, cap->capability, "DRM_CLIENT_CAP_???");
	PRINT_FIELD_U(", ", *cap, value);
	printf("}");
}

static void
print_drm_auth_magic(long rc, void *ptr, void *arg)
{
	struct drm_auth *auth = ptr;

	if (rc < 0) {
		printf("%p", auth);
		return;
	}
	PRINT_FIELD_U("{", *auth, magic);
	printf("}");
}

static void
print_drm_block(long rc, void *ptr, void *arg)
{
	struct drm_block *block = ptr;

	printf("%p", block);
}

static void
print_drm_unblock(long rc, void *ptr, void *arg)
{
	struct drm_block *block = ptr;

	printf("%p", block);
}

static void
print_drm_control(long rc, void *ptr, void *arg)
{
	struct drm_control *control = ptr;

	if (rc < 0) {
		printf("%p", control);
		return;
	}
	printf("{func=");
	printxval(drm_control_func, control->func, "DRM_???");
	PRINT_FIELD_D(", ", *control, irq);
	printf("}");
}

static void
print_drm_add_map(long rc, void *ptr, void *arg)
{
	struct drm_map *map = ptr;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	PRINT_FIELD_X("{", *map, offset);
	PRINT_FIELD_U(", ", *map, size);
	printf(", type=");
	printxval(drm_map_type, map->type, "_DRM_???");
	printf(", flags=");
	printflags(drm_map_flags, map->flags, "_DRM_???");
	printf("}");
}

static void
print_drm_add_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_desc *desc = ptr;

	if (rc < 0) {
		printf("%p", desc);
		return;
	}
	PRINT_FIELD_D("{", *desc, count);
	PRINT_FIELD_D(", ", *desc, size);
	printf(", flags=");
	printflags(drm_buf_desc_flags, desc->flags, "_DRM_???");
	PRINT_FIELD_X(", ", *desc, agp_start);
	PRINT_FIELD_D("} => {", *desc, count);
	PRINT_FIELD_D(", ", *desc, size);
	printf("}");
}

static void
print_drm_mark_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_desc *desc = ptr;

	if (rc < 0) {
		printf("%p", desc);
		return;
	}
	PRINT_FIELD_D("{", *desc, size);
	PRINT_FIELD_D(", ", *desc, low_mark);
	PRINT_FIELD_D(", ", *desc, high_mark);
	printf("}");
}

static void
print_drm_info_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_info *info = ptr;

	if (rc < 0) {
		printf("%p", info);
		return;
	}
	printf("{list=%p", info->list);
	PRINT_FIELD_D(", ", *info, count);
	PRINT_FIELD_D("} => {", *info, count);
	printf("}");
}

static void
print_drm_map_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_map *map = ptr;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	PRINT_FIELD_D("{", *map, count);
	printf(", virtual=%p", map->virtual);
	printf(", list=%p", map->list);
	printf("}");
}

static void
print_drm_free_bufs(long rc, void *ptr, void *arg)
{
	struct drm_buf_free *free = ptr;

	if (rc < 0) {
		printf("%p", free);
		return;
	}
	PRINT_FIELD_D("{", *free, count);
	printf(", list=%p", free->list);
	printf("}");
}

static void
print_drm_rm_map(long rc, void *ptr, void *arg)
{
	struct drm_map *map = ptr;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	printf("{handle=%p}", map->handle);
}

static void
print_drm_sarea_ctx(long rc, void *ptr, void *arg)
{
	struct drm_ctx_priv_map *map = ptr;

	if (rc < 0) {
		printf("%p", map);
		return;
	}
	PRINT_FIELD_U("{", *map, ctx_id);
	printf(", handle=%p}", map->handle);
}


static void
print_drm_ctx(long rc, void *ptr, void *arg)
{
	struct drm_ctx *ctx = ptr;

	if (rc < 0) {
		printf("%p", ctx);
		return;
	}
	PRINT_FIELD_U("{", *ctx, handle);
	printf("}");
}

static void
print_drm_get_ctx(long rc, void *ptr, void *arg)
{
	struct drm_ctx *ctx = ptr;

	if (rc < 0) {
		printf("%p", ctx);
		return;
	}
	printf("{flags=");
	printflags(drm_ctx_flags, ctx->flags, "_DRM_CONTEXT_???");
	printf("}");
}

static void
print_drm_res_ctx(long rc, void *ptr, void *arg)
{
	struct drm_ctx_res *res = ptr;

	if (rc < 0) {
		printf("%p", res);
		return;
	}
	PRINT_FIELD_D("{", *res, count);
	PRINT_FIELD_D("} => {", *res, count);
	printf(", contexts=%p", res->contexts);
	printf("}");
}

static void
print_drm_lock(long rc, void *ptr, void *arg)
{
	struct drm_lock *lock = ptr;

	if (rc < 0) {
		printf("%p", lock);
		return;
	}
	PRINT_FIELD_D("{", *lock, context);
	printf(", flags=");
	printflags(drm_lock_flags, lock->flags, "_DRM_LOCK_???");
	printf("}");
}

static void
print_drm_prime_handle_to_fd(long rc, void *ptr, void *arg)
{
	struct drm_prime_handle *handle = ptr;

	if (rc < 0) {
		printf("%p", handle);
		return;
	}
	PRINT_FIELD_U("{", *handle, handle);
	printf(", flags=");
	printflags(drm_prime_handle_flags, handle->flags, "DRM_???");
	PRINT_FIELD_D(", ", *handle, fd);
	printf("}");
}

static void
print_drm_prime_fd_to_handle(long rc, void *ptr, void *arg)
{
	struct drm_prime_handle *handle = ptr;

	if (rc < 0) {
		printf("%p", handle);
		return;
	}
	PRINT_FIELD_D("{", *handle, fd);
	PRINT_FIELD_U(", ", *handle, handle);
	printf("}");
}

static void
print_drm_agp_enable(long rc, void *ptr, void *arg)
{
	struct drm_agp_mode *mode = ptr;

	if (rc < 0) {
		printf("%p", mode);
		return;
	}
	PRINT_FIELD_U("{", *mode, mode);
	printf("}");
}

static void
print_drm_agp_info(long rc, void *ptr, void *arg)
{
	struct drm_agp_info *info = ptr;

	if (rc < 0) {
		printf("%p", info);
		return;
	}
	PRINT_FIELD_D("{", *info, agp_version_major);
	PRINT_FIELD_D(", ", *info, agp_version_minor);
	PRINT_FIELD_U(", ", *info, mode);
	PRINT_FIELD_X(", ", *info, aperture_base);
	PRINT_FIELD_U(", ", *info, aperture_size);
	PRINT_FIELD_U(", ", *info, memory_allowed);
	PRINT_FIELD_U(", ", *info, memory_used);
	PRINT_FIELD_X(", ", *info, id_vendor);
	PRINT_FIELD_X(", ", *info, id_device);
	printf("}");
}

static void
print_drm_agp_alloc(long rc, void *ptr, void *arg)
{
	struct drm_agp_buffer *buffer = ptr;

	if (rc < 0) {
		printf("%p", buffer);
		return;
	}
	PRINT_FIELD_U("{", *buffer, size);
	PRINT_FIELD_U(", ", *buffer, type);
	PRINT_FIELD_U(", ", *buffer, handle);
	PRINT_FIELD_U(", ", *buffer, physical);
	printf("}");
}

static void
print_drm_agp_free(long rc, void *ptr, void *arg)
{
	struct drm_agp_buffer *buffer = ptr;

	if (rc < 0) {
		printf("%p", buffer);
		return;
	}
	PRINT_FIELD_U("{", *buffer, handle);
	printf("}");
}

static void
print_drm_agp_bind(long rc, void *ptr, void *arg)
{
	struct drm_agp_binding *binding = ptr;

	if (rc < 0) {
		printf("%p", binding);
		return;
	}
	PRINT_FIELD_U("{", *binding, handle);
	PRINT_FIELD_X(", ", *binding, offset);
	printf("}");
}

static void
print_drm_agp_unbind(long rc, void *ptr, void *arg)
{
	struct drm_agp_binding *binding = ptr;

	if (rc < 0) {
		printf("%p", binding);
		return;
	}
	PRINT_FIELD_U("{", *binding, handle);
	printf("}");
}

static void
print_drm_sg_alloc(long rc, void *ptr, void *arg)
{
	struct drm_scatter_gather *sg = ptr;

	if (rc < 0) {
		printf("%p", sg);
		return;
	}
	PRINT_FIELD_U("{", *sg, size);
	PRINT_FIELD_U(", ", *sg, handle);
	printf("}");
}

static void
print_drm_sg_free(long rc, void *ptr, void *arg)
{
	struct drm_scatter_gather *sg = ptr;

	if (rc < 0) {
		printf("%p", sg);
		return;
	}
	PRINT_FIELD_U("{", *sg, handle);
	printf("}");
}

static void
print_drm_wait_vblank(long rc, void *ptr, void *arg)
{
	union drm_wait_vblank *vblank = ptr;

	if (rc < 0) {
		printf("%p", vblank);
		return;
	}
	printf("{request={type=");
	printxval(drm_vblank_seq_type,
		  vblank->request.type & _DRM_VBLANK_TYPES_MASK,
		  "_DRM_VBLANK_???");
	printf(", type.flags=");
	printflags(drm_vblank_seq_type_flags,
		   vblank->request.type & _DRM_VBLANK_FLAGS_MASK,
		   "_DRM_VBLANK_???");
	PRINT_FIELD_U(", ", vblank->request, sequence);
	PRINT_FIELD_U(", ", vblank->request, signal);
	printf("}");

	printf(", {reply={type=");
	printxval(drm_vblank_seq_type,
		  vblank->reply.type & _DRM_VBLANK_TYPES_MASK,
		  "_DRM_VBLANK_???");
	printf(", type.flags=");
	printflags(drm_vblank_seq_type_flags,
		   vblank->reply.type & _DRM_VBLANK_FLAGS_MASK,
		   "_DRM_VBLANK_???");
	PRINT_FIELD_U(", ", vblank->reply, sequence);
	PRINT_FIELD_D(", ", vblank->reply, tval_sec);
	PRINT_FIELD_D(", ", vblank->reply, tval_usec);
	printf("}}");
}

# ifdef DRM_IOCTL_CRTC_GET_SEQUENCE
static void
print_drm_crtc_get_sequence(long rc, void *ptr, void *arg)
{
	struct drm_crtc_get_sequence *seq = ptr;

	if (rc < 0) {
		printf("%p", seq);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *seq, crtc_id);
	PRINT_FIELD_U(", ", *seq, active);
	PRINT_FIELD_U(", ", *seq, sequence);
	PRINT_FIELD_D(", ", *seq, sequence_ns);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_CRTC_QUEUE_SEQUENCE
static void
print_drm_crtc_queue_sequence(long rc, void *ptr, void *arg)
{
	struct drm_crtc_queue_sequence *seq = ptr;

	if (rc < 0) {
		printf("%p", seq);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *seq, crtc_id);
	printf(", flags=");
	printflags(drm_crtc_sequence_flags, seq->flags, "DRM_CRTC_SEQUENCE_???");
	PRINT_FIELD_X(", ", *seq, user_data);
	PRINT_FIELD_U(", ", *seq, sequence);
	PRINT_FIELD_U("} => {", *seq, sequence);
	printf("}");
}
# endif

static void
print_drm_mode_get_resources(long rc, void *ptr, void *arg)
{
	struct drm_mode_card_res *res = ptr;

	if (rc < 0) {
		printf("%p", res);
		return;
	}
	printf("{fb_id_ptr=[]");
	printf(", crtc_id_ptr=[]");
	printf(", connector_id_ptr=[]");
	printf(", encoder_id_ptr=[]");
	PRINT_FIELD_U(", ", *res, count_fbs);
	PRINT_FIELD_U(", ", *res, count_crtcs);
	PRINT_FIELD_U(", ", *res, count_connectors);
	PRINT_FIELD_U(", ", *res, count_encoders);
	PRINT_FIELD_U("} => {", *res, count_fbs);
	PRINT_FIELD_U(", ", *res, count_crtcs);
	PRINT_FIELD_U(", ", *res, count_connectors);
	PRINT_FIELD_U(", ", *res, count_encoders);
	PRINT_FIELD_U(", ", *res, min_width);
	PRINT_FIELD_U(", ", *res, max_width);
	PRINT_FIELD_U(", ", *res, min_height);
	PRINT_FIELD_U(", ", *res, max_height);
	printf("}");
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
	printf(", flags=");
	printflags(drm_mode_flags, info->flags, "DRM_MODE_FLAG_PIC_???");
	printf(", type=");
	printxval(drm_mode_type, info->type, "DRM_MODE_TYPE_???");
	printf(", name=\"%s\"", info->name);
}

static void
print_drm_mode_crtc(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc *crtc = ptr;
	int *is_get = arg;

	if (rc < 0) {
		printf("%p", crtc);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *crtc, crtc_id);
	if (!*is_get) {
		printf(", set_connectors_ptr=[]");
		PRINT_FIELD_U(", ", *crtc, count_connectors);
	}
	PRINT_FIELD_U(", ", *crtc, fb_id);
	PRINT_FIELD_U(", ", *crtc, x);
	PRINT_FIELD_U(", ", *crtc, y);
	PRINT_FIELD_U(", ", *crtc, gamma_size);
	PRINT_FIELD_U(", ", *crtc, mode_valid);
	printf(", mode={");
	drm_mode_print_modeinfo(&crtc->mode);
	printf("}}");
}

static void
print_drm_mode_cursor(long rc, void *ptr, void *arg)
{
	struct drm_mode_cursor *cursor = ptr;

	if (rc < 0) {
		printf("%p", cursor);
		return;
	}
	printf("{flags=");
	printflags(drm_mode_cursor_flags, cursor->flags,
		   "DRM_MODE_CURSOR_???");
	PRINT_FIELD_DRM_CRTC_ID(", ", *cursor, crtc_id);
	PRINT_FIELD_D(", ", *cursor, x);
	PRINT_FIELD_D(", ", *cursor, y);
	PRINT_FIELD_U(", ", *cursor, width);
	PRINT_FIELD_U(", ", *cursor, height);
	PRINT_FIELD_U(", ", *cursor, handle);
	printf("}");
}

static void
print_drm_mode_get_gamma(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc_lut *lut = ptr;

	if (rc < 0) {
		printf("%p", lut);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *lut, crtc_id);
	PRINT_FIELD_U(", ", *lut, gamma_size);
	printf(", red=%#llx", (unsigned long long) lut->red);
	printf(", green=%#llx", (unsigned long long) lut->green);
	printf(", blue=%#llx", (unsigned long long) lut->blue);
	printf("}");
}

static void
print_drm_mode_set_gamma(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc_lut *lut = ptr;

	if (rc < 0) {
		printf("%p", lut);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *lut, crtc_id);
	PRINT_FIELD_U(", ", *lut, gamma_size);
	printf(", red=%#llx", (unsigned long long) lut->red);
	printf(", green=%#llx", (unsigned long long) lut->green);
	printf(", blue=%#llx", (unsigned long long) lut->blue);
	printf("}");
}

static void
print_drm_mode_get_encoder(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_encoder *enc = ptr;

	if (rc < 0) {
		printf("%p", enc);
		return;
	}
	PRINT_FIELD_U("{", *enc, encoder_id);
	PRINT_FIELD_U("} => {", *enc, encoder_id);
	printf(", encoder_type=");
	printxval(drm_mode_encoder_type, enc->encoder_type, "DRM_MODE_ENCODER_???");
	PRINT_FIELD_DRM_CRTC_ID(", ", *enc, crtc_id);
	PRINT_FIELD_X(", ", *enc, possible_crtcs);
	PRINT_FIELD_X(", ", *enc, possible_clones);
	printf("}");
}

static void
print_drm_mode_get_connector(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_connector *con = ptr;

	if (rc < 0) {
		printf("%p", con);
		return;
	}
	PRINT_FIELD_U("{", *con, connector_id);
	PRINT_FIELD_U(", ", *con, count_modes);
	PRINT_FIELD_U(", ", *con, count_props);
	PRINT_FIELD_U(", ", *con, count_encoders);
	PRINT_FIELD_U("} => {", *con, connector_id);
	PRINT_FIELD_U(", ", *con, count_modes);
	PRINT_FIELD_U(", ", *con, count_props);
	PRINT_FIELD_U(", ", *con, count_encoders);
	printf(", encoders_ptr=[]");
	printf(", modes_ptr=[]");
	printf(", props_ptr=[]");
	printf(", prop_values_ptr=[]");
	PRINT_FIELD_U(", ", *con, encoder_id);
	PRINT_FIELD_U(", ", *con, connector_type);
	PRINT_FIELD_U(", ", *con, connector_type_id);
	PRINT_FIELD_U(", ", *con, connection);
	PRINT_FIELD_U(", ", *con, mm_width);
	PRINT_FIELD_U(", ", *con, mm_height);
	PRINT_FIELD_U(", ", *con, subpixel);
	printf("}");
}

static void
print_drm_mode_get_property(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_property *prop = ptr;

	if (rc < 0) {
		printf("%p", prop);
		return;
	}
	PRINT_FIELD_U("{", *prop, prop_id);
	PRINT_FIELD_U(", ", *prop, count_values);
	PRINT_FIELD_U(", ", *prop, count_enum_blobs);
	printf("} => {values_ptr=[]");
	printf(", enum_blob_ptr=[]");
	printf(", flags=");
	printflags(drm_mode_get_property_flags, prop->flags,
		   "DRM_MODE_PROP_???");
	PRINT_FIELD_CSTRING(", ", *prop, name);
	PRINT_FIELD_U(", ", *prop, count_values);
	PRINT_FIELD_U(", ", *prop, count_enum_blobs);
	printf("}");
}

static void
print_drm_mode_set_property(long rc, void *ptr, void *arg)
{
	struct drm_mode_connector_set_property *prop = ptr;

	if (rc < 0) {
		printf("%p", prop);
		return;
	}
	PRINT_FIELD_U("{", *prop, value);
	PRINT_FIELD_U(", ", *prop, prop_id);
	PRINT_FIELD_U(", ", *prop, connector_id);
	printf("}");
}

static void
print_drm_mode_get_prop_blob(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_blob *blob = ptr;

	if (rc < 0) {
		printf("%p", blob);
		return;
	}
	PRINT_FIELD_U("{", *blob, blob_id);
	PRINT_FIELD_U(", ", *blob, length);
	PRINT_FIELD_U("} => {", *blob, length);
	PRINT_FIELD_ADDR64(", ", *blob, data);
	printf("}");
}

static void
print_drm_mode_get_fb(long rc, void *ptr, void *arg)
{
	struct drm_mode_fb_cmd *cmd = ptr;

	if (rc < 0) {
		printf("%p", cmd);
		return;
	}
	PRINT_FIELD_U("{", *cmd, fb_id);
	PRINT_FIELD_U(", ", *cmd, width);
	PRINT_FIELD_U(", ", *cmd, height);
	PRINT_FIELD_U(", ", *cmd, pitch);
	PRINT_FIELD_U(", ", *cmd, bpp);
	PRINT_FIELD_U(", ", *cmd, depth);
	PRINT_FIELD_U(", ", *cmd, handle);
	printf("}");

}

static void
print_drm_mode_add_fb(long rc, void *ptr, void *arg)
{
	struct drm_mode_fb_cmd *cmd = ptr;

	if (rc < 0) {
		printf("%p", cmd);
		return;
	}
	PRINT_FIELD_U("{", *cmd, width);
	PRINT_FIELD_U(", ", *cmd, height);
	PRINT_FIELD_U(", ", *cmd, pitch);
	PRINT_FIELD_U(", ", *cmd, bpp);
	PRINT_FIELD_U(", ", *cmd, depth);
	PRINT_FIELD_U(", ", *cmd, handle);
	PRINT_FIELD_U(", ", *cmd, fb_id);
	printf("}");
}

static void
print_drm_mode_rm_fb(long rc, void *ptr, void *arg)
{
	//TODO unsigned int *handle = ptr;
	struct drm_mode_rm_fb_wrap *wrap = ptr;

	if (rc < 0) {
		printf("%p", wrap);
		return;
	}
	printf("{fb_id=%u}", wrap->fb_id);
}

static void
print_drm_mode_page_flip(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc_page_flip *flip = ptr;

	if (rc < 0) {
		printf("%p", flip);
		return;
	}
	PRINT_FIELD_DRM_CRTC_ID("{", *flip, crtc_id);
	PRINT_FIELD_U(", ", *flip, fb_id);
	printf(", flags=");
	printflags(drm_mode_page_flip_flags, flip->flags, "DRM_MODE_PAGE_FLIP_???");
	PRINT_FIELD_X(", ", *flip, user_data);
	printf("}");
}

static void
print_drm_mode_dirty_fb(long rc, void *ptr, void *arg)
{
	struct drm_mode_fb_dirty_cmd *cmd = ptr;

	if (rc < 0) {
		printf("%p", cmd);
		return;
	}
	PRINT_FIELD_U("{", *cmd, fb_id);
	printf(", flags=");
	printflags(drm_mode_fb_dirty_cmd_flags, cmd->flags,
		   "DRM_MODE_FB_DIRTY_ANNOTATE_???");
	PRINT_FIELD_X(", ", *cmd, color);
	PRINT_FIELD_U(", ", *cmd, num_clips);
	printf(", clips_ptr=[]");
	printf("}");
}

static void
print_drm_mode_create_dumb(long rc, void *ptr, void *arg)
{
	struct drm_mode_create_dumb *dumb = ptr;

	if (rc < 0) {
		printf("%p", dumb);
		return;
	}
	PRINT_FIELD_U("{", *dumb, width);
	PRINT_FIELD_U(", ", *dumb, height);
	PRINT_FIELD_U(", ", *dumb, bpp);
	PRINT_FIELD_X(", ", *dumb, flags);
	PRINT_FIELD_U(", ", *dumb, handle);
	PRINT_FIELD_U(", ", *dumb, pitch);
	PRINT_FIELD_U(", ", *dumb, size);
	printf("}");
}

static void
print_drm_mode_map_dumb(long rc, void *ptr, void *arg)
{
	struct drm_mode_map_dumb *dumb = ptr;

	if (rc < 0) {
		printf("%p", dumb);
		return;
	}
	PRINT_FIELD_U("{", *dumb, handle);
	PRINT_FIELD_U(", ", *dumb, offset);
	printf("}");
}

static void
print_drm_mode_destroy_dumb(long rc, void *ptr, void *arg)
{
	struct drm_mode_destroy_dumb *dumb = ptr;

	if (rc < 0) {
		printf("%p", dumb);
		return;
	}
	PRINT_FIELD_U("{", *dumb, handle);
	printf("}");
}

static void
print_drm_mode_getplaneresources(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_plane_res *res = ptr;

	if (rc < 0) {
		printf("%p", res);
		return;
	}
	//printf("{plane_id_ptr=%#llx", (unsigned long long) res->plane_id_ptr);
	/* TODO: print array */
	printf("{plane_id_ptr=[]");
	PRINT_FIELD_U(", ", *res, count_planes);
	PRINT_FIELD_U("} => {", *res, count_planes);
	printf("}");
}

static void
print_drm_mode_getplane(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_plane *plane = ptr;

	if (rc < 0) {
		printf("%p", plane);
		return;
	}
	PRINT_FIELD_U("{", *plane, plane_id);
	PRINT_FIELD_U(", ", *plane, count_format_types);
	printf(", format_type_ptr=[]");
	PRINT_FIELD_U("} => {", *plane, plane_id);
	PRINT_FIELD_DRM_CRTC_ID(", ", *plane, crtc_id);
	PRINT_FIELD_U(", ", *plane, fb_id);
	PRINT_FIELD_U(", ", *plane, possible_crtcs);
	PRINT_FIELD_U(", ", *plane, gamma_size);
	PRINT_FIELD_U(", ", *plane, count_format_types);
	printf("}");
}

static void
print_drm_mode_setplane(long rc, void *ptr, void *arg)
{
	struct drm_mode_set_plane *plane = ptr;

	if (rc < 0) {
		printf("%p", plane);
		return;
	}
	PRINT_FIELD_U("{", *plane, plane_id);
	PRINT_FIELD_DRM_CRTC_ID(", ", *plane, crtc_id);
	PRINT_FIELD_U(", ", *plane, fb_id);
	printf(", flags=");
	printflags(drm_mode_set_plane_flags, plane->flags,
		   "DRM_MODE_PRESENT_???");
	PRINT_FIELD_D(", ", *plane, crtc_x);
	PRINT_FIELD_D(", ", *plane, crtc_y);
	PRINT_FIELD_U(", ", *plane, crtc_w);
	PRINT_FIELD_U(", ", *plane, crtc_h);
	PRINT_FIELD_U(", ", *plane, src_x);
	printf(" /* %u.%06u */", plane->src_x >> 16,
	       ((plane->src_x & 0xffff) * 15625) >> 10);
	PRINT_FIELD_U(", ", *plane, src_y);
	printf(" /* %u.%06u */", plane->src_y >> 16,
	       ((plane->src_y & 0xffff) * 15625) >> 10);
	PRINT_FIELD_U(", ", *plane, src_h);
	printf(" /* %u.%06u */", plane->src_h >> 16,
	       ((plane->src_h & 0xffff) * 15625) >> 10);
	PRINT_FIELD_U(", ", *plane, src_w);
	printf(" /* %u.%06u */", plane->src_w >> 16,
	       ((plane->src_w & 0xffff) * 15625) >> 10);
	printf("}");
}

static void
print_drm_mode_add_fb2(long rc, void *ptr, void *arg)
{
	struct drm_mode_fb_cmd2 *cmd = ptr;

	if (rc < 0) {
		printf("%p", cmd);
		return;
	}
	PRINT_FIELD_U("{", *cmd, width);
	PRINT_FIELD_U(", ", *cmd, height);
	PRINT_FIELD_X(", ", *cmd, pixel_format);
	printf(", flags=");
	printflags(drm_mode_fb_cmd2_flags, cmd->flags, "DRM_MODE_FB_???");
	printf(", handles=[%u, %u, %u, %u], "
	       "pitches=[%u, %u, %u, %u], "
	       "offsets=[%u, %u, %u, %u]",
	       cmd->handles[0], cmd->handles[1], cmd->handles[2],
	       cmd->handles[3], cmd->pitches[0], cmd->pitches[1],
	       cmd->pitches[2], cmd->pitches[3], cmd->offsets[0],
	       cmd->offsets[1], cmd->offsets[2], cmd->offsets[3]);
# ifdef HAVE_STRUCT_DRM_MODE_FB_CMD2_MODIFIER
	printf(", modifiers=[%" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 "]",
	       (uint64_t) cmd->modifier[0], (uint64_t) cmd->modifier[1],
	       (uint64_t) cmd->modifier[2], (uint64_t) cmd->modifier[3]);
# endif
	PRINT_FIELD_U(", ", *cmd, fb_id);
	printf("}");
}

static void
print_drm_mode_obj_getproperties(long rc, void *ptr, void *arg)
{
	struct drm_mode_obj_get_properties *props = ptr;

	if (rc < 0) {
		printf("%p", props);
		return;
	}
	printf("{props_ptr=[]");
	printf(", prop_values_ptr=[]");
	PRINT_FIELD_U(", ", *props, count_props);
	PRINT_FIELD_U(", ", *props, obj_id);
	PRINT_FIELD_U(", ", *props, obj_type);
	PRINT_FIELD_U("} => {", *props, count_props);
	printf("}");
}

static void
print_drm_mode_obj_setproperty(long rc, void *ptr, void *arg)
{
	struct drm_mode_obj_set_property *prop = ptr;

	if (rc < 0) {
		printf("%p", prop);
		return;
	}
	PRINT_FIELD_U("{", *prop, value);
	PRINT_FIELD_U(", ", *prop, prop_id);
	PRINT_FIELD_U(", ", *prop, obj_id);
	PRINT_FIELD_U(", ", *prop, obj_type);
	printf("}");
}

static void
print_drm_mode_cursor2(long rc, void *ptr, void *arg)
{
	struct drm_mode_cursor2 *cursor2 = ptr;

	if (rc < 0) {
		printf("%p", cursor2);
		return;
	}
	printf("{flags=");
	printflags(drm_mode_cursor_flags, cursor2->flags,
		   "DRM_MODE_CURSOR_???");
	PRINT_FIELD_DRM_CRTC_ID(", ", *cursor2, crtc_id);
	PRINT_FIELD_D(", ", *cursor2, x);
	PRINT_FIELD_D(", ", *cursor2, y);
	PRINT_FIELD_U(", ", *cursor2, width);
	PRINT_FIELD_U(", ", *cursor2, height);
	PRINT_FIELD_U(", ", *cursor2, handle);
	PRINT_FIELD_D(", ", *cursor2, hot_x);
	PRINT_FIELD_D(", ", *cursor2, hot_y);
	printf("}");
}

static void
print_drm_mode_atomic(long rc, void *ptr, void *arg)
{
	struct drm_mode_atomic *atomic = ptr;

	if (rc < 0) {
		printf("%p", atomic);
		return;
	}
	printf("{flags=");
	printflags(drm_mode_atomic_flags, atomic->flags,
		   "DRM_MODE_ATOMIC_???");
	PRINT_FIELD_U(", ", *atomic, count_objs);
	printf(", objs_ptr=[]");
	printf(", count_props_ptr=[]");
	printf(", props_ptr=[]");
	printf(", prop_values_ptr=[]");
	if (atomic->reserved)
		PRINT_FIELD_U(", ", *atomic, reserved);
	PRINT_FIELD_X(", ", *atomic, user_data);
	printf("}");
}

static void
print_drm_mode_createpropblob(long rc, void *ptr, void *arg)
{
	struct drm_mode_create_blob *blob = ptr;

	if (rc < 0) {
		printf("%p", blob);
		return;
	}
	PRINT_FIELD_ADDR64("{", *blob, data);
	PRINT_FIELD_U(", ", *blob, length);
	PRINT_FIELD_U(", ", *blob, blob_id);
	printf("}");
}

static void
print_drm_destroypropblob(long rc, void *ptr, void *arg)
{
	struct drm_mode_destroy_blob *blob = ptr;

	if (rc < 0) {
		printf("%p", blob);
		return;
	}
	PRINT_FIELD_U("{", *blob, blob_id);
	printf("}");
}

# ifdef DRM_IOCTL_SYNCOBJ_CREATE
static void
print_drm_syncobj_create(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_create *create = ptr;

	if (rc < 0) {
		printf("%p", create);
		return;
	}
	printf("{flags=");
	printflags(drm_syncobj_flags, create->flags, "DRM_SYNCOJB_???");
	PRINT_FIELD_U(", ", *create, handle);
	printf("}");
}

# endif

# ifdef DRM_IOCTL_SYNCOBJ_DESTROY
static void
print_drm_syncobj_destroy(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_destroy *destroy = ptr;

	if (rc < 0) {
		printf("%p", destroy);
		return;
	}
	PRINT_FIELD_U("{", *destroy, handle);
	if (destroy->pad)
		PRINT_FIELD_U(", ", *destroy, pad);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD
static void
print_drm_syncobj_handle_to_fd(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_handle *handle = ptr;

	if (rc < 0) {
		printf("%p", handle);
		return;
	}
	PRINT_FIELD_U("{", *handle, handle);
	printf(", flags=");
	printflags(drm_syncobj_handle_to_fd_flags, handle->flags, "DRM_SYNCOBJ_???");
	if (handle->pad)
		PRINT_FIELD_U(", ", *handle, pad);
	PRINT_FIELD_D(", ", *handle, fd);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE
static void
print_drm_syncobj_fd_to_handle(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_handle *handle = ptr;

	if (rc < 0) {
		printf("%p", handle);
		return;
	}
	PRINT_FIELD_D("{", *handle, fd);
	printf(", flags=");
	printflags(drm_syncobj_fd_to_handle_flags, handle->flags, "DRM_SYNCOBJ_???");
	if (handle->pad)
		PRINT_FIELD_U(", ", *handle, pad);
	PRINT_FIELD_U(", ", *handle, handle);
	printf("}");

}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_WAIT
static void
print_drm_syncobj_wait(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_wait *wait = ptr;

	if (rc < 0) {
		printf("%p", wait);
		return;
	}
	printf("{handles=[]");
	PRINT_FIELD_D(", ", *wait, timeout_nsec);
	PRINT_FIELD_U(", ", *wait, count_handles);
	printf(", flags=");
	printflags(drm_syncobj_wait_flags, wait->flags,
		   "DRM_SYNCOBJ_WAIT_FLAGS_???");
	PRINT_FIELD_U(", ", *wait, first_signaled);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_RESET
static void
print_drm_syncobj_reset_or_signal(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_array *array = ptr;

	if (rc < 0) {
		printf("%p", array);
		return;
	}
	printf("{handles=[]");
	PRINT_FIELD_U(", ", *array, count_handles);
	if (array->pad)
		PRINT_FIELD_U(", ", *array, pad);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_MODE_CREATE_LEASE
static void
print_drm_mode_create_lease(long rc, void *ptr, void *arg)
{
	struct drm_mode_create_lease *lease= ptr;

	if (rc < 0) {
		printf("%p", lease);
		return;
	}
	printf("{object_ids=[]");
	PRINT_FIELD_U(", ", *lease, object_count);
	PRINT_FIELD_X(", ", *lease, flags);
	printf(", flags=");
	printflags(drm_mode_create_lease_flags, lease->flags, "O_???");
	PRINT_FIELD_U(", ", *lease, lessee_id);
	PRINT_FIELD_D(", ", *lease, fd);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_MODE_LIST_LESSEES
static void
print_drm_mode_list_lessees(long rc, void *ptr, void *arg)
{
	struct drm_mode_list_lessees *lessees = ptr;

	if (rc < 0) {
		printf("%p", lessees);
		return;
	}
	PRINT_FIELD_U("{", *lessees, count_lessees);
	if (lessees->pad)
		PRINT_FIELD_U(", ", *lessees, pad);
	printf(", lessees_ptr=[]");
	PRINT_FIELD_U("} => {", *lessees, count_lessees);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_MODE_GET_LEASE
static void
print_drm_mode_get_lease(long rc, void *ptr, void *arg)
{
	struct drm_mode_get_lease *lease = ptr;

	if (rc < 0) {
		printf("%p", lease);
		return;
	}
	PRINT_FIELD_U("{", *lease, count_objects);
	if (lease->pad)
		PRINT_FIELD_U(", ", *lease, pad);
	printf(", objects_ptr=[]");
	PRINT_FIELD_U("} => {", *lease, count_objects);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
static void
print_drm_mode_revoke_lease(long rc, void *ptr, void *arg)
{
	struct drm_mode_revoke_lease *lease = ptr;

	if (rc < 0) {
		printf("%p", lease);
		return;
	}
	PRINT_FIELD_U("{", *lease, lessee_id);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
static void
print_drm_syncobj_timeline_wait(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_timeline_wait *wait = ptr;

	if (rc < 0) {
		printf("%p", wait);
		return;
	}
	printf("{handles=[]");
	PRINT_FIELD_D(", ", *wait, timeout_nsec);
	PRINT_FIELD_U(", ", *wait, count_handles);
	printf(", flags=");
	printflags(drm_syncobj_wait_flags, wait->flags, "DRM_SYNCOBJ_WAIT_FLAGS_???");
	PRINT_FIELD_U(", ", *wait, first_signaled);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_QUERY
static void
print_drm_syncobj_query_or_timeline_signal(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_timeline_array *array = ptr;

	if (rc < 0) {
		printf("%p", array);
		return;
	}
	printf("{handles=[]");
	printf(", points=[]");
	PRINT_FIELD_U(", ", *array, count_handles);
	if (array->pad)
		PRINT_FIELD_U(", ", *array, pad);
	printf("}");
}
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
static void
print_drm_syncobj_transfer(long rc, void *ptr, void *arg)
{
	struct drm_syncobj_transfer *transfer = ptr;

	if (rc < 0) {
		printf("%p", transfer);
		return;
	}
	PRINT_FIELD_U("{", *transfer, src_handle);
	PRINT_FIELD_U(", ", *transfer, dst_handle);
	PRINT_FIELD_U(", ", *transfer, src_point);
	PRINT_FIELD_U(", ", *transfer, dst_point);
	printf(", flags=");
	printflags(drm_syncobj_wait_flags, transfer->flags, "DRM_SYNCOBJ_WAIT_FLAGS_???");
	if (transfer->pad)
		PRINT_FIELD_U(", ", *transfer, pad);
	printf("}");
}
# endif

int
main(int argc, char **argv)
{
	unsigned long num_skip;
	long inject_retval;
	bool locked = false;

	if (argc == 1)
		return 0;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	inject_retval = strtol(argv[2], NULL, 0);

	if (inject_retval < 0)
		error_msg_and_fail("Expected non-negative INJECT_RETVAL, "
				   "but got %ld", inject_retval);

	for (unsigned int i = 0; i < num_skip; i++) {
		long rc = ioctl(-1, DRM_IOCTL_VERSION, NULL);
		printf("ioctl(-1, DRM_IOCTL_VERSION, NULL) = %s%s\n",
		       sprintrc(rc),
		       rc == inject_retval ? " (INJECTED)" : "");

		if (rc != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", DRM_IOCTL_VERSION, NULL) returning %lu",
				   inject_retval);

	/* drm_version */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_version, ver);
	char bogus_name[4096] = "bogus_name";
	char bogus_date[4096] = "bogus_date";
	char bogus_desc[4096] = "bogus_desc";
	ver->version_major = (int) magic;
	ver->version_minor = (int) magic;
	ver->version_patchlevel = (int) magic;
	ver->name_len = strlen(bogus_name);
	ver->name = bogus_name;
	ver->date_len = strlen(bogus_date);
	ver->date = bogus_date;
	ver->desc_len = strlen(bogus_desc);
	ver->desc = bogus_desc;

	/* drm_unique */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_unique, unique);
	char bogus_unique[4096] = "bogus_unique";
	unique->unique_len = strlen(bogus_unique);
	unique->unique = bogus_unique;

	/* drm_auth */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_auth, auth);
	auth->magic = magic;

	/* drm_irq_busid */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_irq_busid, busid);
	busid->busnum = (int) magic;
	busid->devnum = (int) magic;
	busid->funcnum = (int) magic;
	busid->irq = (int) magic;

	/* drm_map */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_map, map);
	map->offset = lmagic;
	map->size = lmagic;
	map->type = _DRM_FRAME_BUFFER;
	map->flags = _DRM_RESTRICTED;
	map->handle = (void *) lmagic;
	map->mtrr = magic;

	/* drm_client */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_client, client);
	client->idx = magic;
	client->auth = magic;
	client->pid = magic;
	client->uid = magic;

	/* drm_stats */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_stats, stats);
	stats->count = magic;
	for (unsigned int i = 0; i < 15; i++) {
		stats->data[i].value = lmagic;
		stats->data[i].type = _DRM_STAT_LOCK;
	}

	/* drm_set_version */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_set_version, set_ver);
	set_ver->drm_di_major = (int) magic;
	set_ver->drm_di_minor = (int) magic;
	set_ver->drm_dd_major = (int) magic;
	set_ver->drm_dd_minor = (int) magic;

	/* drm_modeset_ctl */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_modeset_ctl, ctl);
	ctl->crtc = magic;
	ctl->cmd = _DRM_PRE_MODESET;

	/* drm_gem_close */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_gem_close, close);
	close->handle = magic;

	/* drm_gem_flink */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_gem_flink, flink);
	flink->handle = magic;
	flink->name = magic;

	/* drm_gen_open */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_gem_open, open);
	open->name = magic;
	open->handle = magic;
	open->size = lmagic;

	/* drm_get_cap */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_get_cap, cap);
	cap->capability = DRM_CAP_DUMB_BUFFER;
	cap->value = lmagic;

	/* drm_set_client_cap */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_set_client_cap, cli_cap);
	cli_cap->capability = DRM_CLIENT_CAP_STEREO_3D;
	cli_cap->value = lmagic;

	/* drm_block */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_block, block);
	block->unused = (int) magic;

	/* drm_control */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_control, control);
	control->func = DRM_ADD_COMMAND;
	control->irq = (int) magic;

	/* drm_buf_desc */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_buf_desc, desc);
	desc->count = (int) magic;
	desc->size = (int) magic;
	desc->low_mark = (int) magic;
	desc->high_mark = (int) magic;
	desc->flags = _DRM_PAGE_ALIGN;
	desc->agp_start = lmagic;

	/* drm_buf_info */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_buf_info, info);
	info->count = (int) magic;
	info->list = (struct drm_buf_desc *) lmagic;

	/* drm_buf_map */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_buf_map, buf_map);
	buf_map->count = (int) magic;
	buf_map->virtual = (void *) lmagic;
	buf_map->list = (struct drm_buf_pub *) lmagic;

	/* drm_buf_free */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_buf_free, buf_free);
	buf_free->count = (int) magic;
	buf_free->list = (int *) lmagic;

	/* drm_ctx_priv_map */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_ctx_priv_map, priv_map);
	priv_map->ctx_id = magic;
	priv_map->handle = (void *) lmagic;

	/* drm_ctx */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_ctx, ctx);
	ctx->handle = magic;
	ctx->flags = _DRM_CONTEXT_PRESERVED;

	/* drm_ctx_res */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_ctx_res, ctx_res);
	ctx_res->count = (int) magic;
	ctx_res->contexts = (struct drm_ctx *) lmagic;

	/* drm_draw */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_draw, draw);
	draw->handle = magic;

	/* drm_lock */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_lock, lock);
	lock->context = (int) magic;
	lock->flags = _DRM_LOCK_READY;

	/* drm_prime_handle */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_prime_handle, prime_handle);
	prime_handle->handle = magic;
	prime_handle->flags = DRM_CLOEXEC;
	prime_handle->fd = (int) magic;

	/* drm_agp_mode */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_agp_mode, agp_mode);
	agp_mode->mode = lmagic;

	/* drm_agp_info */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_agp_info, agp_info);
	agp_info->agp_version_major = (int) magic;
	agp_info->agp_version_minor = (int) magic;
	agp_info->mode = lmagic;
	agp_info->aperture_base = lmagic;
	agp_info->aperture_size = lmagic;
	agp_info->memory_allowed = lmagic;
	agp_info->memory_used = lmagic;
	agp_info->id_vendor = (unsigned short) magic;
	agp_info->id_device = (unsigned short) magic;

	/* drm_agp_buffer */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_agp_buffer, agp_buffer);
	agp_buffer->size = lmagic;
	agp_buffer->handle = lmagic;
	agp_buffer->type = lmagic;
	agp_buffer->physical = lmagic;

	/* drm_agp_binding */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_agp_binding, agp_binding);
	agp_binding->handle = lmagic;
	agp_binding->offset = lmagic;

	/* drm_scatter_gather */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_scatter_gather, sg);
	sg->size = lmagic;
	sg->handle = lmagic;

	/* drm_wait_vblank */
	TAIL_ALLOC_OBJECT_CONST_PTR(union drm_wait_vblank, vblank);
	vblank->request.type = _DRM_VBLANK_ABSOLUTE | _DRM_VBLANK_SIGNAL;
	vblank->request.sequence = magic;
	vblank->request.signal = lmagic;
	vblank->reply.tval_usec = (long) lmagic;

# ifdef DRM_IOCTL_CRTC_GET_SEQUENCE
	/* drm_crtc_get_sequence */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_crtc_get_sequence, get_seq);
	get_seq->crtc_id = magic;
	get_seq->active = magic;
	get_seq->sequence = lmagic;
	get_seq->sequence_ns = (long) lmagic;
# endif

# ifdef DRM_IOCTL_CRTC_QUEUE_SEQUENCE
	/* drm_crtc_queue_sequence */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_crtc_queue_sequence, queue_seq);
	queue_seq->crtc_id = magic;
	queue_seq->flags = DRM_CRTC_SEQUENCE_RELATIVE;
	queue_seq->sequence = lmagic;
	queue_seq->user_data = lmagic;
# endif

	/* drm_update_draw */

	/* drm_mode_card_res */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_card_res, card_res);
	card_res->fb_id_ptr = lmagic;
	card_res->crtc_id_ptr = lmagic;
	card_res->connector_id_ptr = lmagic;
	card_res->encoder_id_ptr = lmagic;
	card_res->count_fbs = 0;
	card_res->count_crtcs = 0;
	card_res->count_connectors = 0;
	card_res->count_encoders = 0;
	card_res->min_width = magic;
	card_res->max_width = magic;
	card_res->min_height = magic;
	card_res->max_height = magic;

	/* drm_mode_crtc */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_crtc, crtc);
	char bogus_mode_name[DRM_DISPLAY_MODE_LEN] = "bogus_mode_name";
	crtc->crtc_id = magic;
	crtc->set_connectors_ptr = lmagic;
	crtc->count_connectors = 0;
	crtc->fb_id = magic;
	crtc->x = magic;
	crtc->y = magic;
	crtc->gamma_size = magic;
	crtc->mode_valid = magic;
	snprintf(crtc->mode.name, sizeof(bogus_mode_name), "%s", bogus_mode_name);

	/* drm_mode_cursor */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_cursor, cursor);
	cursor->flags = DRM_MODE_CURSOR_BO | DRM_MODE_CURSOR_MOVE;
	cursor->crtc_id = magic;
	cursor->x = (int) magic;
	cursor->y = (int) magic;
	cursor->width = magic;
	cursor->height = magic;
	cursor->handle = magic;

	/* drm_mode_crtc_lut */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_crtc_lut, lut);
	lut->crtc_id = magic;
	lut->gamma_size = magic;
	lut->red = lmagic;
	lut->green = lmagic;
	lut->blue = lmagic;

	/* drm_mode_get_encoder */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_encoder, enc);
	enc->encoder_id = magic;
	enc->encoder_type = DRM_MODE_ENCODER_NONE;
	enc->crtc_id = magic;
	enc->possible_crtcs = magic;
	enc->possible_clones = magic;

	/* drm_mode_get_connector */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_connector, con);
	char con_str[4096];
	snprintf(con_str, sizeof(con_str), "DRM_IOWR(0xa7, %#lx) /* DRM_IOCTL_MODE_GETCONNECTOR */",
		 (long unsigned int) _IOC_SIZE(DRM_IOCTL_MODE_GETCONNECTOR));
	con->connector_id = magic;
	con->encoders_ptr = lmagic;
	con->modes_ptr = lmagic;
	con->props_ptr = lmagic;
	con->prop_values_ptr = lmagic;
	con->count_modes = 0;
	con->count_props = 0;
	con->count_encoders = 0;
	con->encoder_id = magic;
	con->connector_type = magic;
	con->connector_type_id = magic;
	con->connection = magic;
	con->mm_width = magic;
	con->mm_height = magic;
	con->subpixel = magic;

	/* drm_mode_mode_cmd */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_mode_cmd, mode_cmd);
	mode_cmd->connector_id = magic;
	snprintf(mode_cmd->mode.name, sizeof(bogus_mode_name), "%s", bogus_mode_name);

	/* drm_mode_get_property */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_property, prop);
	char bogus_property_name[DRM_PROP_NAME_LEN] = "bogus_property_name";
	prop->values_ptr = lmagic;
	prop->enum_blob_ptr = lmagic;
	prop->prop_id = magic;
	prop->flags = DRM_MODE_PROP_PENDING | DRM_MODE_PROP_RANGE;
	snprintf(prop->name, sizeof(bogus_property_name), "%s", bogus_property_name);
	prop->count_values = 0;
	prop->count_enum_blobs = 0;

	/* drm_mode_connector_set_property */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_connector_set_property, set_prop);
	set_prop->value = lmagic;
	set_prop->prop_id = magic;
	set_prop->connector_id = magic;

	/* drm_mode_get_blob */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_blob, blob);
	blob->blob_id = magic;
	blob->length = magic;
	blob->data = lmagic;

	/* drm_mode_fb_cmd */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_fb_cmd, cmd);
	cmd->width = magic;
	cmd->height = magic;
	cmd->pitch = magic;
	cmd->bpp = magic;
	cmd->depth = magic;
	cmd->handle = magic;
	cmd->fb_id = magic;

	/* drm_mode_crtc_page_flip */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_crtc_page_flip, flip);
	flip->crtc_id = magic;
	flip->fb_id = magic;
	flip->flags = DRM_MODE_PAGE_FLIP_EVENT;
	flip->user_data = lmagic;

	/* drm_mode_fb_dirty_cmd */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_fb_dirty_cmd, dirty_cmd);
	dirty_cmd->fb_id = magic;
	dirty_cmd->flags = DRM_MODE_FB_DIRTY_ANNOTATE_COPY |
			   DRM_MODE_FB_DIRTY_ANNOTATE_FILL;
	dirty_cmd->color = magic;
	dirty_cmd->num_clips = 0;
	dirty_cmd->clips_ptr = lmagic;

	/* struct drm_mode_create_dumb */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_create_dumb, dumb);
	dumb->width = magic;
	dumb->height = magic;
	dumb->bpp = magic;
	dumb->flags = magic;
	dumb->handle = magic;
	dumb->pitch = magic;
	dumb->size = lmagic;

	/* struct drm_mode_map_dumb */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_map_dumb, map_dumb);
	map_dumb->handle = magic;
	map_dumb->offset = magic;

	/* struct drm_mode_destroy_dumb */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_destroy_dumb, destroy_dumb);
	destroy_dumb->handle = magic;

	/* drm_mode_get_plane_res */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_plane_res, plane_res);
	plane_res->plane_id_ptr = lmagic;
	plane_res->count_planes = 0;

	/* drm_mode_get_plane */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_plane, get_plane);
	get_plane->plane_id = magic;
	get_plane->crtc_id = magic;
	get_plane->fb_id = magic;
	get_plane->possible_crtcs = magic;
	get_plane->gamma_size = magic;
	get_plane->count_format_types = 0;
	get_plane->format_type_ptr = lmagic;

	/* drm_mode_set_plane */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_set_plane, set_plane);
	set_plane->plane_id = magic;
	set_plane->crtc_id = magic;
	set_plane->fb_id = magic;
	set_plane->flags = DRM_MODE_PRESENT_TOP_FIELD |
			   DRM_MODE_PRESENT_BOTTOM_FIELD;
	set_plane->crtc_x = magic;
	set_plane->crtc_y = magic;
	set_plane->crtc_w = magic;
	set_plane->crtc_h = magic;
	set_plane->src_x = magic;
	set_plane->src_y = magic;
	set_plane->src_h = magic;
	set_plane->src_w = magic;

	/* wrap up DRM_IOCTL_MODE_RMFB */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_rm_fb_wrap, wrap);
	wrap->fb_id = 3;

	/* drm_mode_fb_cmd2 */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_fb_cmd2, cmd2);
	char cmd2_str[4096];
	snprintf(cmd2_str, sizeof(cmd2_str), "DRM_IOWR(0xb8, %#lx) /* DRM_IOCTL_MODE_ADDFB2 */",
		 (long unsigned int) _IOC_SIZE(DRM_IOCTL_MODE_ADDFB2));

	cmd2->width = 1;
	cmd2->height = 1;
	cmd2->pixel_format = 0x1;
	cmd2->flags = DRM_MODE_FB_INTERLACED | DRM_MODE_FB_MODIFIERS;
	for (unsigned int i = 0; i < 4; i++) {
		cmd2->handles[i] = 1;
		cmd2->pitches[i] = 1;
		cmd2->offsets[i] = 1;
# ifdef HAVE_STRUCT_DRM_MODE_FB_CMD2_MODIFIER
		cmd2->modifier[i] = 1;
# endif
	}
	cmd2->fb_id = 1;

	/* drm_mode_obj_get_properties */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_obj_get_properties, obj_get_prop);
	obj_get_prop->props_ptr = lmagic;
	obj_get_prop->prop_values_ptr = lmagic;
	obj_get_prop->count_props = 0;
	obj_get_prop->obj_id = magic;
	obj_get_prop->obj_type = magic;

	/* drm_mode_obj_set_property */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_obj_set_property, obj_set_prop);
	obj_set_prop->value = lmagic;
	obj_set_prop->prop_id = magic;
	obj_set_prop->obj_id = magic;
	obj_set_prop->obj_type = magic;

	/* drm_mode_cursor2 */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_cursor2, cursor2);
	cursor2->flags = DRM_MODE_CURSOR_BO | DRM_MODE_CURSOR_MOVE;
	cursor2->crtc_id = magic;
	cursor2->x = (int) magic;
	cursor2->y = (int) magic;
	cursor2->width = magic;
	cursor2->height = magic;
	cursor2->handle = magic;
	cursor2->hot_x = (int) magic;
	cursor2->hot_y = (int) magic;

	/* drm_mode_atomic */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_atomic, atomic);
	atomic->flags = DRM_MODE_ATOMIC_TEST_ONLY | DRM_MODE_ATOMIC_NONBLOCK;
	atomic->count_objs = 0;
	atomic->objs_ptr = lmagic;
	atomic->count_props_ptr = lmagic;
	atomic->props_ptr = lmagic;
	atomic->prop_values_ptr = lmagic;
	atomic->reserved = lmagic;
	atomic->user_data = lmagic;

	/* drm_mode_create_blob */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_create_blob, create_blob);
	create_blob->data = lmagic;
	create_blob->length = magic;
	create_blob->blob_id = magic;

	/* drm_mode_destroy_blob */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_destroy_blob, destroy_blob);
	destroy_blob->blob_id = magic;

# ifdef DRM_IOCTL_SYNCOBJ_CREATE
	/* drm_syncobj_create */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_create, sync_create);
	sync_create->handle = magic;
	sync_create->flags = DRM_SYNCOBJ_CREATE_SIGNALED;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_DESTROY
	/* drm_syncobj_destroy */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_destroy, sync_destroy);
	sync_destroy->handle = magic;
	sync_destroy->pad = magic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD
	/* drm_syncobj_handle */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_handle, sync_handle);
	sync_handle->handle = magic;
	sync_handle->flags = DRM_SYNCOBJ_FD_TO_HANDLE_FLAGS_IMPORT_SYNC_FILE |
			     DRM_SYNCOBJ_HANDLE_TO_FD_FLAGS_EXPORT_SYNC_FILE;
	sync_handle->fd = 123;
	sync_handle->pad= magic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_WAIT
	/* drm_syncobj_wait */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_wait, sync_wait);
	sync_wait->handles = lmagic;
	sync_wait->timeout_nsec = (long) lmagic;
	sync_wait->count_handles = 0;
	sync_wait->flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL |
			   DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT;
	sync_wait->first_signaled = magic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_RESET
	/* drm_syncobj_array */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_array, sync_array);
	sync_array->handles = lmagic;
	sync_array->count_handles = 0;
	sync_array->pad = magic;
# endif

# ifdef DRM_IOCTL_MODE_CREATE_LEASE
	/* drm_mode_create_lease */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_create_lease, create_lease);
	create_lease->object_ids = lmagic;
	create_lease->object_count = 0;
	create_lease->flags = O_CLOEXEC | O_NONBLOCK;
	create_lease->lessee_id = magic;
	create_lease->fd = (int) magic;
# endif

# ifdef DRM_IOCTL_MODE_LIST_LESSEES
	/* drm_mode_list_lessees */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_list_lessees, list_lessees);
	list_lessees->count_lessees = 0;
	list_lessees->pad = magic;
	list_lessees->lessees_ptr = lmagic;
# endif

# ifdef DRM_IOCTL_MODE_GET_LEASE
	/* drm_mode_get_lease */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_lease, get_lease);
	get_lease->count_objects = 0;
	get_lease->pad = magic;
	get_lease->objects_ptr = magic;
# endif

# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
	/* drm_mode_revoke_lease */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_revoke_lease, revoke_lease);
	revoke_lease->lessee_id = magic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
	/* drm_syncobj_timeline_wait */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_timeline_wait, timeline_wait);
	timeline_wait->handles = lmagic;
	timeline_wait->points = lmagic;
	timeline_wait->timeout_nsec = (long) lmagic;
	timeline_wait->count_handles = 0;
	timeline_wait->flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
	timeline_wait->first_signaled = magic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_QUERY
	/* drm_syncobj_timeline_array */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_timeline_array, timeline_array);
	timeline_array->handles = lmagic;
	timeline_array->points = lmagic;
	timeline_array->count_handles = 0;
	timeline_array->pad = magic;
# endif

# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
	/* drm_syncobj_transfer */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_syncobj_transfer, sync_transfer);
	sync_transfer->src_handle = magic;
	sync_transfer->dst_handle = magic;
	sync_transfer->src_point = lmagic;
	sync_transfer->dst_point = lmagic;
	sync_transfer->flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
	sync_transfer->pad = magic;
# endif

	struct drm_check a[] = {
		{ ARG_STR(DRM_IOCTL_VERSION), ver, print_drm_version },
		{ ARG_STR(DRM_IOCTL_GET_UNIQUE), unique, print_drm_get_unique },
		{ ARG_STR(DRM_IOCTL_GET_MAGIC), auth, print_drm_get_magic },
		{ ARG_STR(DRM_IOCTL_IRQ_BUSID), busid, print_drm_irq_busid },
		{ ARG_STR(DRM_IOCTL_GET_MAP), map, print_drm_get_map },
		{ ARG_STR(DRM_IOCTL_GET_CLIENT), client, print_drm_get_client },
		{ ARG_STR(DRM_IOCTL_SET_VERSION), set_ver, print_drm_set_version },
		{ ARG_STR(DRM_IOCTL_MODESET_CTL), ctl, print_drm_modeset_ctl },
		{ ARG_STR(DRM_IOCTL_GEM_CLOSE), close, print_drm_gem_close },
		{ ARG_STR(DRM_IOCTL_GEM_FLINK), flink, print_drm_gem_flink },
		{ ARG_STR(DRM_IOCTL_GEM_OPEN), open, print_drm_gem_open },
		{ ARG_STR(DRM_IOCTL_GET_CAP), cap, print_drm_get_cap },
		{ ARG_STR(DRM_IOCTL_SET_CLIENT_CAP), cli_cap, print_drm_set_client_cap },
		{ ARG_STR(DRM_IOCTL_AUTH_MAGIC), auth, print_drm_auth_magic },
		{ ARG_STR(DRM_IOCTL_BLOCK), block, print_drm_block },
		{ ARG_STR(DRM_IOCTL_UNBLOCK), block, print_drm_unblock },
		{ ARG_STR(DRM_IOCTL_CONTROL), control, print_drm_control },
		{ ARG_STR(DRM_IOCTL_ADD_MAP), map, print_drm_add_map },
		{ ARG_STR(DRM_IOCTL_ADD_BUFS), desc, print_drm_add_bufs },
		{ ARG_STR(DRM_IOCTL_MARK_BUFS), desc, print_drm_mark_bufs },
		{ ARG_STR(DRM_IOCTL_INFO_BUFS), info, print_drm_info_bufs },
		{ ARG_STR(DRM_IOCTL_MAP_BUFS), buf_map, print_drm_map_bufs },
		{ ARG_STR(DRM_IOCTL_FREE_BUFS), buf_free, print_drm_free_bufs },
		{ ARG_STR(DRM_IOCTL_RM_MAP), map, print_drm_rm_map },
		{ ARG_STR(DRM_IOCTL_SET_SAREA_CTX), priv_map, print_drm_sarea_ctx },
		{ ARG_STR(DRM_IOCTL_GET_SAREA_CTX), priv_map, print_drm_sarea_ctx },
		{ ARG_STR(DRM_IOCTL_ADD_CTX), ctx, print_drm_ctx },
		{ ARG_STR(DRM_IOCTL_RM_CTX), ctx, print_drm_ctx },
		{ ARG_STR(DRM_IOCTL_GET_CTX), ctx, print_drm_get_ctx },
		{ ARG_STR(DRM_IOCTL_SWITCH_CTX), ctx, print_drm_ctx },
		{ ARG_STR(DRM_IOCTL_NEW_CTX), ctx, print_drm_ctx },
		{ ARG_STR(DRM_IOCTL_RES_CTX), ctx_res, print_drm_res_ctx },
		{ ARG_STR(DRM_IOCTL_LOCK), lock, print_drm_lock },
		{ ARG_STR(DRM_IOCTL_UNLOCK), lock, print_drm_lock },
		{ ARG_STR(DRM_IOCTL_PRIME_HANDLE_TO_FD), prime_handle, print_drm_prime_handle_to_fd },
		{ ARG_STR(DRM_IOCTL_PRIME_FD_TO_HANDLE), prime_handle, print_drm_prime_fd_to_handle },
		{ ARG_STR(DRM_IOCTL_AGP_ENABLE), agp_mode, print_drm_agp_enable },
		{ ARG_STR(DRM_IOCTL_AGP_INFO), agp_info, print_drm_agp_info },
		{ ARG_STR(DRM_IOCTL_AGP_ALLOC), agp_buffer, print_drm_agp_alloc },
		{ ARG_STR(DRM_IOCTL_AGP_FREE), agp_buffer, print_drm_agp_free },
		{ ARG_STR(DRM_IOCTL_AGP_BIND), agp_binding, print_drm_agp_bind },
		{ ARG_STR(DRM_IOCTL_AGP_UNBIND), agp_binding, print_drm_agp_unbind },
		{ ARG_STR(DRM_IOCTL_SG_ALLOC), sg, print_drm_sg_alloc },
		{ ARG_STR(DRM_IOCTL_SG_FREE), sg, print_drm_sg_free },
		{ ARG_STR(DRM_IOCTL_WAIT_VBLANK), vblank, print_drm_wait_vblank },
# ifdef DRM_IOCTL_CRTC_GET_SEQUENCE
		{ ARG_STR(DRM_IOCTL_CRTC_GET_SEQUENCE), get_seq, print_drm_crtc_get_sequence },
# endif
# ifdef DRM_IOCTL_CRTC_QUEUE_SEQUENCE
		{ ARG_STR(DRM_IOCTL_CRTC_QUEUE_SEQUENCE), queue_seq, print_drm_crtc_queue_sequence },
# endif
		{ ARG_STR(DRM_IOCTL_MODE_GETRESOURCES), card_res, print_drm_mode_get_resources },
		{ ARG_STR(DRM_IOCTL_MODE_CURSOR), cursor, print_drm_mode_cursor },
		{ ARG_STR(DRM_IOCTL_MODE_GETGAMMA), lut, print_drm_mode_get_gamma },
		{ ARG_STR(DRM_IOCTL_MODE_SETGAMMA), lut, print_drm_mode_set_gamma },
		{ ARG_STR(DRM_IOCTL_MODE_GETENCODER), enc, print_drm_mode_get_encoder },
		{ ARG_STR(DRM_IOCTL_MODE_GETPROPERTY), prop, print_drm_mode_get_property },
		{ ARG_STR(DRM_IOCTL_MODE_SETPROPERTY), set_prop, print_drm_mode_set_property },
		{ ARG_STR(DRM_IOCTL_MODE_GETPROPBLOB), blob, print_drm_mode_get_prop_blob },
		{ ARG_STR(DRM_IOCTL_MODE_GETFB), cmd, print_drm_mode_get_fb },
		{ ARG_STR(DRM_IOCTL_MODE_ADDFB), cmd, print_drm_mode_add_fb },
		{ ARG_STR(DRM_IOCTL_MODE_RMFB), wrap, print_drm_mode_rm_fb },
		{ ARG_STR(DRM_IOCTL_MODE_PAGE_FLIP), flip, print_drm_mode_page_flip },
		{ ARG_STR(DRM_IOCTL_MODE_DIRTYFB), dirty_cmd, print_drm_mode_dirty_fb },
		{ ARG_STR(DRM_IOCTL_MODE_CREATE_DUMB), dumb, print_drm_mode_create_dumb },
		{ ARG_STR(DRM_IOCTL_MODE_MAP_DUMB), map_dumb, print_drm_mode_map_dumb },
		{ ARG_STR(DRM_IOCTL_MODE_DESTROY_DUMB), destroy_dumb, print_drm_mode_destroy_dumb },
		{ ARG_STR(DRM_IOCTL_MODE_GETPLANERESOURCES), plane_res, print_drm_mode_getplaneresources },
		{ ARG_STR(DRM_IOCTL_MODE_GETPLANE), get_plane, print_drm_mode_getplane },
		{ ARG_STR(DRM_IOCTL_MODE_SETPLANE), set_plane, print_drm_mode_setplane },
		{ ARG_STR(DRM_IOCTL_MODE_OBJ_GETPROPERTIES), obj_get_prop, print_drm_mode_obj_getproperties},
		{ ARG_STR(DRM_IOCTL_MODE_OBJ_SETPROPERTY), obj_set_prop, print_drm_mode_obj_setproperty },
		{ ARG_STR(DRM_IOCTL_MODE_CURSOR2), cursor2, print_drm_mode_cursor2 },
		{ ARG_STR(DRM_IOCTL_MODE_ATOMIC), atomic, print_drm_mode_atomic },
		{ ARG_STR(DRM_IOCTL_MODE_CREATEPROPBLOB), create_blob, print_drm_mode_createpropblob},
		{ ARG_STR(DRM_IOCTL_MODE_DESTROYPROPBLOB), destroy_blob, print_drm_destroypropblob },
# ifdef DRM_IOCTL_SYNCOBJ_CREATE
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_CREATE), sync_create, print_drm_syncobj_create },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_DESTROY
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_DESTROY), sync_destroy, print_drm_syncobj_destroy },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD), sync_handle, print_drm_syncobj_handle_to_fd },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE), sync_handle, print_drm_syncobj_fd_to_handle },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_WAIT
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_WAIT), sync_wait, print_drm_syncobj_wait },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_RESET
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_RESET), sync_array, print_drm_syncobj_reset_or_signal },
# endif
# ifdef DRM_IOCTL_MODE_CREATE_LEASE
		{ ARG_STR(DRM_IOCTL_MODE_CREATE_LEASE), create_lease, print_drm_mode_create_lease },
# endif
# ifdef DRM_IOCTL_MODE_LIST_LESSEES
		{ ARG_STR(DRM_IOCTL_MODE_LIST_LESSEES), list_lessees, print_drm_mode_list_lessees },
# endif
# ifdef DRM_IOCTL_MODE_GET_LEASE
		{ ARG_STR(DRM_IOCTL_MODE_GET_LEASE), get_lease, print_drm_mode_get_lease },
# endif
# ifdef DRM_IOCTL_MODE_REVOKE_LEASE
		{ ARG_STR(DRM_IOCTL_MODE_REVOKE_LEASE), revoke_lease, print_drm_mode_revoke_lease },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT), timeline_wait, print_drm_syncobj_timeline_wait },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_QUERY
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_QUERY), timeline_array, print_drm_syncobj_query_or_timeline_signal },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_TRANSFER
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_TRANSFER), sync_transfer, print_drm_syncobj_transfer },
# endif
# ifdef DRM_IOCTL_SYNCOBJ_QUERY
		{ ARG_STR(DRM_IOCTL_SYNCOBJ_TIMELINE_SIGNAL), timeline_array, print_drm_syncobj_query_or_timeline_signal },
# endif
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(a); i++) {
		test_drm(&a[i], NULL);
	}

	struct drm_check check_crtc[] = {
		{ ARG_STR(DRM_IOCTL_MODE_GETCRTC), crtc, print_drm_mode_crtc },
		{ ARG_STR(DRM_IOCTL_MODE_SETCRTC), crtc, print_drm_mode_crtc }
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(check_crtc); i++) {
		int is_get = check_crtc[i].cmd == DRM_IOCTL_MODE_GETCRTC;
		test_drm(&check_crtc[i], (void *)&is_get);
	}

	struct drm_check b = {
		.cmd = DRM_IOCTL_MODE_GETCONNECTOR,
		.cmd_str = con_str,
		.arg_ptr = con,
		.print_arg = print_drm_mode_get_connector,
	};

	test_drm(&b, NULL);

	struct drm_check check_cmd2 = {
		.cmd = DRM_IOCTL_MODE_ADDFB2,
		.cmd_str = cmd2_str,
		.arg_ptr = cmd2,
		.print_arg = print_drm_mode_add_fb2,
	};

	test_drm(&check_cmd2, NULL);

	puts("+++ exited with 0 +++");
	return 0;
}
#else

SKIP_MAIN_UNDEFINED("HAVE_DRM_H && HAVE_DRM_DRM_H");

#endif
