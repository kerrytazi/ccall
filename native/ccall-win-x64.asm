.code

;struct CCallArgs
;{
;	ireg iregs[MAX_IREGS];  0  8 * 4
;	const uint8_t* extra;  32  8
;	size_t extra_size;     40  8
;};
;
;struct CCallRet
;{
;	ireg iregs[MAX_IREGS];  0  8 * 1
;	xmm xmms[MAX_XMMS];     8  8 * 1
;	void* big_struct;      16  8
;};

; void ccall(void* func, const CCallArgs* args, CCallRet* ret)
ccall PROC
	sub rsp, 64
	mov [rsp + 64 -  8], rcx ; func
	mov [rsp + 64 - 16], rdx ; args
	mov [rsp + 64 - 24], r8  ; ret
	mov [rsp + 64 - 32], rsi ; save rsi
	mov [rsp + 64 - 40], rdi ; save rdi
	mov [rsp + 64 - 48], rbx ; save rbx
	mov [rsp + 64 - 56], rbp ; save rbx

	; ret->big_struct = args->iregs[0].ptr
	mov rax, [rdx + 0 * 8]
	mov [r8 + 16], rax

	mov rax, rdx

	mov rsi, [rax + 32] ; from extra
	mov rbx, [rax + 40] ; extra_size

	mov rbp, rsp

	sub rsp, rbx ; decrease rsp early just in case of signal/interrupt
	sub rsp, 24

	mov r9, rsp
	and r9, 15
	jz _aligned
	sub rsp, 8

_aligned:

	lea rdi, [rsp + 32] ; to stack
	mov rcx, rbx
	shr rcx, 3 ; extra_size / 8

	cld
	rep movsq

	; iregs
	mov rcx, [rax + 0 * 8]
	mov rdx, [rax + 1 * 8]
	mov r8,  [rax + 2 * 8]
	mov r9,  [rax + 3 * 8]

	; xmms
	movsd xmm0, qword ptr [rax + 0 * 8]
	movsd xmm1, qword ptr [rax + 1 * 8]
	movsd xmm2, qword ptr [rax + 2 * 8]
	movsd xmm3, qword ptr [rax + 3 * 8]

	mov rax, [rbp + 64 - 8] ; func
	call rax
	mov rsp, rbp

	mov r8, [rsp + 64 - 24] ; ret

	mov [r8], rax
	movsd qword ptr [r8 + 8], xmm0

	mov rsi, [rsp + 64 - 32] ; restore rsi
	mov rdi, [rsp + 64 - 40] ; restore rdi
	mov rbx, [rsp + 64 - 48] ; restore rbx
	mov rbp, [rsp + 64 - 56] ; restore rbp
	add rsp, 64
	ret
ccall ENDP

END
