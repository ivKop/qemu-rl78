/*
 * Renesas RL78/G13 Promotion Board
 *
 * Copyright (c) 2011-2012 Andreas Färber
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "hw/hw.h"
#include "hw/boards.h"
#include "sysemu/sysemu.h"
#include "exec/memory.h"
#include "exec/address-spaces.h"
#include "qemu/error-report.h"
#include "hw/loader.h"
#include "elf.h"

#define CODE_FLASH_START 0x0
#define DATA_FLASH_START 0xF1000
#define RAM_END   0xFFEFF

static void rl78g13_pb_init(QEMUMachineInitArgs *args)
{
    RL78CPU *cpu;
    MemoryRegion *ram = g_new(MemoryRegion, 1);
    MemoryRegion *code_flash = g_new(MemoryRegion, 1);
    MemoryRegion *gpr = g_new(MemoryRegion, 1);
    MemoryRegion *sfr = g_new(MemoryRegion, 1);
    ram_addr_t code_flash_size = 64 * 1024;
    const char *cpu_model = args->cpu_model;
    Error *err = NULL;
    char *filename;
    int bios_size;

    /* init CPU */
    if (cpu_model == NULL) {
        cpu_model = "g13";
    }
    cpu = RL78_CPU(object_new(TYPE_RL78_CPU));

    /* allocate RAM */
    if (ram_size > 4096) {
        fprintf(stderr, "rl78g13_pb: Cannot model more than 4 KB of RAM.\n");
        exit(1);
    }
    ram_size = 4 * 1024;
    memory_region_init_ram(ram, NULL, "rl78g13_pb.ram", ram_size);
    memory_region_add_subregion(get_system_memory(), RAM_END - (ram_size - 1), ram);
    memory_region_init_ram(gpr, NULL, "rl78g13_pb.gpr", 32);
    memory_region_add_subregion(ram, ram_size - 32, gpr);

    /* allocate flash */
    memory_region_init_ram(code_flash, NULL, "rl78g13_pb.code_flash", code_flash_size);
    memory_region_set_readonly(code_flash, true);
    memory_region_add_subregion(get_system_memory(), CODE_FLASH_START, code_flash);
    if (bios_name == NULL) {
        fprintf(stderr, "Must specify BIOS file name.\n");
        exit(1);
    }
    filename = qemu_find_file(QEMU_FILE_TYPE_BIOS, bios_name);
    if (filename == NULL) {
        fprintf(stderr, "BIOS file not found.\n");
        exit(1);
    }
    bios_size = load_elf(filename, NULL, NULL, NULL,
                         NULL, NULL, 0, ELF_MACHINE, 0);
    g_free(filename);
    if (bios_size < 0 || bios_size > ram_size) {
        hw_error("qemu: could not load RL78 bios '%s' (%d)\n", bios_name, bios_size);
        exit(1);
    }

    /* allocate SFR area */
    memory_region_init_ram(sfr, NULL, "rl78g13_pb.sfr", 256);
    memory_region_add_subregion(get_system_memory(), 0xFFF00, sfr);

    object_property_set_bool(OBJECT(cpu), true, "realized", &err);
    if (err != NULL) {
        fprintf(stderr, "%s\n", error_get_pretty(err));
        error_free(err);
        exit(1);
    }
}

static QEMUMachine rl78g13_pb_machine = {
    .name = "rl78g13pb",
    .desc = "Renesas RL78/G13 Promotion Board",
    .init = rl78g13_pb_init,
    .max_cpus = 1,
    .is_default = 1,
};

static void rl78g13_pb_machine_init(void)
{
    qemu_register_machine(&rl78g13_pb_machine);
}

machine_init(rl78g13_pb_machine_init)
