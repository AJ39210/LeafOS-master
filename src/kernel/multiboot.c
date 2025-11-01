#include "kernel.h"

static multiboot_info_t *saved_mbi = NULL;

void multiboot_store_info(multiboot_info_t *mbi) {
    saved_mbi = mbi;
}

multiboot_info_t *get_multiboot_info(void) {
    return saved_mbi;
}

uint32_t get_initrd_address(void) {
    if (!saved_mbi || !(saved_mbi->flags & 0x8)) {
        return 0;  /* Modules not present */
    }

    if (saved_mbi->mods_count < 1) {
        return 0;  /* No modules loaded */
    }

    multiboot_module_t *mods = (multiboot_module_t *)saved_mbi->mods_addr;
    return mods[0].mod_start;
}

uint32_t get_initrd_size(void) {
    if (!saved_mbi || !(saved_mbi->flags & 0x8)) {
        return 0;
    }

    if (saved_mbi->mods_count < 1) {
        return 0;
    }

    multiboot_module_t *mods = (multiboot_module_t *)saved_mbi->mods_addr;
    return mods[0].mod_end - mods[0].mod_start;
}
