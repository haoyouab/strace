#include "tests.h"

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

# include <errno.h>
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
	PRINT_FIELD_U("{", *ver, version_major);
	PRINT_FIELD_U(", ", *ver, version_minor);
	PRINT_FIELD_U(", ", *ver, version_patchlevel);
	PRINT_FIELD_U(", ", *ver, name_len);
	printf(", name=\"%s\"", ver->name);
	PRINT_FIELD_U(", ", *ver, date_len);
	printf(", date=\"%s\"", ver->date);
	PRINT_FIELD_U(", ", *ver, desc_len);
	printf(", desc=\"%s\"", ver->desc);
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
	printf("}");
	PRINT_FIELD_D(" => {", *ver, drm_di_major);
	PRINT_FIELD_D(", ", *ver, drm_di_minor);
	PRINT_FIELD_D(", ", *ver, drm_dd_major);
	PRINT_FIELD_D(", ", *ver, drm_dd_minor);
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
print_drm_wait_vblank(long rc, void *ptr, void *arg)
{
	union drm_wait_vblank *vblank = ptr;

	if (rc < 0) {
		printf("%p", vblank);
		return;
	}
	PRINT_FIELD_U("{request={", vblank->request, type);
	PRINT_FIELD_U(", ", vblank->request, sequence);
	PRINT_FIELD_U(", ", vblank->request, signal);
	printf("}");


	PRINT_FIELD_U(", reply={", vblank->reply, type);
	PRINT_FIELD_U(", ", vblank->reply, sequence);
	PRINT_FIELD_U(", ", vblank->reply, tval_sec);
	PRINT_FIELD_U(", ", vblank->reply, tval_usec);
	printf("}}");
}

static void
print_drm_mode_get_resources(long rc, void *ptr, void *arg)
{
	struct drm_mode_card_res *res = ptr;

	if (rc < 0) {
		printf("%p", res);
		return;
	}
	printf("{fb_id_ptr=%#llx, ", (unsigned long long) res->fb_id_ptr);
	printf("crtc_id_ptr=%#llx, ", (unsigned long long) res->crtc_id_ptr);
	printf("connector_id_ptr=%#llx, ", (unsigned long long) res->connector_id_ptr);
	printf("encoder_id_ptr=%#llx", (unsigned long long) res->encoder_id_ptr);
	PRINT_FIELD_U(", ", *res, count_fbs);
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
	PRINT_FIELD_X(", ", *info, flags);
	PRINT_FIELD_U(", ", *info, type);
	printf(", name=\"%s\"", info->name);
}

static void
print_drm_mode_get_crtc(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc *crtc = ptr;

	if (rc < 0) {
		printf("%p", crtc);
		return;
	}
	PRINT_FIELD_U("{", *crtc, crtc_id);
	printf(", set_connectors_ptr=%#llx", (unsigned long long) crtc->set_connectors_ptr);
	PRINT_FIELD_U(", ", *crtc, count_connectors);
	PRINT_FIELD_U(", ", *crtc, fb_id);
	PRINT_FIELD_U(", ", *crtc, x);
	PRINT_FIELD_U(", ", *crtc, y);
	PRINT_FIELD_U(", ", *crtc, gamma_size);
	PRINT_FIELD_U(", ", *crtc, mode_valid);
	printf("mode={");
	drm_mode_print_modeinfo(&crtc->mode);
	printf("}}");
}

static void
print_drm_mode_set_crtc(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc *crtc = ptr;

	if (rc < 0) {
		printf("%p", crtc);
		return;
	}
	printf("{set_connectors_ptr=%#llx", (unsigned long long) crtc->set_connectors_ptr);
	PRINT_FIELD_U(", ", *crtc, count_connectors);
	PRINT_FIELD_U(", ", *crtc, crtc_id);
	PRINT_FIELD_U(", ", *crtc, fb_id);
	PRINT_FIELD_U(", ", *crtc, x);
	PRINT_FIELD_U(", ", *crtc, y);
	PRINT_FIELD_U(", ", *crtc, gamma_size);
	PRINT_FIELD_U(", ", *crtc, mode_valid);
	printf("mode={");
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
	PRINT_FIELD_X("{", *cursor, flags);
	PRINT_FIELD_U(", ", *cursor, crtc_id);
	PRINT_FIELD_D(", ", *cursor, x);
	PRINT_FIELD_D(", ", *cursor, y);
	PRINT_FIELD_U(", ", *cursor, width);
	PRINT_FIELD_U(", ", *cursor, height);
	PRINT_FIELD_U(", ", *cursor, handle);
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
	PRINT_FIELD_X("{", *cursor2, flags);
	PRINT_FIELD_U(", ", *cursor2, crtc_id);
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
print_drm_mode_get_gamma(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc_lut *lut = ptr;

	if (rc < 0) {
		printf("%p", lut);
		return;
	}
	PRINT_FIELD_U("{", *lut, crtc_id);
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
	PRINT_FIELD_U("{", *lut, crtc_id);
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
	PRINT_FIELD_U(", ", *enc, encoder_type);
	PRINT_FIELD_U(", ", *enc, crtc_id);
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
	printf(", encoders_ptr=%#llx", (unsigned long long) con->encoders_ptr);
	printf(", modes_ptr=%#llx", (unsigned long long) con->modes_ptr);
	printf(", props_ptr=%#llx", (unsigned long long) con->props_ptr);
	printf(", prop_values_ptr=%#llx", (unsigned long long) con->prop_values_ptr);
	PRINT_FIELD_U(", ", *con, count_modes);
	PRINT_FIELD_U(", ", *con, count_props);
	PRINT_FIELD_U(", ", *con, count_encoders);
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
	printf(", values_ptr=%#llx", prop->values_ptr);
	printf(", enum_blob_ptr=%#llx", prop->enum_blob_ptr);
	PRINT_FIELD_X(", ", *prop, flags);
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
	PRINT_FIELD_U(", ", *blob, data);
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
	PRINT_FIELD_U(", ", *cmd, flags);
	printf(", handles={%u, %u, %u, %u}, "
	       "pitches={%u, %u, %u, %u}, "
	       "offsets={%u, %u, %u, %u}",
	       cmd->handles[0], cmd->handles[1], cmd->handles[2],
	       cmd->handles[3], cmd->pitches[0], cmd->pitches[1],
	       cmd->pitches[2], cmd->pitches[3], cmd->offsets[0],
	       cmd->offsets[1], cmd->offsets[2], cmd->offsets[3]);
#ifdef HAVE_STRUCT_DRM_MODE_FB_CMD2_MODIFIER
	printf(", modifiers={%" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 "}",
	       (uint64_t) cmd->modifier[0], (uint64_t) cmd->modifier[1],
	       (uint64_t) cmd->modifier[2], (uint64_t) cmd->modifier[3]);
#endif
	PRINT_FIELD_U(", ", *cmd, fb_id);
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
print_drm_mode_rm_fb(long rc, void *ptr, void *arg)
{
	//unsigned int *handle = ptr;
	struct drm_mode_rm_fb_wrap *wrap = ptr;

	if (rc < 0) {
		printf("%p", wrap);
		return;
	}
	printf("%u", wrap->fb_id);
}

static void
print_drm_mode_page_flip(long rc, void *ptr, void *arg)
{
	struct drm_mode_crtc_page_flip *flip = ptr;

	if (rc < 0) {
		printf("%p", flip);
		return;
	}
	PRINT_FIELD_U("{", *flip, crtc_id);
	PRINT_FIELD_U(", ", *flip, fb_id);
	PRINT_FIELD_X(", ", *flip, flags);
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
	PRINT_FIELD_X(", ", *cmd, flags);
	PRINT_FIELD_X(", ", *cmd, color);
	PRINT_FIELD_U(", ", *cmd, num_clips);
	printf(", clips_ptr=%#llx", cmd->clips_ptr);
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
	ver->version_major = 1;
	ver->version_minor = 2;
	ver->version_patchlevel = 3;
	ver->name_len = strlen(bogus_name);
	ver->name = bogus_name;
	ver->date_len = strlen(bogus_date);
	ver->date = bogus_date;
	ver->desc_len = strlen(bogus_desc);
	ver->desc = bogus_desc;

	/* drm_set_version */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_set_version, set_ver);
	set_ver->drm_di_major = 1;
	set_ver->drm_di_minor = 2;
	set_ver->drm_dd_major = 3;
	set_ver->drm_dd_minor = 4;

	/* drm_unique */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_unique, unique);
	char bogus_unique[4096] = "bogus_unique";
	unique->unique_len = strlen(bogus_unique);
	unique->unique = bogus_unique;

	/* drm_auth */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_auth, auth);
	auth->magic = 123;

	/* drm_wait_vblank */
	TAIL_ALLOC_OBJECT_CONST_PTR(union drm_wait_vblank, vblank);
	vblank->request.type = 1;
	vblank->request.sequence = 2;
	vblank->request.signal = 3;
	vblank->reply.tval_usec = 4;

	/* drm_mode_card_res */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_card_res, res);
	res->fb_id_ptr = 0xffffffff;
	res->crtc_id_ptr = 0xffffffff;
	res->connector_id_ptr = 0xffffffff;
	res->encoder_id_ptr = 0xffffffff;
	res->count_fbs = 1;
	res->count_crtcs = 1;
	res->count_connectors = 1;
	res->count_encoders = 1;
	res->min_width = 1;
	res->max_width = 1;
	res->min_height = 1;
	res->max_height = 1;

	/* drm_mode_crtc */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_crtc, crtc);
	char bogus_mode_name[DRM_DISPLAY_MODE_LEN] = "bogus_mode_name";
	crtc->crtc_id = 123;
	crtc->set_connectors_ptr = 0xffffffff;
	crtc->count_connectors = 1;
	crtc->fb_id = 1;
	crtc->x = 1;
	crtc->y = 1;
	crtc->gamma_size = 1;
	crtc->mode_valid = 1;
	snprintf(crtc->mode.name, sizeof(bogus_mode_name), "%s", bogus_mode_name);

	/* drm_mode_cursor */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_cursor, cursor);
	cursor->flags = 1;
	cursor->crtc_id = 1;
	cursor->x = 1;
	cursor->y = 1;
	cursor->width = 1;
	cursor->height = 1;
	cursor->handle = 1;

	/* drm_mode_cursor2 */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_cursor2, cursor2);
	cursor2->flags = 1;
	cursor2->crtc_id = 1;
	cursor2->x = 1;
	cursor2->y = 1;
	cursor2->width = 1;
	cursor2->height = 1;
	cursor2->handle = 1;
	cursor2->hot_x = 1;
	cursor2->hot_y = 1;

	/* drm_mode_crtc_lut */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_crtc_lut, lut);
	lut->crtc_id = 1;
	lut->gamma_size = 1;
	lut->red = 255;
	lut->green = 255;
	lut->blue = 255;

	/* drm_mode_get_encoder */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_encoder, enc);
	enc->encoder_id = 1;
	enc->encoder_type = 1;
	enc->crtc_id = 1;
	enc->possible_crtcs = 1;
	enc->possible_clones = 1;

	/* drm_mode_get_connector */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_connector, con);
	char con_str[4096];
	snprintf(con_str, sizeof(con_str), "DRM_IOWR(0xa7, %#lx) /* DRM_IOCTL_MODE_GETCONNECTOR */",
		 (long unsigned int) _IOC_SIZE(DRM_IOCTL_MODE_GETCONNECTOR));
	con->connector_id = 1;
	con->encoders_ptr = 0xffffffff;
	con->modes_ptr = 0xffffffff;
	con->props_ptr = 0xffffffff;
	con->prop_values_ptr = 0xffffffff;
	con->count_modes = 1;
	con->count_props = 1;
	con->count_encoders = 1;
	con->encoder_id = 1;
	con->connector_type = 1;
	con->connector_type_id = 1;
	con->connection = 1;
	con->mm_width = 1;
	con->mm_height = 1;
	con->subpixel = 1;

	/* drm_mode_get_property */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_property, prop);
	char bogus_property_name[DRM_PROP_NAME_LEN] = "bogus_property_name";
	prop->prop_id = 1;
	prop->values_ptr = 0xffffffff;
	prop->enum_blob_ptr = 0xffffffff;
	prop->flags = 0x1;
	snprintf(prop->name, sizeof(bogus_property_name), "%s", bogus_property_name);

	/* drm_mode_connector_set_property */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_connector_set_property, set_prop);
	set_prop->value = 1;
	set_prop->prop_id = 1;
	set_prop->connector_id = 1;

	/* drm_mode_get_blob */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_get_blob, blob);
	blob->blob_id = 1;
	blob->length = 1;
	blob->data = 1;

	/* drm_mode_fb_cmd */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_fb_cmd, cmd);
	cmd->width = 1;
	cmd->height = 1;
	cmd->pitch = 1;
	cmd->bpp = 1;
	cmd->depth = 1;
	cmd->handle = 1;
	cmd->fb_id = 1;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_fb_cmd2, cmd2);
	cmd2->width = 1;
	cmd2->height = 1;
	cmd2->pixel_format = 0x1;
	cmd2->flags = 1;
	for (unsigned int i = 0; i < 4; i++) {
		cmd2->handles[i] = 1;
		cmd2->pitches[i] = 1;
		cmd2->offsets[i] = 1;
#ifdef HAVE_STRUCT_DRM_MODE_FB_CMD2_MODIFIER
		cmd2->modifier[i] = 1;
#endif
	}
	cmd2->fb_id = 1;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_rm_fb_wrap, wrap);
	wrap->fb_id = 3;

	/* struct drm_mode_crtc_page_flip */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_crtc_page_flip, flip);
	flip->crtc_id = 1;
	flip->fb_id = 1;
	flip->flags = 0x1;
	flip->user_data = 1;

	/* struct drm_mode_fb_dirty_cmd */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_fb_dirty_cmd, dirty_cmd);
	dirty_cmd->fb_id = 1;
	dirty_cmd->flags = 0x1;
	dirty_cmd->color = 0x1;
	dirty_cmd->num_clips = 1;
	dirty_cmd->clips_ptr = 0xffffffff;

	/* struct drm_mode_create_dumb */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_create_dumb, dumb);
	dumb->width = 1;
	dumb->height = 1;
	dumb->bpp = 1;
	dumb->flags = 0x1;
	dumb->handle = 1;
	dumb->pitch = 1;
	dumb->size = 1;

	/* struct drm_mode_map_dumb */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_map_dumb, map_dumb);
	map_dumb->handle = 1;
	map_dumb->offset = 1;

	/* struct drm_mode_destroy_dumb */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_mode_destroy_dumb, destroy_dumb);
	destroy_dumb->handle = 1;

	/* struct drm_gem_close */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_gem_close, close);
	close->handle = 1;

	struct drm_check a[] = {
		{ ARG_STR(DRM_IOCTL_VERSION), ver, print_drm_version },
		{ ARG_STR(DRM_IOCTL_SET_VERSION), set_ver, print_drm_set_version },
		{ ARG_STR(DRM_IOCTL_GET_UNIQUE), unique, print_drm_get_unique },
		{ ARG_STR(DRM_IOCTL_GET_MAGIC), auth, print_drm_get_magic },
		{ ARG_STR(DRM_IOCTL_WAIT_VBLANK), vblank, print_drm_wait_vblank },
		{ ARG_STR(DRM_IOCTL_MODE_GETRESOURCES), res, print_drm_mode_get_resources },
		{ ARG_STR(DRM_IOCTL_MODE_GETCRTC), crtc, print_drm_mode_get_crtc },
		{ ARG_STR(DRM_IOCTL_MODE_SETCRTC), crtc, print_drm_mode_set_crtc },
		{ ARG_STR(DRM_IOCTL_MODE_CURSOR), cursor, print_drm_mode_cursor },
		{ ARG_STR(DRM_IOCTL_MODE_CURSOR2), cursor2, print_drm_mode_cursor2 },
		{ ARG_STR(DRM_IOCTL_MODE_GETGAMMA), lut, print_drm_mode_get_gamma },
		{ ARG_STR(DRM_IOCTL_MODE_SETGAMMA), lut, print_drm_mode_set_gamma },
		{ ARG_STR(DRM_IOCTL_MODE_GETENCODER), enc, print_drm_mode_get_encoder },
		{ ARG_STR(DRM_IOCTL_MODE_GETPROPERTY), prop, print_drm_mode_get_property },
		{ ARG_STR(DRM_IOCTL_MODE_SETPROPERTY), set_prop, print_drm_mode_set_property },
		{ ARG_STR(DRM_IOCTL_MODE_GETPROPBLOB), blob, print_drm_mode_get_prop_blob },
		{ ARG_STR(DRM_IOCTL_MODE_ADDFB), cmd, print_drm_mode_add_fb },
		{ ARG_STR(DRM_IOCTL_MODE_ADDFB2), cmd2, print_drm_mode_add_fb2 },
		{ ARG_STR(DRM_IOCTL_MODE_GETFB), cmd, print_drm_mode_get_fb },
		{ ARG_STR(DRM_IOCTL_MODE_RMFB), wrap, print_drm_mode_rm_fb },
		{ ARG_STR(DRM_IOCTL_MODE_PAGE_FLIP), flip, print_drm_mode_page_flip },
		{ ARG_STR(DRM_IOCTL_MODE_DIRTYFB), dirty_cmd, print_drm_mode_dirty_fb },
		{ ARG_STR(DRM_IOCTL_MODE_CREATE_DUMB), dumb, print_drm_mode_create_dumb },
		{ ARG_STR(DRM_IOCTL_MODE_MAP_DUMB), map_dumb, print_drm_mode_map_dumb },
		{ ARG_STR(DRM_IOCTL_MODE_DESTROY_DUMB), destroy_dumb, print_drm_mode_destroy_dumb },
		{ ARG_STR(DRM_IOCTL_GEM_CLOSE), close, print_drm_gem_close },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(a); i++) {
		test_drm(&a[i], NULL);
	}

	struct drm_check b = {
		.cmd = DRM_IOCTL_MODE_GETCONNECTOR,
		.arg_ptr = con,
		.print_arg = print_drm_mode_get_connector,
	};

	if (_IOC_SIZE(DRM_IOCTL_MODE_GETCONNECTOR) == 0x50) {
		b.cmd_str = "DRM_IOCTL_MODE_GETCONNECTOR";
	} else {
		b.cmd_str = con_str;
	}
	test_drm(&b, NULL);

	puts("+++ exited with 0 +++");
	return 0;
}
#else

SKIP_MAIN_UNDEFINED("HAVE_DRM_H && HAVE_DRM_DRM_H");

#endif
