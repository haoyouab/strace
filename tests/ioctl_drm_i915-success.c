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
#  include <i915_drm.h>
# else
#  include <drm/drm.h>
#  include <drm/i915_drm.h>
# endif

# include "xlat.h"
# include "xlat/drm_i915_getparams.h"
# include "xlat/drm_i915_setparams.h"

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
print_drm_i915_gem_execbuffer2(long rc, void *ptr, void *arg)
{
	struct drm_i915_gem_execbuffer2 *eb = ptr;

	if (rc < 0) {
		printf("%p", eb);
		return;
	}
	printf("{buffers_ptr=%#llx", (unsigned long long) eb->buffers_ptr);
	PRINT_FIELD_U(", ", *eb, buffer_count);
	PRINT_FIELD_X(", ", *eb, batch_start_offset);
	PRINT_FIELD_U(", ", *eb, batch_len);
	PRINT_FIELD_U(", ", *eb, DR1);
	PRINT_FIELD_U(", ", *eb, DR4);
	PRINT_FIELD_U(", ", *eb, num_cliprects);
	printf(", cliprects_ptr=%#llx", (unsigned long long) eb->cliprects_ptr);
	PRINT_FIELD_X(", ", *eb, flags);
	printf("}");
}

static void
print_drm_i915_getparam(long rc, void *ptr, void *arg)
{
	struct drm_i915_getparam *param = ptr;

	if (rc < 0) {
		printf("%p", param);
		return;
	}
	printf("{param=");
	printxval(drm_i915_getparams, param->param, "I915_PARAM_???");
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
		long rc = ioctl(-1, DRM_IOCTL_I915_INIT, NULL);
		printf("ioctl(-1, DRM_IOCTL_I915_INIT, %s) = %s%s\n",
		       rc == inject_retval ? "NULL" : "0",
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
	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_i915_gem_execbuffer2, eb);
	eb->buffers_ptr = 0xffffffff;
	eb->buffer_count = 123;
	eb->batch_start_offset = 0x54;
	eb->batch_len = 20;
	eb->DR1 = 1;
	eb->DR4 = 4;
	eb->num_cliprects = 21;
	eb->cliprects_ptr = 0xffffffff;
	eb->flags = 0x20;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct drm_i915_getparam, getparam);
	getparam->param = I915_PARAM_IRQ_ACTIVE;

	struct drm_check a[] = {
		{ ARG_STR(DRM_IOCTL_I915_GEM_EXECBUFFER2), eb, print_drm_i915_gem_execbuffer2 },
		{ ARG_STR(DRM_IOCTL_I915_GETPARAM), getparam, print_drm_i915_getparam },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(a); i++) {
		test_drm(&a[i], NULL);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
#else

SKIP_MAIN_UNDEFINED("HAVE_DRM_H && HAVE_DRM_DRM_H");

#endif
