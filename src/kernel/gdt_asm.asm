; gdt flush assembly stub
; (c) 2025 HorizonOS Project
;

global gdt_flush

gdt_flush:
    mov eax, [esp+4]    ; get pointer to gdt pointer
    lgdt [eax]          ; loads gdt
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush     ; far jump to reload cs
.flush:
    ret