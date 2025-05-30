
.data
_args_jmp_table qword _args_0, _args_1, _args_2, _args_3, _args_4
_ret_jmp_table qword _ret_int_0, _ret_int_1, _ret_int_2, _ret_float_4, _ret_int_4, 0, 0, _ret_float_8, _ret_int_8

;struct Value
;{
;	value_t val{};
;};

off_Value_val EQU 0
sizeof_Value EQU 8

;struct Signature
;{
;	uint32_t args_count = 0;
;	uint8_t ret_size = 0;
;	bool ret_is_float = false;
;	bool ret_is_big = false;
;};

off_Signature_args_count EQU 0
off_Signature_ret_size EQU 4
off_Signature_ret_is_float EQU 5
;off_Signature_ret_is_big EQU 6
sizeof_Signature EQU 16

.code

; void ccall(void *func, const Value* args, const Signature* sig)
ccall PROC
	mov r8, qword ptr [r8]
	mov qword ptr [rsp+32], rbx ; save rbx
	mov qword ptr [rsp+24], r8  ; rsp+24 *sig
	mov qword ptr [rsp+16], rdx ; rsp+16 args
	mov qword ptr [rsp+8], rcx  ; rsp+8  func

	mov r10, qword ptr [rsp+16] ; args
	mov r11d, dword ptr [rsp+24 + off_Signature_args_count] ; sig->args_count

	cmp r11, 4

	lea rbx, [r11*8]
	jae _args_size_checked
	mov rbx, 32
_args_size_checked:

	jbe _args_4_or_less

	mov rdx, -sizeof_Value

_stack_args_loop:
	mov rax, qword ptr [r10 + r11 * sizeof_Value - sizeof_Value]
	mov qword ptr [rsp+rdx], rax
	sub rdx, sizeof_Value
	dec r11
	cmp r11, 4
	ja _stack_args_loop

_args_4_or_less:

	lea rax, _args_jmp_table
	jmp qword ptr [rax + r11 * 8]
_jmp_from::

_args_4::
	mov r9, qword ptr [r10 + sizeof_Value * 3]
	movlpd xmm3, qword ptr [r10 + sizeof_Value * 3]

_args_3::
	mov r8, qword ptr [r10 + sizeof_Value * 2]
	movlpd xmm2, qword ptr [r10 + sizeof_Value * 2]

_args_2::
	mov rdx, qword ptr [r10 + sizeof_Value * 1]
	movlpd xmm1, qword ptr [r10 + sizeof_Value * 1]

_args_1::
	mov rcx, qword ptr [r10 + sizeof_Value * 0]
	movlpd xmm0, qword ptr [r10 + sizeof_Value * 0]

_args_0::

	mov rax, qword ptr [rsp+8] ; func
	sub rsp, rbx
	call rax
	add rsp, rbx

	mov rbx, qword ptr [rsp+32] ; restore rbx

	mov rdx, qword ptr [rsp+16] ; args
	sub rdx, sizeof_Value ; result ptr lays before args
	movzx rcx, byte ptr [rsp+24 + off_Signature_ret_size]
	movzx r8, byte ptr [rsp+24 + off_Signature_ret_is_float]

	test r8, r8
	jz _ret_int

_ret_float:
	sub rcx, 1

_ret_int:
	lea r9, _ret_jmp_table
	jmp qword ptr [r9 + rcx * 8]

_ret_float_8::
	mov rdx, qword ptr [rdx]
	movsd qword ptr [rdx], xmm0
	ret
_ret_float_4::
	mov rdx, qword ptr [rdx]
	movss dword ptr [rdx], xmm0
	ret
_ret_int_8::
	mov rdx, qword ptr [rdx]
	mov qword ptr [rdx], rax
	ret
_ret_int_4::
	mov rdx, qword ptr [rdx]
	mov dword ptr [rdx], eax
	ret
_ret_int_2::
	mov rdx, qword ptr [rdx]
	mov word ptr [rdx], ax
	ret
_ret_int_1::
	mov rdx, qword ptr [rdx]
	mov byte ptr [rdx], al
	ret
_ret_int_0::
	ret

ccall ENDP

END
