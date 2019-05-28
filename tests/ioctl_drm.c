#include "tests.h"

#if defined(HAVE_DRM_H) || defined(HAVE_DRM_DRM_H)

# include <errno.h>
# include <inttypes.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/ioctl.h>

# ifdef HAVE_DRM_H
#  include <drm.h>
# else
#  include <drm/drm.h>
# endif

# define TEST_NULL_ARG_EX(cmd, str)					\
	do {								\
		ioctl(-1, cmd, 0);					\
		printf("ioctl(-1, %s, NULL) = -1 EBADF (%m)\n", str);	\
	} while(0)

# define TEST_NULL_ARG(cmd) TEST_NULL_ARG_EX(cmd, #cmd)

static const unsigned long lmagic = (unsigned long) 0xdeadbeefbadc0dedULL;

int
main(void)
{
	TEST_NULL_ARG(DRM_IOCTL_VERSION);
	TEST_NULL_ARG(DRM_IOCTL_SET_VERSION);
	TEST_NULL_ARG(DRM_IOCTL_GET_UNIQUE);
	TEST_NULL_ARG(DRM_IOCTL_GET_MAGIC);
	TEST_NULL_ARG(DRM_IOCTL_WAIT_VBLANK);
	TEST_NULL_ARG(DRM_IOCTL_MODE_GETRESOURCES);
	TEST_NULL_ARG(DRM_IOCTL_MODE_GETCRTC);
	TEST_NULL_ARG(DRM_IOCTL_MODE_SETCRTC);
	TEST_NULL_ARG(DRM_IOCTL_MODE_CURSOR);
	TEST_NULL_ARG(DRM_IOCTL_MODE_CURSOR2);
	TEST_NULL_ARG(DRM_IOCTL_MODE_GETGAMMA);
	TEST_NULL_ARG(DRM_IOCTL_MODE_SETGAMMA);
	TEST_NULL_ARG(DRM_IOCTL_MODE_GETENCODER);
	TEST_NULL_ARG(DRM_IOCTL_MODE_GETPROPERTY);
	TEST_NULL_ARG(DRM_IOCTL_MODE_SETPROPERTY);
	TEST_NULL_ARG(DRM_IOCTL_MODE_GETPROPBLOB);
	TEST_NULL_ARG(DRM_IOCTL_MODE_GETFB);
	TEST_NULL_ARG(DRM_IOCTL_MODE_ADDFB);
	TEST_NULL_ARG(DRM_IOCTL_MODE_ADDFB2);
	TEST_NULL_ARG(DRM_IOCTL_MODE_RMFB);
	TEST_NULL_ARG(DRM_IOCTL_MODE_PAGE_FLIP);
	TEST_NULL_ARG(DRM_IOCTL_MODE_DIRTYFB);
	TEST_NULL_ARG(DRM_IOCTL_MODE_CREATE_DUMB);
	TEST_NULL_ARG(DRM_IOCTL_MODE_MAP_DUMB);
	TEST_NULL_ARG(DRM_IOCTL_MODE_DESTROY_DUMB);
	TEST_NULL_ARG(DRM_IOCTL_GEM_CLOSE);

	if (_IOC_SIZE(DRM_IOCTL_MODE_GETCONNECTOR) == 0x50) {
		TEST_NULL_ARG(DRM_IOCTL_MODE_GETCONNECTOR);
	} else {
		ioctl(-1, DRM_IOCTL_MODE_GETCONNECTOR, 0);
		printf("ioctl(-1, DRM_IOWR(%#lx, %#lx) /* DRM_IOCTL_MODE_GETCONNECTOR */, NULL) = -1 EBADF (%m)\n",
		       (long unsigned int)  _IOC_NR(DRM_IOCTL_MODE_GETCONNECTOR),
		       (long unsigned int) _IOC_SIZE(DRM_IOCTL_MODE_GETCONNECTOR));
	}

	ioctl(-1, _IOC(_IOC_READ, 0x45, 0x1, 0xff), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_READ, 0x45, 0x1, 0xff)", lmagic);

	ioctl(-1, _IOC(_IOC_WRITE, 0x45, 0x1, 0xff), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_WRITE, 0x45, 0x1, 0xff)", lmagic);

	ioctl(-1, _IOC(_IOC_READ|_IOC_WRITE, 0x45, 0xfe, 0xff), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_READ|_IOC_WRITE, 0x45, 0xfe, 0xff)", lmagic);

	ioctl(-1, _IOC(_IOC_READ|_IOC_WRITE, 0x45, 0, 0), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_READ|_IOC_WRITE, 0x45, 0, 0)", lmagic);

	puts("+++ exited with 0 +++");
	return 0;
}
#else

SKIP_MAIN_UNDEFINED("HAVE_DRM_H");

#endif
