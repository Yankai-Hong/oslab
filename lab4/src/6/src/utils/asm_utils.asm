[bits 32]

global asm_lidt
global asm_unhandled_interrupt
global asm_unhandled_interrupt_new
global asm_halt

ASM_UNHANDLED_INTERRUPT_INFO db '23336087'
                             db 0
ASM_IDTR dw 0
         dd 0

; void asm_unhandled_interrupt()
asm_unhandled_interrupt:
    cli
    mov esi, ASM_UNHANDLED_INTERRUPT_INFO
    xor ebx, ebx
    mov ah, 0x03

.output_information:
    cmp byte[esi], 0
    je .end
    mov al, byte[esi]
    mov word[gs:bx], ax

    inc esi
    add ebx, 2
    inc ah
    
    jmp .output_information
.end:
    jmp $

; void asm_unhandled_interrupt_new()
asm_unhandled_interrupt_new:
    cli

    mov esi, ASM_UNHANDLED_INTERRUPT_INFO

    ; 设置光标到 (0, 0)
    mov ah, 02h
    mov bh, 0
    mov dh, 0
    mov dl, 0
    int 10h

    mov bh, 0
    mov bl, 1
    mov cx, 1

print_number:
    mov al, [esi]
    cmp al, 0
    je done

    mov ah, 02h
    int 10h

    mov ah, 0x09
    int 10h

    inc esi
    ; inc bl
    inc dl
    jmp print_number

done:
    sti
    jmp $

; void asm_lidt(uint32 start, uint16 limit)
asm_lidt:
    push ebp
    mov ebp, esp
    push eax

    mov eax, [ebp + 4 * 3]
    mov [ASM_IDTR], ax
    mov eax, [ebp + 4 * 2]
    mov [ASM_IDTR + 2], eax
    lidt [ASM_IDTR]

    pop eax
    pop ebp
    ret


asm_halt:
    jmp $