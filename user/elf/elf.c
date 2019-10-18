#include "elf.h"

// Reference document
// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0044f/IHI0044F_aaelf.pdf

// ELF headers start from p. 17

uint8_t load_elf(const char *buf)
{
    const elf_ehdr *ehdr = (elf_ehdr *)buf;
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
