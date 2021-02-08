section .multiboot_header

header_start:
  dd 0xe85250d6                     ; magic number (multiboot 2)
  dd 0                              ; architecture 0 (32bit i386)
  dd header_end - header_start      ; header size

  ; checksum (magic number + architecture + header size)
  ; has to be 0U when added with the other magic fields
  ; 0x100000000(33bits) -> 0x00000000(32bit)
  dd 0x100000000 - (0xe85250d6 + (header_end - header_start))

  dw 0       ; type
  dw 0       ; flags
  dd 8       ; size
header_end: