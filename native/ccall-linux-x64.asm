	SECTION .text
	global ccall

;struct CCallArgs
;{
;	ireg iregs[MAX_IREGS];   0  8 * 6
;	xmm xmms[MAX_XMMS];     48  8 * 8
;	const uint8_t* extra;  112  8
;	size_t extra_size;     120  8
;};
;
;struct CCallRet
;{
;	ireg iregs[MAX_IREGS];  0  8 * 2
;	xmm xmms[MAX_XMMS];    16  8 * 2
;	void* big_struct;      32  8
;};

; void ccall(void* func, const CCallArgs* args, CCallRet* ret)
ccall:
	push rbp
	sub rsp, 32
	mov [rsp     ], rdi ; func
	mov [rsp +  8], rsi ; args
	mov [rsp + 16], rdx ; ret
	mov [rsp + 24], rbx ; save rbx

	; ret->big_struct = args->iregs[0].ptr
	mov rax, [rsi + 0 * 8]
	mov [rdx + 32], rax

	mov rax, rsi ; args

	mov rsi, [rax + 112] ; from extra
	mov rbx, [rax + 120] ; extra_size
	lea rdi, [rsp]   ; to stack
	sub rdi, rbx
	mov rcx, rbx
	shr rcx, 3 ; extra_size / 8

	sub rsp, rbx ; decrease rsp early just in case of signal/interrupt

	cld
	rep movsq

	; iregs
	mov rdi, [rax + 0 * 8]
	mov rsi, [rax + 1 * 8]
	mov rdx, [rax + 2 * 8]
	mov rcx, [rax + 3 * 8]
	mov r8,  [rax + 4 * 8]
	mov r9,  [rax + 5 * 8]

	; xmms
	movsd xmm0, [rax + 48 + 0 * 8]
	movsd xmm1, [rax + 48 + 1 * 8]
	movsd xmm2, [rax + 48 + 2 * 8]
	movsd xmm3, [rax + 48 + 3 * 8]
	movsd xmm4, [rax + 48 + 4 * 8]
	movsd xmm5, [rax + 48 + 5 * 8]
	movsd xmm6, [rax + 48 + 6 * 8]
	movsd xmm7, [rax + 48 + 7 * 8]

	; pop rax ; func
	mov rax, [rsp + rbx]
	call rax
	add rsp, rbx

	mov rdi, [rsp + 16] ; ret

	mov [rdi + 0 * 8], rax
	mov [rdi + 1 * 8], rdx
	movsd [rdi + 16 + 0 * 8], xmm0
	movsd [rdi + 16 + 1 * 8], xmm1

	mov rbx, [rsp + 24] ; restore rbx
	add rsp, 32
	pop rbp
	ret
