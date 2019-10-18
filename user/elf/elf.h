#ifndef __MIKAN__ELF_H__
#define __MIKAN__ELF_H__

#include <stdint.h>

typedef uint16_t elf_half;
typedef uint32_t elf_offs;
typedef uint32_t elf_addr;
typedef uint32_t elf_word;
typedef int32_t elf_sword;

typedef struct {
    uint8_t ident[16];
    elf_half type;
    elf_half machine;
    elf_word version;
    elf_addr entry;
    elf_offs phoffs;
    elf_offs shoffs;
    elf_word flags;
    elf_half ehsize;
    elf_half phentsize;
    elf_half phnum;
    elf_half shentsize;
    elf_half shnum;
    elf_half shstrndx;
} elf_ehdr;

#define ELF_E_NONE      0
#define ELF_E_INVALID   1
#define ELF_E_UNSUPPORT 2

uint8_t load_elf(const char *buf);

#include <stdio.h>
#define ELF_LOG printf

#endif
