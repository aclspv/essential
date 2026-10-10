#ifndef aclcur_reg_var_h
#define aclcur_reg_var_h

#include "../reg.h"
#include <ae2f/cc/profiles.h>

/** variable-kind register emission callbacks, see aclcur_regvar_in(). */
aclspv_compile_t aclcur_regvar_emit_reg(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regvar_emit_load(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regvar_emit_store(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);

/**
 * variable-kind register emission callbacks for explicit (volatile) loads
 * and stores: emit OpLoad / OpStore with the Volatile memory-access
 * operand. see aclcur_regvar_in().
 * */
aclspv_compile_t aclcur_regvar_emit_load_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regvar_emit_store_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);

/**
 * pointer producer for the parameterized load/store emission: fills
 * *wr_ptr with the SPIR-V pointer id to use as the memory operand for
 * register c_reg.
 *
 * the default producer is the identity: a register id IS its own
 * pointer (an OpVariable / OpFunctionParameter result id already
 * denotes its storage). a view chain (e.g. an ALIAS2 push-constant
 * member) mints a fresh OpAccessChain instead — the pointer operand may
 * differ from the register id.
 *
 * return 0 on success, an error code on failure.
 * */
typedef aclspv_compile_t aclcur_regvar_ptr_t(
		aclspv_id_t c_reg
		, aclspv_id_t* ae2f_restrict wr_ptr
		, h_aclspv_ctx_t h_ctx
		);

/**
 * parameterized load emission: like aclcur_regvar_emit_load, but the
 * OpLoad memory operand comes from fn_ptr instead of the register id.
 *
 * the phi materialization is pointer-independent: the OpPhi short-circuit
 * is taken under exactly the stock conditions, and only the OpLoad path
 * consults fn_ptr. c_volatile selects the explicit (volatile) form,
 * which skips the phi path and appends the Volatile memory-access
 * operand.
 *
 * fn_ptr is deliberately unqualified: C99 `restrict` may only qualify
 * pointers to object types, not pointers to functions.
 * */
aclspv_compile_t aclcur_regvar_emit_loadof(
		aclspv_id_t c_id
		, h_aclspv_ctx_t h_ctx
		, aclcur_regvar_ptr_t* fn_ptr
		, aclspv_bool_t c_volatile
		);

/**
 * parameterized store emission: like aclcur_regvar_emit_store, but the
 * OpStore memory operand comes from fn_ptr instead of the register id.
 *
 * c_volatile selects the explicit variant, which skips the
 * ACLCUR_FLAG0_STORE_EXPLICIT gating: a non-explicit store is a pure
 * SSA definition and materializes no memory write.
 * */
aclspv_compile_t aclcur_regvar_emit_storeof(
		aclspv_id_t c_id
		, h_aclspv_ctx_t h_ctx
		, aclcur_regvar_ptr_t* fn_ptr
		, aclspv_bool_t c_volatile
		);

/**
 * address of a variable-kind register.
 *
 * an OpVariable / OpFunctionParameter result id is already typed
 * `pointer<StorageClass, T>` and denotes the storage it declares, so the
 * register id IS the address of its own storage: `&x` mints no opcode.
 * */
aclspv_id_t aclcur_regvar_addr(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

/** byte size of a variable-kind register's storage: the size of its type. */
aclconst_specsz aclcur_regvar_size(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

/**
 * stock "variable" register interface: one storage location per register.
 *
 * - aclcur_regvar_emit_reg emits the OpVariable into the function body
 *   section for Function storage, and into the module variables section
 *   otherwise;
 * - aclcur_regvar_emit_load / aclcur_regvar_emit_store emit OpLoad /
 *   OpStore into the function body section, the register id doubling as
 *   variable/result id;
 * - aclcur_regvar_emit_load_explicit / aclcur_regvar_emit_store_explicit
 *   are the explicit (volatile) variants: the same ops with the Volatile
 *   memory-access operand.
 *
 * - aclcur_regvar_emit_loadof / aclcur_regvar_emit_storeof are the
 *   parameterized forms: the OpLoad / OpStore memory operand is supplied
 *   by a caller-provided pointer producer instead of the register id —
 *   the pointer may differ from the register id, e.g. an ALIAS2 view
 *   chain whose pointer is a 2-hop OpAccessChain. the OpPhi
 *   short-circuit and the explicit/non-explicit gating are shared with
 *   the stock hooks above;
 *
 * - aclcur_regvar_addr is the id passthrough described above;
 *
 * - aclcur_regvar_size is the byte size of the declared type;
 *
 * callers that already emitted the variable by themselves (module-scope
 * parameters) pass a copy of this interface with m_emit_reg zeroed.
 *
 * note on push-constant parameters: the parameter register id is NOT
 * its variable id — h_ctx->m_id_push_constant names the packed-struct
 * OpVariable result id shared by every push-constant parameter, while
 * the parameter itself holds a distinct register id (see
 * aclemit/fndecl/entp/args.c). this split is by design; the future
 * shape is a view (aclcur_reg_view) over the packed struct with the
 * member index. documented here, not fixed here.
 * */
ae2f_inline_f static ae2f_ccconst aclcur_reg_t aclcur_regvar_in(void) {
	const aclcur_reg_t in = {
		{ 0, 0, aclcur_regvar_addr, aclcur_regvar_size, aclcur_regvar_emit_reg, aclcur_regvar_emit_load, aclcur_regvar_emit_load_explicit, aclcur_regvar_emit_store, aclcur_regvar_emit_store_explicit },
		{ { 0 } },
		0, 0, 0, 0, 0
	};
	return in;
}

#endif

