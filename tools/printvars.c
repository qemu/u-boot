#define phys_to_virt
#define virt_to_phys
#define MAP_NOCACHE
#define MAP_WRCOMBINE
#define MAP_WRBACK
#define MAP_WRTHROUGH
#define map_physmem
#define unmap_physmem
#define BITS_PER_LONG 64
#include <stdio.h>
#include <stdbool.h>
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;
typedef unsigned long phys_addr_t;
typedef unsigned long phys_size_t;
typedef unsigned char uchar;
#define BIT(nr) (1UL << (nr))
#include <linux/kconfig.h>
#include <generated/autoconf.h>
#include <config.h>
#include <env_internal.h>

int main(int argc, char **argv) {
#define ROUND(a, b)              (((a) + (b) - 1) & ~((b) - 1))
#ifdef CONFIG_SYS_MALLOC_LEN
	printf("CONFIG_SYS_MALLOC_LEN=0x%x\n", CONFIG_SYS_MALLOC_LEN);
#endif
	return 0;
}
