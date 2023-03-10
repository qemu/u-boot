// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for fdt command
 *
 * Copyright 2022 Google LLCmap_to_sysmem(fdt));
 */

#include <common.h>
#include <console.h>
#include <fdt_support.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <test/suites.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;
/*
 * Missing tests:
 * fdt boardsetup         - Do board-specific set up
 * fdt checksign [<addr>] - check FIT signature
 *                          <addr> - address of key blob
 *                                   default gd->fdt_blob
 */

/* Declare a new fdt test */
#define FDT_TEST(_name, _flags)	UNIT_TEST(_name, _flags, fdt_test)

/**
 * make_test_fdt() - Create an FDT with just a root node
 *
 * The size is set to the minimum needed
 *
 * @uts: Test state
 * @fdt: Place to write FDT
 * @size: Maximum size of space for fdt
 */
static int make_test_fdt(struct unit_test_state *uts, void *fdt, int size)
{
	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	return 0;
}

/**
 * make_fuller_fdt() - Create an FDT with root node and properties
 *
 * The size is set to the minimum needed
 *
 * @uts: Test state
 * @fdt: Place to write FDT
 * @size: Maximum size of space for fdt
 */
static int make_fuller_fdt(struct unit_test_state *uts, void *fdt, int size)
{
	fdt32_t regs[2] = { cpu_to_fdt32(0x1234), cpu_to_fdt32(0x1000) };

	/*
	 * Assemble the following DT for test purposes:
	 *
	 * / {
	 * 	#address-cells = <0x00000001>;
	 * 	#size-cells = <0x00000001>;
	 * 	compatible = "u-boot,fdt-test";
	 * 	model = "U-Boot FDT test";
	 *
	 *	aliases {
	 *		badalias = "/bad/alias";
	 *		subnodealias = "/test-node@1234/subnode";
	 *		testnodealias = "/test-node@1234";
	 *	};
	 *
	 * 	test-node@1234 {
	 * 		#address-cells = <0x00000000>;
	 * 		#size-cells = <0x00000000>;
	 * 		compatible = "u-boot,fdt-test-device1";
	 * 		clock-names = "fixed", "i2c", "spi", "uart2", "uart1";
	 * 		u-boot,empty-property;
	 * 		clock-frequency = <0x00fde800>;
	 * 		regs = <0x00001234 0x00001000>;
	 *
	 * 		subnode {
	 * 			#address-cells = <0x00000000>;
	 * 			#size-cells = <0x00000000>;
	 * 			compatible = "u-boot,fdt-subnode-test-device";
	 * 		};
	 * 	};
	 * };
	 */

	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);

	ut_assertok(fdt_property_u32(fdt, "#address-cells", 1));
	ut_assertok(fdt_property_u32(fdt, "#size-cells", 1));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "compatible", "u-boot,fdt-test"));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "model", "U-Boot FDT test"));

	ut_assert(fdt_begin_node(fdt, "aliases") >= 0);
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "badalias", "/bad/alias"));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "subnodealias", "/test-node@1234/subnode"));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "testnodealias", "/test-node@1234"));
	ut_assertok(fdt_end_node(fdt));

	ut_assert(fdt_begin_node(fdt, "test-node@1234") >= 0);
	ut_assertok(fdt_property_cell(fdt, "#address-cells", 0));
	ut_assertok(fdt_property_cell(fdt, "#size-cells", 0));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "compatible", "u-boot,fdt-test-device1"));
	/* <stringlist> */
	ut_assertok(fdt_property(fdt, "clock-names", "fixed\0i2c\0spi\0uart2\0uart1\0", 26));
	/* <empty> */
	ut_assertok(fdt_property(fdt, "u-boot,empty-property", NULL, 0));
	/*
	 * <u32>
	 * This value is deliberate as it used to break cmd/fdt.c
	 * is_printable_string() implementation.
	 */
	ut_assertok(fdt_property_u32(fdt, "clock-frequency", 16640000));
	/* <prop-encoded-array> */
	ut_assertok(fdt_property(fdt, "regs", &regs, sizeof(regs)));
	ut_assert(fdt_begin_node(fdt, "subnode") >= 0);
	ut_assertok(fdt_property_cell(fdt, "#address-cells", 0));
	ut_assertok(fdt_property_cell(fdt, "#size-cells", 0));
	ut_assertok(fdt_property_string(fdt, "compatible", "u-boot,fdt-subnode-test-device"));
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_end_node(fdt));

	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	return 0;
}

/* Test 'fdt addr' getting/setting address */
static int fdt_test_addr(struct unit_test_state *uts)
{
	const void *fdt_blob, *new_fdt;
	char fdt[256];
	ulong addr;
	int ret;

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt addr -c", 0));
	ut_assert_nextline("Control fdt: %08lx",
			   (ulong)map_to_sysmem(gd->fdt_blob));
	ut_assertok(ut_check_console_end(uts));

	/* The working fdt is not set, so this should fail */
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");
	ut_asserteq(CMD_RET_FAILURE, run_command("fdt addr", 0));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	/* Set up a working FDT and try again */
	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(run_command("fdt addr", 0));
	ut_assert_nextline("Working fdt: %08lx", (ulong)map_to_sysmem(fdt));
	ut_assertok(ut_check_console_end(uts));

	/* Set the working FDT */
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");
	ut_assertok(run_commandf("fdt addr %08x", addr));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_asserteq(addr, map_to_sysmem(working_fdt));
	ut_assertok(ut_check_console_end(uts));
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");

	/* Set the control FDT */
	fdt_blob = gd->fdt_blob;
	gd->fdt_blob = NULL;
	ret = run_commandf("fdt addr -c %08x", addr);
	new_fdt = gd->fdt_blob;
	gd->fdt_blob = fdt_blob;
	ut_assertok(ret);
	ut_asserteq(addr, map_to_sysmem(new_fdt));
	ut_assertok(ut_check_console_end(uts));

	/* Test setting an invalid FDT */
	fdt[0] = 123;
	ut_asserteq(1, run_commandf("fdt addr %08x", addr));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	/* Test detecting an invalid FDT */
	fdt[0] = 123;
	set_working_fdt_addr(addr);
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_asserteq(1, run_commandf("fdt addr"));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_addr, UT_TESTF_CONSOLE_REC);

/* Test 'fdt addr' resizing an fdt */
static int fdt_test_addr_resize(struct unit_test_state *uts)
{
	char fdt[256];
	const int newsize = sizeof(fdt) / 2;
	ulong addr;

	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test setting and resizing the working FDT to a larger size */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt addr %08x %x", addr, newsize));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(ut_check_console_end(uts));

	/* Try shrinking it */
	ut_assertok(run_commandf("fdt addr %08x %x", addr, sizeof(fdt) / 4));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assert_nextline("New length %d < existing length %d, ignoring",
			   (int)sizeof(fdt) / 4, newsize);
	ut_assertok(ut_check_console_end(uts));

	/* ...quietly */
	ut_assertok(run_commandf("fdt addr -q %08x %x", addr, sizeof(fdt) / 4));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(ut_check_console_end(uts));

	/* We cannot easily provoke errors in fdt_open_into(), so ignore that */

	return 0;
}
FDT_TEST(fdt_test_addr_resize, UT_TESTF_CONSOLE_REC);

static int fdt_test_move(struct unit_test_state *uts)
{
	char fdt[256];
	ulong addr, newaddr = 0x10000;
	const int size = sizeof(fdt);
	uint32_t ts;
	void *buf;

	/* Original source DT */
	ut_assertok(make_test_fdt(uts, fdt, size));
	ts = fdt_totalsize(fdt);
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Moved target DT location */
	buf = map_sysmem(newaddr, size);
	memset(buf, 0, size);

	/* Test moving the working FDT to a new location */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt move %08x %08x %x", addr, newaddr, ts));
	ut_assert_nextline("Working FDT set to %lx", newaddr);
	ut_assertok(ut_check_console_end(uts));

	/* Compare the source and destination DTs */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("cmp.b %08x %08x %x", addr, newaddr, ts));
	ut_assert_nextline("Total of %d byte(s) were the same", ts);
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_move, UT_TESTF_CONSOLE_REC);

static int fdt_test_resize(struct unit_test_state *uts)
{
	char fdt[256];
	const unsigned int newsize = 0x2000;
	uint32_t ts;
	ulong addr;

	/* Original source DT */
	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	fdt_shrink_to_minimum(fdt, 0);	/* Resize with 0 extra bytes */
	ts = fdt_totalsize(fdt);
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test resizing the working FDT and verify the new space was added */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt resize %x", newsize));
	ut_asserteq(ts + newsize, fdt_totalsize(fdt));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_resize, UT_TESTF_CONSOLE_REC);

/* Test 'fdt get value' reading an fdt */
static int fdt_test_get_value_string(struct unit_test_state *uts,
				     const char *node, const char *prop,
				     const char *idx,  const char *strres,
				     const int intres)
{
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt get value var %s %s %s",
				 node, prop, idx ? : ""));
	if (strres) {
		ut_asserteq_str(strres, env_get("var"));
	} else {
		ut_asserteq(intres, env_get_hex("var", 0x1234));
	}
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_value_common(struct unit_test_state *uts,
				     const char *node)
{
	/* Test getting default element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", NULL, "fixed", 0);

	/* Test getting 0th element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", "0", "fixed", 0);

	/* Test getting 1st element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", "1", "i2c", 0);

	/* Test getting 2nd element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", "2", "spi", 0);

	/*
	 * Test getting default element of $node node regs property.
	 * The result here is highly unusual, the non-index value read from
	 * integer array is a string of concatenated values from the array,
	 * but only if the array is shorter than 40 characters. Anything
	 * longer is an error. This is a special case for handling hashes.
	 */
	fdt_test_get_value_string(uts, node, "regs", NULL, "3412000000100000", 0);

	/* Test getting 0th element of $node node regs property */
	fdt_test_get_value_string(uts, node, "regs", "0", NULL, 0x1234);

	/* Test getting 1st element of $node node regs property */
	fdt_test_get_value_string(uts, node, "regs", "1", NULL, 0x1000);

	/* Test missing 10th element of $node node clock-names property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt get value ften %s clock-names 10", node));
	ut_assertok(ut_check_console_end(uts));

	/* Test missing 10th element of $node node regs property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt get value ften %s regs 10", node));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of $node node nonexistent property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt get value fnone %s nonexistent", node));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_value(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;
	int ret;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	ret = fdt_test_get_value_common(uts, "/test-node@1234");
	if (!ret)
		ret = fdt_test_get_value_common(uts, "testnodealias");
	if (ret)
		return ret;

	/* Test getting default element of /nonexistent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value fnode /nonexistent nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value vbadalias badalias nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of nonexistent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value vnoalias noalias nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get_value, UT_TESTF_CONSOLE_REC);

static int fdt_test_get_name(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test getting name of node 0 in /, which is /aliases node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name nzero / 0", 0));
	ut_asserteq_str("aliases", env_get("nzero"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node 1 in /, which is /test-node@1234 node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name none / 1", 0));
	ut_asserteq_str("test-node@1234", env_get("none"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node -1 in /, which is /aliases node, same as 0 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name nmone / -1", 0));
	ut_asserteq_str("aliases", env_get("nmone"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node 2 in /, which does not exist */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name ntwo / 2", 1));
	ut_assert_nextline("libfdt node not found");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node 0 in /test-node@1234, which is /subnode node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name snzero /test-node@1234 0", 0));
	ut_asserteq_str("subnode", env_get("snzero"));
	ut_assertok(run_command("fdt get name asnzero testnodealias 0", 0));
	ut_asserteq_str("subnode", env_get("asnzero"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node 1 in /test-node@1234, which does not exist */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name snone /test-node@1234 1", 1));
	ut_assert_nextline("libfdt node not found");
	ut_asserteq(1, run_command("fdt get name asnone testnodealias 1", 1));
	ut_assert_nextline("libfdt node not found");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node -1 in /test-node@1234, which is /subnode node, same as 0 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name snmone /test-node@1234 -1", 0));
	ut_asserteq_str("subnode", env_get("snmone"));
	ut_assertok(run_command("fdt get name asnmone testnodealias -1", 0));
	ut_asserteq_str("subnode", env_get("asnmone"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of nonexistent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name nonode /nonexistent 0", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name vbadalias badalias 0", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of nonexistent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name vnoalias noalias 0", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get_name, UT_TESTF_CONSOLE_REC);

static int fdt_test_get_addr_common(struct unit_test_state *uts, char *fdt,
				    const char *path, const char *prop)
{
	unsigned int offset;
	int path_offset;
	void *prop_ptr;
	int len = 0;

	path_offset = fdt_path_offset(fdt, path);
	ut_assert(path_offset >= 0);
	prop_ptr = (void *)fdt_getprop(fdt, path_offset, prop, &len);
	ut_assertnonnull(prop_ptr);
	offset = (char *)prop_ptr - fdt;

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt get addr pstr %s %s", path, prop));
	ut_asserteq((ulong)map_sysmem(env_get_hex("fdtaddr", 0x1234), 0),
		    (ulong)(map_sysmem(env_get_hex("pstr", 0x1234), 0) - offset));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_addr(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test getting address of root node / string property "compatible" */
	fdt_test_get_addr_common(uts, fdt, "/", "compatible");

	/* Test getting address of node /test-node@1234 stringlist property "clock-names" */
	fdt_test_get_addr_common(uts, fdt, "/test-node@1234", "clock-names");
	fdt_test_get_addr_common(uts, fdt, "testnodealias", "clock-names");

	/* Test getting address of node /test-node@1234 u32 property "clock-frequency" */
	fdt_test_get_addr_common(uts, fdt, "/test-node@1234", "clock-frequency");
	fdt_test_get_addr_common(uts, fdt, "testnodealias", "clock-frequency");

	/* Test getting address of node /test-node@1234 empty property "u-boot,empty-property" */
	fdt_test_get_addr_common(uts, fdt, "/test-node@1234", "u-boot,empty-property");
	fdt_test_get_addr_common(uts, fdt, "testnodealias", "u-boot,empty-property");

	/* Test getting address of node /test-node@1234 array property "regs" */
	fdt_test_get_addr_common(uts, fdt, "/test-node@1234", "regs");
	fdt_test_get_addr_common(uts, fdt, "testnodealias", "regs");

	/* Test getting address of node /test-node@1234/subnode non-existent property "noprop" */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get addr pnoprop /test-node@1234/subnode noprop", 1));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting address of non-existent node /test-node@1234/nonode@1 property "noprop" */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get addr pnonode /test-node@1234/nonode@1 noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get_addr, UT_TESTF_CONSOLE_REC);

static int fdt_test_get_size_common(struct unit_test_state *uts,
				     const char *path, const char *prop,
				     const unsigned int val)
{
	ut_assertok(console_record_reset_enable());
	if (prop) {
		ut_assertok(run_commandf("fdt get size sstr %s %s", path, prop));
	} else {
		ut_assertok(run_commandf("fdt get size sstr %s", path));
	}
	ut_asserteq(val, env_get_hex("sstr", 0x1234));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_size(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test getting size of root node / string property "compatible" */
	fdt_test_get_size_common(uts, "/", "compatible", 16);

	/* Test getting size of node /test-node@1234 stringlist property "clock-names" */
	fdt_test_get_size_common(uts, "/test-node@1234", "clock-names", 26);
	fdt_test_get_size_common(uts, "testnodealias", "clock-names", 26);

	/* Test getting size of node /test-node@1234 u32 property "clock-frequency" */
	fdt_test_get_size_common(uts, "/test-node@1234", "clock-frequency", 4);
	fdt_test_get_size_common(uts, "testnodealias", "clock-frequency", 4);

	/* Test getting size of node /test-node@1234 empty property "u-boot,empty-property" */
	fdt_test_get_size_common(uts, "/test-node@1234", "u-boot,empty-property", 0);
	fdt_test_get_size_common(uts, "testnodealias", "u-boot,empty-property", 0);

	/* Test getting size of node /test-node@1234 array property "regs" */
	fdt_test_get_size_common(uts, "/test-node@1234", "regs", 8);
	fdt_test_get_size_common(uts, "testnodealias", "regs", 8);

	/* Test getting node count of node / */
	fdt_test_get_size_common(uts, "/", NULL, 2);

	/* Test getting node count of node /test-node@1234/subnode */
	fdt_test_get_size_common(uts, "/test-node@1234/subnode", NULL, 0);
	fdt_test_get_size_common(uts, "subnodealias", NULL, 0);

	/* Test getting size of node /test-node@1234/subnode non-existent property "noprop" */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnoprop /test-node@1234/subnode noprop", 1));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_asserteq(1, run_command("fdt get size pnoprop subnodealias noprop", 1));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting size of non-existent node /test-node@1234/nonode@1 property "noprop" */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnonode /test-node@1234/nonode@1 noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting node count of non-existent node /test-node@1234/nonode@1 */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnonode /test-node@1234/nonode@1", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting node count of bad alias badalias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnonode badalias noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting node count of non-existent alias noalias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnonode noalias", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get_size, UT_TESTF_CONSOLE_REC);

static int fdt_test_set_single(struct unit_test_state *uts,
			       const char *path, const char *prop,
			       const char *sval, int ival, bool integer)
{
	/*
	 * Set single element string/integer/<empty> property into DT, that is:
	 * => fdt set /path property string
	 * => fdt set /path property integer
	 * => fdt set /path property
	 */
	ut_assertok(console_record_reset_enable());
	if (sval)
		ut_assertok(run_commandf("fdt set %s %s %s", path, prop, sval));
	else if (integer)
		ut_assertok(run_commandf("fdt set %s %s <%d>", path, prop, ival));
	else
		ut_assertok(run_commandf("fdt set %s %s", path, prop));

	/* Validate the property is present and has correct value. */
	ut_assertok(run_commandf("fdt get value svar %s %s", path, prop));
	if (sval)
		ut_asserteq_str(sval, env_get("svar"));
	else if (integer)
		ut_asserteq(ival, env_get_hex("svar", 0x1234));
	else
		ut_assertnull(env_get("svar"));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_set_multi(struct unit_test_state *uts,
			      const char *path, const char *prop,
			      const char *sval1, const char *sval2,
			      int ival1, int ival2)
{
	/*
	 * Set multi element string/integer array property in DT, that is:
	 * => fdt set /path property <string1 string2>
	 * => fdt set /path property <integer1 integer2>
	 *
	 * The set is done twice in here deliberately, The first set adds
	 * the property with an extra trailing element in its array to make
	 * the array longer, the second set is the expected final content of
	 * the array property. The longer array is used to verify that the
	 * new array is correctly sized and read past the new array length
	 * triggers failure.
	 */
	ut_assertok(console_record_reset_enable());
	if (sval1 && sval2) {
		ut_assertok(run_commandf("fdt set %s %s %s %s end", path, prop, sval1, sval2));
		ut_assertok(run_commandf("fdt set %s %s %s %s", path, prop, sval1, sval2));
	} else {
		ut_assertok(run_commandf("fdt set %s %s <%d %d 10>", path, prop, ival1, ival2));
		ut_assertok(run_commandf("fdt set %s %s <%d %d>", path, prop, ival1, ival2));
	}

	/*
	 * Validate the property is present and has correct value.
	 *
	 * The "end/10" above and "svarn" below is used to validate that
	 * previous 'fdt set' to longer array does not polute newly set
	 * shorter array.
	 */
	ut_assertok(run_commandf("fdt get value svar1 %s %s 0", path, prop));
	ut_assertok(run_commandf("fdt get value svar2 %s %s 1", path, prop));
	ut_asserteq(1, run_commandf("fdt get value svarn %s %s 2", path, prop));
	if (sval1 && sval2) {
		ut_asserteq_str(sval1, env_get("svar1"));
		ut_asserteq_str(sval2, env_get("svar2"));
		ut_assertnull(env_get("svarn"));
	} else {
		ut_asserteq(ival1, env_get_hex("svar1", 0x1234));
		ut_asserteq(ival2, env_get_hex("svar2", 0x1234));
		ut_assertnull(env_get("svarn"));
	}
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_set_node(struct unit_test_state *uts,
			     const char *path, const char *prop)
{
	fdt_test_set_single(uts, path, prop, "new", 0, false);
	fdt_test_set_single(uts, path, prop, "rewrite", 0, false);
	fdt_test_set_single(uts, path, prop, NULL, 42, true);
	fdt_test_set_single(uts, path, prop, NULL, 0, false);
	fdt_test_set_multi(uts, path, prop, NULL, NULL, 42, 1701);
	fdt_test_set_multi(uts, path, prop, NULL, NULL, 74656, 9);
	fdt_test_set_multi(uts, path, prop, "42", "1701", 0, 0);
	fdt_test_set_multi(uts, path, prop, "74656", "9", 0, 0);

	return 0;
}

static int fdt_test_set(struct unit_test_state *uts)
{
	char fdt[8192];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	fdt_shrink_to_minimum(fdt, 4096);	/* Resize with 4096 extra bytes */
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test setting of root node / existing property "compatible" */
	fdt_test_set_node(uts, "/", "compatible");

	/* Test setting of root node / new property "newproperty" */
	fdt_test_set_node(uts, "/", "newproperty");

	/* Test setting of subnode existing property "compatible" */
	fdt_test_set_node(uts, "/test-node@1234/subnode", "compatible");
	fdt_test_set_node(uts, "subnodealias", "compatible");

	/* Test setting of subnode new property "newproperty" */
	fdt_test_set_node(uts, "/test-node@1234/subnode", "newproperty");
	fdt_test_set_node(uts, "subnodealias", "newproperty");

	/* Test setting property of non-existent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt set /no-node noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test setting property of non-existent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt set noalias noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	/* Test setting property of bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt set badalias noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_set, UT_TESTF_CONSOLE_REC);

static int fdt_test_mknode(struct unit_test_state *uts)
{
	char fdt[8192];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	fdt_shrink_to_minimum(fdt, 4096);	/* Resize with 4096 extra bytes */
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test creation of new node in / */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt mknode / newnode"));
	ut_assertok(run_commandf("fdt list /newnode"));
	ut_assert_nextline("newnode {");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in /test-node@1234 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt mknode /test-node@1234 newsubnode"));
	ut_assertok(run_commandf("fdt list /test-node@1234/newsubnode"));
	ut_assert_nextline("newsubnode {");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in /test-node@1234 by alias */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt mknode testnodealias newersubnode"));
	ut_assertok(run_commandf("fdt list testnodealias/newersubnode"));
	ut_assert_nextline("newersubnode {");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in /test-node@1234 over existing node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode testnodealias newsubnode"));
	ut_assert_nextline("libfdt fdt_add_subnode(): FDT_ERR_EXISTS");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in /test-node@1234 by alias over existing node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode testnodealias newersubnode"));
	ut_assert_nextline("libfdt fdt_add_subnode(): FDT_ERR_EXISTS");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in non-existent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode /no-node newnosubnode"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in non-existent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode noalias newfailsubnode"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode badalias newbadsubnode"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_mknode, UT_TESTF_CONSOLE_REC);

static int fdt_test_rm(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test removal of property in root node / */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt print / compatible"));
	ut_assert_nextline("compatible = \"u-boot,fdt-test\"");
	ut_assertok(run_commandf("fdt rm / compatible"));
	ut_asserteq(1, run_commandf("fdt print / compatible"));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of property clock-names in subnode /test-node@1234 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt print /test-node@1234 clock-names"));
	ut_assert_nextline("clock-names = \"fixed\", \"i2c\", \"spi\", \"uart2\", \"uart1\"");
	ut_assertok(run_commandf("fdt rm /test-node@1234 clock-names"));
	ut_asserteq(1, run_commandf("fdt print /test-node@1234 clock-names"));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of property u-boot,empty-property in subnode /test-node@1234 by alias */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt print testnodealias u-boot,empty-property"));
	ut_assert_nextline("testnodealias u-boot,empty-property");
	ut_assertok(run_commandf("fdt rm testnodealias u-boot,empty-property"));
	ut_asserteq(1, run_commandf("fdt print testnodealias u-boot,empty-property"));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of non-existent property noprop in subnode /test-node@1234 */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt rm /test-node@1234 noprop"));
	ut_assert_nextline("libfdt fdt_delprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of non-existent node /no-node@5678 */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt rm /no-node@5678"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of subnode /test-node@1234/subnode by alias */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rm subnodealias"));
	ut_asserteq(1, run_commandf("fdt print /test-node@1234/subnode"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of node by non-existent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt rm noalias"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of node by bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt rm noalias"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of node /test-node@1234 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rm /test-node@1234"));
	ut_asserteq(1, run_commandf("fdt print /test-node@1234"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of node / */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rm /"));
	ut_asserteq(1, run_commandf("fdt print /"));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_rm, UT_TESTF_CONSOLE_REC);

static int fdt_test_bootcpu(struct unit_test_state *uts)
{
	char fdt[256];
	ulong addr;
	int i;

	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test getting default bootcpu entry */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt header get bootcpu boot_cpuid_phys"));
	ut_asserteq(0, env_get_ulong("bootcpu", 10, 0x1234));
	ut_assertok(ut_check_console_end(uts));

	/* Test setting and getting new bootcpu entry, twice, to test overwrite */
	for (i = 42; i <= 43; i++) {
		ut_assertok(console_record_reset_enable());
		ut_assertok(run_commandf("fdt bootcpu %d", i));
		ut_assertok(ut_check_console_end(uts));

		/* Test getting new bootcpu entry */
		ut_assertok(console_record_reset_enable());
		ut_assertok(run_commandf("fdt header get bootcpu boot_cpuid_phys"));
		ut_asserteq(i, env_get_ulong("bootcpu", 10, 0x1234));
		ut_assertok(ut_check_console_end(uts));
	}

	return 0;
}
FDT_TEST(fdt_test_bootcpu, UT_TESTF_CONSOLE_REC);

int do_ut_fdt(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(fdt_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(fdt_test);

	return cmd_ut_category("fdt", "fdt_test_", tests, n_ents, argc, argv);
}
