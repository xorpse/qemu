#pragma once

#include "disas/bfd.h"

const uint64_t bfd_arch = bfd_arch_mips;
const uint64_t bfd_machine = 32 ; /* bfd_mach_mipsisa32 */
/* our bfd.h is so outdated, that it doesn't include it.*/
