/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * Data structures and headers for ext4 support have been taken from
 * ext2 ls load support in Uboot
 *
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * based on code from grub2 fs/ext2.c and fs/fshelp.c by
 * GRUB  --  GRand Unified Bootloader
 * Copyright (C) 2003, 2004  Free Software Foundation, Inc.
 */

#ifndef __EXT_COMMON__
#define __EXT_COMMON__

#include <compiler.h>

struct cmd_tbl;

#define SECTOR_SIZE		0x200
#define LOG2_SECTOR_SIZE	9

/* Magic value used to identify an ext2 filesystem.  */
#define	EXT2_MAGIC			0xEF53
/* Amount of indirect blocks in an inode.  */
#define INDIRECT_BLOCKS			12
/* Maximum lenght of a pathname.  */
#define EXT2_PATH_MAX				4096
/* Maximum nesting of symlinks, used to prevent a loop.  */
#define	EXT2_MAX_SYMLINKCNT		8

/* Filetype used in directory entry.  */
#define	FILETYPE_UNKNOWN		0
#define	FILETYPE_REG			1
#define	FILETYPE_DIRECTORY		2
#define	FILETYPE_SYMLINK		7

/* Filetype information as used in inodes.  */
#define FILETYPE_INO_MASK		0170000
#define FILETYPE_INO_REG		0100000
#define FILETYPE_INO_DIRECTORY		0040000
#define FILETYPE_INO_SYMLINK		0120000
#define EXT2_ROOT_INO			2 /* Root inode */

/* The size of an ext2 block in bytes.  */
#define EXT2_BLOCK_SIZE(data)	   (1 << LOG2_BLOCK_SIZE(data))

/* Log2 size of ext2 block in bytes.  */
#define LOG2_BLOCK_SIZE(data)	   (le32_to_cpu		   \
				    (data->sblock.log2_block_size) \
				    + EXT2_MIN_BLOCK_LOG_SIZE)

#define EXT2_FT_DIR	2
#define SUCCESS	1

/* Macro-instructions used to manage several block sizes  */
#define EXT2_MIN_BLOCK_LOG_SIZE	10 /* 1024 */
#define EXT2_MAX_BLOCK_LOG_SIZE	16 /* 65536 */
#define EXT2_MIN_BLOCK_SIZE		(1 << EXT2_MIN_BLOCK_LOG_SIZE)
#define EXT2_MAX_BLOCK_SIZE		(1 << EXT2_MAX_BLOCK_LOG_SIZE)

/* The ext2 superblock.  */
struct ext2_sblock {
	__le32 total_inodes;
	__le32 total_blocks;
	__le32 reserved_blocks;
	__le32 free_blocks;
	__le32 free_inodes;
	__le32 first_data_block;
	__le32 log2_block_size;
	__le32 log2_fragment_size;
	__le32 blocks_per_group;
	__le32 fragments_per_group;
	__le32 inodes_per_group;
	__le32 mtime;
	__le32 utime;
	__le16 mnt_count;
	__le16 max_mnt_count;
	__le16 magic;
	__le16 fs_state;
	__le16 error_handling;
	__le16 minor_revision_level;
	__le32 lastcheck;
	__le32 checkinterval;
	__le32 creator_os;
	__le32 revision_level;
	__le16 uid_reserved;
	__le16 gid_reserved;
	__le32 first_inode;
	__le16 inode_size;
	__le16 block_group_number;
	__le32 feature_compatibility;
	__le32 feature_incompat;
	__le32 feature_ro_compat;
	uint8_t unique_id[16];
	char volume_name[16];
	char last_mounted_on[64];
	__le32 compression_info;
	uint8_t prealloc_blocks;
	uint8_t prealloc_dir_blocks;
	__le16 reserved_gdt_blocks;
	uint8_t journal_uuid[16];
	__le32 journal_inode;
	__le32 journal_dev;
	__le32 last_orphan;
	__le32 hash_seed[4];
	uint8_t default_hash_version;
	uint8_t journal_backup_type;
	__le16 descriptor_size;
	__le32 default_mount_options;
	__le32 first_meta_block_group;
	__le32 mkfs_time;
	__le32 journal_blocks[17];

	/* 64 bit support */
	__le32 total_blocks_high;
	__le32 reserved_blocks_high;
	__le32 free_blocks_high;
	__le16 min_extra_inode_size;
	__le16 want_extra_inode_size;
	__le32 flags;
	__le16 raid_stride;
	__le16 mmp_interval;
	__le64 mmp_block;
	__le32 raid_stripe_width;
	uint8_t log2_groups_per_flex;
	uint8_t checksum_type;
	uint8_t encryption_level;
	uint8_t reserved_pad;
	__le64 kbytes_written;
	__le32 snapshot_inum;
	__le32 snapshot_id;
	__le64 snapshot_r_blocks_count;
	__le32 snapshot_list;
	__le32 error_count;
	__le32 first_error_time;
	__le32 first_error_ino;
	__le64 first_error_block;
	uint8_t first_error_func[32];
	__le32 first_error_line;
	__le32 last_error_time;
	__le32 last_error_ino;
	__le32 last_error_line;
	__le64 last_error_block;
	uint8_t last_error_func[32];
	uint8_t mount_opts[64];
	__le32 usr_quota_inum;
	__le32 grp_quota_inum;
	__le32 overhead_clusters;
	__le32 backup_bgs[2];
	uint8_t encrypt_algos[4];
	uint8_t encrypt_pw_salt[16];
	__le32 lpf_ino;
	__le32 prj_quota_inum;
	__le32 checksum_seed;
	uint8_t wtime_hi;
	uint8_t mtime_hi;
	uint8_t mkfs_time_hi;
	uint8_t lastcheck_hi;
	uint8_t first_error_time_hi;
	uint8_t last_error_time_hi;
	uint8_t pad[2];
	__le32 reserved[96];
	__le32 checksum;
};

struct ext2_block_group {
	__le32 block_id;	/* Blocks bitmap block */
	__le32 inode_id;	/* Inodes bitmap block */
	__le32 inode_table_id;	/* Inodes table block */
	__le16 free_blocks;	/* Free blocks count */
	__le16 free_inodes;	/* Free inodes count */
	__le16 used_dir_cnt;	/* Directories count */
	__le16 bg_flags;
	__le32 bg_exclude_bitmap;
	__le16 bg_block_id_csum;
	__le16 bg_inode_id_csum;
	__le16 bg_itable_unused; /* Unused inodes count */
	__le16 bg_checksum;	/* crc16(s_uuid+group_num+group_desc)*/
	/* following fields only exist if descriptor size is 64 */
	__le32 block_id_high;
	__le32 inode_id_high;
	__le32 inode_table_id_high;
	__le16 free_blocks_high;
	__le16 free_inodes_high;
	__le16 used_dir_cnt_high;
	__le16 bg_itable_unused_high;
	__le32 bg_exclude_bitmap_high;
	__le16 bg_block_id_csum_high;
	__le16 bg_inode_id_csum_high;
	__le32 bg_reserved;
};

#define EXT4_BG_INODE_BITMAP_CSUM_HI_END	\
	(offsetof(struct ext2_block_group, bg_inode_id_csum_high) + \
	 sizeof(__le16))
#define EXT4_BG_BLOCK_BITMAP_CSUM_HI_END	\
	(offsetof(struct ext2_block_group, bg_block_id_csum_high) + \
	 sizeof(__le16))

/* The ext2 inode. */
struct ext2_inode {
	__le16 mode;
	__le16 uid;
	__le32 size;
	__le32 atime;
	__le32 ctime;
	__le32 mtime;
	__le32 dtime;
	__le16 gid;
	__le16 nlinks;
	__le32 blockcnt;	/* Blocks of either 512 or block_size bytes */
	__le32 flags;
	__le32 osd1;
	union {
		struct datablocks {
			__le32 dir_blocks[INDIRECT_BLOCKS];
			__le32 indir_block;
			__le32 double_indir_block;
			__le32 triple_indir_block;
		} blocks;
		char symlink[60];
		char inline_data[60];
	} b;
	__le32 generation;
	__le32 acl;
	__le32 size_high;	/* previously dir_acl, but never used */
	__le32 fragment_addr;

	__le16 blocks_high; /* were l_i_reserved1 */
	__le16 file_acl_high;
	__le16 uid_high;
	__le16 gid_high;
	__le16 checksum_lo; /* crc32c(uuid+inum+inode) LE */
	__le16 reserved;

	/* optional part */
	__le16 extra_isize;
	__le16 checksum_hi; /* crc32c(uuid+inum+inode) BE */
	__le32 ctime_extra;
	__le32 mtime_extra;
	__le32 atime_extra;
	__le32 crtime;
	__le32 crtime_extra;
	__le32 version_hi;
	__le32 projid;
};

/* The header of an ext2 directory entry. */
struct ext2_dirent {
	__le32 inode;
	__le16 direntlen;
	__u8 namelen;
	__u8 filetype;
};

struct ext4_dir_entry_tail {
	__le32	det_reserved_zero1;	/* Pretend to be unused */
	__le16	det_rec_len;		/* 12 */
	__u8	det_reserved_zero2;	/* Zero name length */
	__u8	det_reserved_ft;	/* 0xDE, fake file type */
	__le32	det_checksum;		/* crc32c(uuid+inum+dirblock) */
};

struct ext2fs_node {
	struct ext2_data *data;
	struct ext2_inode inode;
	int ino;
	int inode_read;
};

/* Information about a "mounted" ext2 filesystem. */
struct ext2_data {
	struct ext2_sblock sblock;
	struct ext2_inode *inode;
	struct ext2fs_node diropen;
};

extern lbaint_t part_offset;

int do_ext2ls(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ext2load(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ext4_load(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[]);
int do_ext4_ls(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ext4_write(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[]);
#endif
