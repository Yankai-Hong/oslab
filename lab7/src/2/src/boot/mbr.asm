%include "boot.inc"
org 0x7c00
[bits 16]
xor ax, ax ; eax = 0
; 初始化段寄存器, 段地址全部设为0
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

; 初始化栈指针
mov sp, 0x7c00    

mov ax, LOADER_START_SECTOR
mov cx, LOADER_SECTOR_COUNT
mov bx, LOADER_START_ADDRESS   

load_bootloader: 
    push ax
    push bx
    call asm_read_hard_disk  ; 读取硬盘
    add sp, 4
    inc ax
    add bx, 512
    loop load_bootloader

    ; 获取内存大小
    ; mov ax, 0xe801
    ; int 15h
    ; mov [0x7c00], ax
    ; mov [0x7c00+2], bx

    ; 获取内存大小
    call asm_read_memory

    jmp 0x0000:0x7e00        ; 跳转到bootloader

jmp $ ; 死循环

asm_read_memory:
    xor ebx, ebx                 ; EBX = 0，第一次调用
    xor ax, ax
    mov es, ax                   ; ES = 0，确保 ES:DI = 0x0000:0x0500
    mov di, 0x500                ; 缓冲区地址：ES:DI = 0x0000:0x0500

    mov edx, 0x534D4150          ; 'SMAP' 签名
    mov eax, 0xE820              ; E820 功能号
    mov ecx, 20                  ; 期望返回结构长度为 20 字节

.next_entry:
    push di                      ; 保存当前指针，以便失败时恢复
    int 0x15
    jc .fail                     ; CF=1 表示调用失败
    cmp eax, 0x534D4150
    jne .fail                    ; 返回签名不对，失败
    pop di                       ; 成功则恢复 DI

    add di, 20                   ; 指向下一个结构体写入地址
    cmp ebx, 0
    jne .next_entry              ; EBX != 0，说明还有数据可读

.success:
    mov ax, di                   ; DI 指向下一个空位，即总长度
    mov [0x7c00], ax             ; 写入到 0x7c00，用于 C 程序读取
    ret

.fail:
    pop di                       ; 弹出之前 push 的 DI
    mov ax, 0                    ; 表示失败
    mov [0x7c00], ax
    ret



; asm_read_hard_disk(memory,block)
; 加载逻辑扇区号为block的扇区到内存地址memory

asm_read_hard_disk:                           
    push bp
    mov bp, sp

    push ax
    push bx
    push cx
    push dx

    mov ax, [bp + 2 * 3] ; 逻辑扇区低16位

    mov dx, 0x1f3
    out dx, al    ; LBA地址7~0

    inc dx        ; 0x1f4
    mov al, ah
    out dx, al    ; LBA地址15~8

    xor ax, ax
    inc dx        ; 0x1f5
    out dx, al    ; LBA地址23~16 = 0

    inc dx        ; 0x1f6
    mov al, ah
    and al, 0x0f
    or al, 0xe0   ; LBA地址27~24 = 0
    out dx, al

    mov dx, 0x1f2
    mov al, 1
    out dx, al   ; 读取1个扇区

    mov dx, 0x1f7    ; 0x1f7
    mov al, 0x20     ;读命令
    out dx,al

    ; 等待处理其他操作
  .waits:
    in al, dx        ; dx = 0x1f7
    and al,0x88
    cmp al,0x08
    jnz .waits                         
    

    ; 读取512字节到地址ds:bx
    mov bx, [bp + 2 * 2]
    mov cx, 256   ; 每次读取一个字，2个字节，因此读取256次即可          
    mov dx, 0x1f0
  .readw:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .readw
      
    pop dx
    pop cx
    pop bx
    pop ax
    pop bp

    ret

times 510 - ($ - $$) db 0
db 0x55, 0xaa