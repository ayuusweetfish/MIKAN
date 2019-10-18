#include "elf.h"

// Reference document
// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0044f/IHI0044F_aaelf.pdf

// ELF headers start from p. 17

static uint8_t check_ehdr(const elf_ehdr *ehdr)
{
    if (ehdr->ident[0] != 0x7f ||
        ehdr->ident[1] != 'E' ||
        ehdr->ident[2] != 'L' ||
        ehdr->ident[3] != 'F')
    {
        return ELF_E_INVALID;
    }
    if (ehdr->ident[4] != 1 ||      // class 32-bit ELFCLASS32
        ehdr->ident[5] != 1 ||      // data LSB     ELFDATA2LSB
        ehdr->ident[6] != 1 ||      // ELF version  EV_CURRENT
        ehdr->type != 2 ||          // type         ET_EXEC
        ehdr->machine != 40 ||      // machine      EM_ARM
        (ehdr->flags >> 24) != 5 || // ABI version  EF_ARM_ABIMASK
        !(ehdr->flags & 0x400))     // hard float   EF_ARM_ABI_FLOAT_HARD
    {
        return ELF_E_UNSUPPORT;
    }

    ELF_LOG("entry 0x%x\n", ehdr->entry);

    return ELF_E_NONE;
}

static inline const elf_shdr *get_shdr(const elf_ehdr *ehdr)
{
    return (const elf_shdr *)((const char *)ehdr + ehdr->shoffs);
}

static inline const char *get_strtab(const elf_ehdr *ehdr)
{
    return (ehdr->shstrndx == 0) ?  NULL :
        (const char *)ehdr + get_shdr(ehdr)[ehdr->shstrndx].offs;
}

static inline const char *get_strtab_ent(const elf_ehdr *ehdr, uint32_t offs)
{
    const char *table = get_strtab(ehdr);
    return table ? table + offs : NULL;
}

uint8_t load_elf(const char *buf)
{
    const elf_ehdr *ehdr = (elf_ehdr *)buf;

    uint8_t ehdr_result = check_ehdr(ehdr);
    if (ehdr_result != ELF_E_NONE) return ehdr_result;

    const elf_shdr *shdr = get_shdr(ehdr);
    for (uint32_t i = 0; i < ehdr->shnum; i++) {
        const elf_shdr *section = shdr + i;
        ELF_LOG("section type = 0x%x, flags = 0x%x, size = %d\n",
            section->type, section->flags, section->size);
    }

    return ELF_E_NONE;
}
