#include "defs.h"
#include "print_fields.h"

#if defined(HAVE_AMDGPU_DRM_H) || defined(HAVE_DRM_AMDGPU_DRM_H)

# ifdef HAVE_AMDGPU_DRM_H
#  include <amdgpu_drm.h>
# else
#  include <drm/amdgpu_drm.h>
# endif

# include "xlat/drm_amdgpu_ioctls.h"

# include "xlat/drm_amdgpu_gem_domains.h"
# include "xlat/drm_amdgpu_gem_domain_flags.h"

static int
drm_amdgpu_gem_create(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_gem_create create;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &create))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{in={", create.in, bo_size);
		PRINT_FIELD_U(", ", create.in, alignment);
		tprints(", domains=");
		printxval(drm_amdgpu_gem_domains, create.in.domains, "AMDGPU_GEM_DOMAIN_???");
		tprints(", domain_flags=");
		printxval(drm_amdgpu_gem_domain_flags, create.in.domain_flags, "AMDGPU_GEM_CREATE_???");
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &create)) {
		PRINT_FIELD_U("{out={", create.out, handle);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_amdgpu_gem_mmap(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_gem_mmap mmap;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &mmap))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{in={", mmap.in, handle);
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &mmap)) {
		PRINT_FIELD_PTR("{out={", mmap.out, addr_ptr);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_amdgpu_ctx_op.h"
# include "xlat/drm_amdgpu_ctx_priority.h"
# include "xlat/drm_amdgpu_ctx_reset_status.h"
# include "xlat/drm_amdgpu_ctx_query2_flags.h"

static int
drm_amdgpu_ctx(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_ctx ctx;
	unsigned long op;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &ctx))
			return RVAL_IOCTL_DECODED;
		/* Store op because it will be used on exiting */
		op = ctx.in.op;
		set_tcb_priv_ulong(tcp, op);
		tprints("{in={op=");
		printxval(drm_amdgpu_ctx_op, ctx.in.op, "AMDGPU_CTX_OP_???");
		PRINT_FIELD_X(", ", ctx.in, flags);
		PRINT_FIELD_U(", ", ctx.in, ctx_id);
# ifdef HAVE_STRUCT_DRM_AMDGPU_CTX_IN_PRIORITY
		tprints(", priority=");
		printxval(drm_amdgpu_ctx_priority, ctx.in.priority, "AMDGPU_CTX_PRIORITY_???");
# endif
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &ctx)) {
		op = get_tcb_priv_ulong(tcp);
		tprints("{out={");
		switch (op) {
			case AMDGPU_CTX_OP_ALLOC_CTX:
				PRINT_FIELD_U("alloc={", ctx.out.alloc, ctx_id);
				tprints("}");
				break;
			case AMDGPU_CTX_OP_FREE_CTX:
				break;
			case AMDGPU_CTX_OP_QUERY_STATE:
				tprints("state={");
				PRINT_FIELD_X("", ctx.out.state, flags);
				PRINT_FIELD_U(", ", ctx.out.state, hangs);
				tprints(", reset_status=");
				printxval(drm_amdgpu_ctx_reset_status, ctx.out.state.reset_status, "AMDGPU_CTX_???_RESET");
				tprints("}");
				break;
# ifdef AMDGPU_CTX_OP_QUERY_STATE2
			case AMDGPU_CTX_OP_QUERY_STATE2:
				tprints("state={flags=");
				printxval(drm_amdgpu_ctx_query2_flags, ctx.out.state.flags, "AMDGPU_CTX_QUERY2_FLAGS_???");
				PRINT_FIELD_U(", ", ctx.out.state, hangs);
				tprints(", reset_status=");
				printxval(drm_amdgpu_ctx_reset_status, ctx.out.state.reset_status, "AMDGPU_CTX_???_RESET");
				tprints("}");
				break;
# endif
			default:
				tprints("}}");
				return RVAL_IOCTL_DECODED;
		}
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_amdgpu_bo_list_operation.h"

static int
drm_amdgpu_bo_list(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_bo_list list;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &list))
			return RVAL_IOCTL_DECODED;
		tprints("{in={operation=");
		printxval(drm_amdgpu_bo_list_operation, list.in.operation, "AMDGPU_BO_LIST_OP_???");
		PRINT_FIELD_U(", ", list.in, list_handle);
		PRINT_FIELD_U(", ", list.in, bo_number);
		PRINT_FIELD_U(", ", list.in, bo_info_size);
		PRINT_FIELD_PTR(", ", list.in, bo_info_ptr);
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &list)) {
		PRINT_FIELD_U("{out={", list.out, list_handle);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_amdgpu_cs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_cs cs;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &cs))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{in={", cs.in, ctx_id);
		PRINT_FIELD_U(", ", cs.in, bo_list_handle);
		PRINT_FIELD_U(", ", cs.in, num_chunks);
		PRINT_FIELD_U(", ", cs.in, chunks);
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &cs)) {
		PRINT_FIELD_U("{out={", cs.out, handle);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_amdgpu_info_query.h"

static int
drm_amdgpu_info(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_amdgpu_info info;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &info)) {
		PRINT_FIELD_PTR("{", info, return_pointer);
		PRINT_FIELD_U(", ", info, return_size);
		tprints(", query=");
		printxval(drm_amdgpu_info_query, info.query, "AMDGPU_INFO_???");
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_amdgpu_gem_metadata_op.h"

static int
drm_amdgpu_gem_metadata(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_amdgpu_gem_metadata metadata;
	unsigned long op;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &metadata))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{", metadata, handle);
		tprints(", op=");
		printxval(drm_amdgpu_gem_metadata_op, metadata.op, "AMDGPU_GEM_METADATA_OP_???");
		if (metadata.op == AMDGPU_GEM_METADATA_OP_SET_METADATA) {
			PRINT_FIELD_X(", data={", metadata.data, flags);
			PRINT_FIELD_U(", ", metadata.data, tiling_info);
			PRINT_FIELD_U(", ", metadata.data, data_size_bytes);
			/* Should we print all elements in data array ? */
			PRINT_FIELD_PTR(", ", metadata.data, data);
			tprints("}");
		} else {
			/* Since op is not a "set" operation, */
			/* we might use it on exiting tcb. */
			op = metadata.op;
			set_tcb_priv_ulong(tcp, op);
		}

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &metadata)) {
		op = get_tcb_priv_ulong(tcp);
		PRINT_FIELD_X(", data={", metadata.data, flags);
		PRINT_FIELD_U(", ", metadata.data, tiling_info);
		PRINT_FIELD_U(", ", metadata.data, data_size_bytes);
		/* Should we print all elements in data array ? */
		PRINT_FIELD_PTR(", ", metadata.data, data);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
drm_amdgpu_gem_wait_idle(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_gem_wait_idle idle;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &idle))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{in={", idle.in, handle);
		PRINT_FIELD_U(", ", idle.in, timeout);
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &idle)) {
		PRINT_FIELD_U("{out={", idle.out, status);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_amdgpu_gem_va_op.h"
# include "xlat/drm_amdgpu_gem_va_flags.h"

static int
drm_amdgpu_gem_va(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_amdgpu_gem_va va;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &va)) {
		PRINT_FIELD_U("{", va, handle);
		tprints(", operation=");
		printxval(drm_amdgpu_gem_va_op, va.operation, "AMDGPU_VA_OP_???");
		tprints(", flags=");
		printxval(drm_amdgpu_gem_va_flags, va.flags, "AMDGPU_VM_PAGE_???");
		PRINT_FIELD_ADDR(", ", va, va_address);
		PRINT_FIELD_X(", ", va, offset_in_bo);
		PRINT_FIELD_U(", ", va, map_size);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
drm_amdgpu_wait_cs(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_wait_cs wait_cs;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &wait_cs))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{in={", wait_cs.in, handle);
		PRINT_FIELD_U(", ", wait_cs.in, timeout);
		PRINT_FIELD_U(", ", wait_cs.in, ip_type);
		PRINT_FIELD_U(", ", wait_cs.in, ip_instance);
		PRINT_FIELD_U(", ", wait_cs.in, ring);
		PRINT_FIELD_U(", ", wait_cs.in, ctx_id);
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &wait_cs)) {
		PRINT_FIELD_U("{out={", wait_cs.out, status);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_amdgpu_gem_op_op.h"

static int
drm_amdgpu_gem_op(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_amdgpu_gem_op op;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &op)) {
		PRINT_FIELD_U("{", op, handle);
		tprints(", op=");
		printxval(drm_amdgpu_gem_op_op, op.op, "AMDGPU_GEM_OP_???");
		PRINT_FIELD_U(", ", op, value);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

# include "xlat/drm_amdgpu_gem_userptr_flags.h"

static int
drm_amdgpu_gem_userptr(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct drm_amdgpu_gem_userptr userptr;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &userptr))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_ADDR("{", userptr, addr);
		PRINT_FIELD_U(", ", userptr, size);
		tprints(", flags=");
		printxval(drm_amdgpu_gem_userptr_flags, userptr.flags, "AMDGPU_GEM_USERPTR_???");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &userptr)) {
		PRINT_FIELD_U(", ", userptr, handle);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

# ifdef DRM_IOCTL_AMDGPU_WAIT_FENCES
static int
drm_amdgpu_wait_fences(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_wait_fences fences;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &fences))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_PTR("{in={", fences.in, fences);
		PRINT_FIELD_U(", ", fences.in, fence_count);
		PRINT_FIELD_U(", ", fences.in, wait_all);
		PRINT_FIELD_U(", ", fences.in, timeout_ns);
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &fences)) {
		PRINT_FIELD_U("{out={", fences.out, status);
		PRINT_FIELD_U(", ", fences.out, first_signaled);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_AMDGPU_VM

# include "xlat/drm_amdgpu_vm_op.h"

static int
drm_amdgpu_vm(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_vm vm;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &vm)) {
		tprints("{in={op=");
		printxval(drm_amdgpu_vm_op, vm.in.op, "AMDGPU_VM_OP_???");
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_AMDGPU_FENCE_TO_HANDLE

# include "xlat/drm_amdgpu_fence_to_handle_what.h"

static int
drm_amdgpu_fence_to_handle(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_fence_to_handle handle;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &handle))
			return RVAL_IOCTL_DECODED;
		PRINT_FIELD_U("{in={fence={", handle.in.fence, ctx_id);
		PRINT_FIELD_U(", ", handle.in.fence, ip_type);
		PRINT_FIELD_U(", ", handle.in.fence, ip_instance);
		PRINT_FIELD_U(", ", handle.in.fence, ring);
		PRINT_FIELD_U(", ", handle.in.fence, seq_no);
		tprints("}, what=");
		printxval(drm_amdgpu_fence_to_handle_what, handle.in.what,
			  "AMDGPU_FENCE_TO_HANDLE_GET_???");
		tprints("}");

		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &handle)) {
		PRINT_FIELD_U("{out={", handle.out, handle);
		tprints("}");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif

# ifdef DRM_IOCTL_AMDGPU_SCHED

# include "xlat/drm_amdgpu_sched_op.h"
# include "xlat/drm_amdgpu_sched_priority.h"

static int
drm_amdgpu_sched(struct tcb *const tcp, const kernel_ulong_t arg)
{
	union drm_amdgpu_sched sched;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &sched)) {
		tprints("{in={op=");
		printxval(drm_amdgpu_sched_op, sched.in.op, "AMDGPU_SCHED_OP_???");
		PRINT_FIELD_U(", ", sched.in, fd);
		tprints(", priority=");
		printxval(drm_amdgpu_sched_priority, sched.in.priority, "DRM_SCHED_PRIORITY_???");
#  ifdef HAVE_STRUCT_DRM_AMDGPU_SCHED_IN_CTX_ID
		PRINT_FIELD_U(", ", sched.in, ctx_id);
#  elif HAVE_STRUCT_DRM_AMDGPU_SCHED_IN_FLAGS
		PRINT_FIELD_X(", ", sched.in, flags);
#  endif
		tprints("}}");
	}

	return RVAL_IOCTL_DECODED;
}
# endif

int
drm_amdgpu_decode_number(struct tcb *const tcp, const kernel_ulong_t arg)
{
	const char *str = xlookup(drm_amdgpu_ioctls, arg);

	if (str) {
		tprintf("%s", str);
		return IOCTL_NUMBER_STOP_LOOKUP;
	}

	return 0;
}

int
drm_amdgpu_ioctl(struct tcb *const tcp, const unsigned int code,
		 const kernel_ulong_t arg)
{
	switch (code) {
	case DRM_IOCTL_AMDGPU_GEM_CREATE:
		return drm_amdgpu_gem_create(tcp, arg);
	case DRM_IOCTL_AMDGPU_GEM_MMAP:
		return drm_amdgpu_gem_mmap(tcp, arg);
	case DRM_IOCTL_AMDGPU_CTX:
		return drm_amdgpu_ctx(tcp, arg);
	case DRM_IOCTL_AMDGPU_BO_LIST:
		return drm_amdgpu_bo_list(tcp, arg);
	case DRM_IOCTL_AMDGPU_CS:
		return drm_amdgpu_cs(tcp, arg);
	case DRM_IOCTL_AMDGPU_INFO:
		return drm_amdgpu_info(tcp, arg);
	case DRM_IOCTL_AMDGPU_GEM_METADATA:
		return drm_amdgpu_gem_metadata(tcp, arg);
	case DRM_IOCTL_AMDGPU_GEM_WAIT_IDLE:
		return drm_amdgpu_gem_wait_idle(tcp, arg);
	case DRM_IOCTL_AMDGPU_GEM_VA:
		return drm_amdgpu_gem_va(tcp, arg);
	case DRM_IOCTL_AMDGPU_WAIT_CS:
		return drm_amdgpu_wait_cs(tcp, arg);
	case DRM_IOCTL_AMDGPU_GEM_OP:
		return drm_amdgpu_gem_op(tcp, arg);
	case DRM_IOCTL_AMDGPU_GEM_USERPTR:
		return drm_amdgpu_gem_userptr(tcp, arg);
# ifdef DRM_IOCTL_AMDGPU_WAIT_FENCES
	case DRM_IOCTL_AMDGPU_WAIT_FENCES:
		return drm_amdgpu_wait_fences(tcp, arg);
# endif
# ifdef DRM_IOCTL_AMDGPU_VM
	case DRM_IOCTL_AMDGPU_VM:
		return drm_amdgpu_vm(tcp, arg);
# endif
# ifdef DRM_IOCTL_AMDGPU_FENCE_TO_HANDLE
	case DRM_IOCTL_AMDGPU_FENCE_TO_HANDLE:
		return drm_amdgpu_fence_to_handle(tcp, arg);
# endif
# ifdef DRM_IOCTL_AMDGPU_SCHED
	case DRM_IOCTL_AMDGPU_SCHED:
		return drm_amdgpu_sched(tcp, arg);
# endif
	default:
		tprints(", ");
		printaddr(arg);
		return RVAL_IOCTL_DECODED;
	}
}

#endif /* HAVE_AMDGPU_DRM_H || HAVE_DRM_NOUVEAU_DRM_H */
