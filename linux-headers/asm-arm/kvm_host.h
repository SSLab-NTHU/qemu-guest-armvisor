/*
 * Kernel-based Virtual Machine driver for Linux
 *
 * Copyright (c) 2009-2012 SSLab, NTHU
 *
 * This header defines architecture specific interfaces, arm version
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */
#ifndef __ARM_KVM_HOST_H__
#define __ARM_KVM_HOST_H__

#define KVM_MAX_VCPUS 1
#define KVM_MEMORY_SLOTS 32
	/* memory slots that does not exposed to userspace */
#define KVM_PRIVATE_MEM_SLOTS 4

#define KVM_COALESCED_MMIO_PAGE_OFFSET 1

/* We don't currently support large pages. */
#define KVM_PAGES_PER_HPAGE (1<<31)
#include <asm/kvm_asm.h>

#define INVALID_PAGE (~(hpa_t)0)

#define KVM_ALIAS_SLOTS 4
#define KVM_PERMILLE_MMU_PAGES 20
#define KVM_MIN_ALLOC_MMU_PAGES 64
#define KVM_MMU_HASH_PT_SHIFT 10
#define KVM_MMU_HASH_PD_SHIFT 12
#define KVM_NUM_PT_MMU_PAGES (1 << KVM_MMU_HASH_PT_SHIFT) //NUM_OF_PT_PAGES
#define KVM_NUM_PD_MMU_PAGES (1 << KVM_MMU_HASH_PD_SHIFT) //NUM_OF_PD_PAGES

#define KVM_NR_MEM_OBJS 40

#define EXIT_GUEST_EXEPTION 5
//struct kvm_guest_debug_arch{
//struct kvm_guest_debug {
//    int enabled;
//    unsigned long bp[4];
//    int singlestep;
//};
//struct kvm_debug_exit_arch{
//  
//};

extern spinlock_t kvm_lock;
extern struct list_head vm_list;


struct kvm_mmu_memory_cache {
	int nobjs;
	void *objects[KVM_NR_MEM_OBJS];
};

struct kvm_vm_stat {

   // u32 mmu_shadow_zapped;
   // u32 mmu_pte_write;
   // u32 mmu_pte_updated;
   // u32 mmu_pde_zapped;
   // u32 mmu_flooded;
   // u32 mmu_recycled;
   // u32 mmu_cache_miss;
   // u32 mmu_unsync;
    u32 remote_tlb_flush;
   // u32 lpages;

};
struct kvm_mem_alias {
        gfn_t base_gfn;
        unsigned long npages;
        gfn_t target_gfn;
};
struct kvm_arch{
		int naliases;
    struct kvm_mem_alias aliases[KVM_ALIAS_SLOTS];

    unsigned int n_free_mmu_pages;
    unsigned int n_requested_mmu_pages;
    unsigned int n_alloc_mmu_pages;
    struct hlist_head mmu_pt_page_hash[KVM_NUM_PT_MMU_PAGES]; //pt hash
    struct hlist_head mmu_pd_page_hash[KVM_NUM_PD_MMU_PAGES]; //pd hash

    /* Hash table of struct kvm_mmu_page.
    */
    struct list_head active_mmu_pages;
    struct list_head assigned_dev_head;
};
struct kvm_vcpu_stat { 
    u32 sum_exits;
    u32 mmio_exits;
    u32 dcr_exits;
    u32 signal_exits;
    u32 light_exits;
    /* Account for special types of light exits: */
    u32 itlb_real_miss_exits;
    u32 itlb_virt_miss_exits;
    u32 dtlb_real_miss_exits;
    u32 dtlb_virt_miss_exits;
    u32 syscall_exits;
    u32 isi_exits;
    u32 dsi_exits;
    u32 emulated_inst_exits;
    u32 dec_exits;
    u32 ext_intr_exits;
    u32 halt_wakeup;
};

#define NR_PTE_CHAIN_ENTRIES 5

struct kvm_pte_chain {
	u64 *parent_ptes[NR_PTE_CHAIN_ENTRIES];
	struct hlist_node link;
};

union kvm_mmu_page_role {
	unsigned word;
	struct {
		unsigned glevels:4;
		unsigned level:4;
		unsigned quadrant:2;
		unsigned pad_for_nice_hex_output:6;
		unsigned direct:1;
		unsigned access:3;
		unsigned invalid:1;
		//unsigned cr4_pge:1;
		unsigned nxe:1;
		unsigned metaphysical : 1;
		unsigned hugepage_access : 3;	
	};
};

struct kvm_mmu_page {
	struct list_head link;
	struct hlist_node hash_link;

	struct list_head oos_link;

	/*
	 * The following two entries are used to key the shadow page in the
	 * hash table.
	 */
	gfn_t gfn;
	union kvm_mmu_page_role role;

	u32 *spt;
	/* hold the gfn of each spte inside spt */
	gfn_t *gfns; //unused in kvm36
	/*
	 * One bit set per slot which has memory
	 * in this shadow page.
	 */
	DECLARE_BITMAP(slot_bitmap, KVM_MEMORY_SLOTS + KVM_PRIVATE_MEM_SLOTS);
	int multimapped;         /* More than one parent_pte? */
	int root_count;          /* Currently serving as active root */
	bool unsync;
	unsigned int unsync_children;
	union {
		u64 *parent_pte;               /* !multimapped */
		struct hlist_head parent_ptes; /* multimapped, kvm_pte_chain */
	};
	DECLARE_BITMAP(unsync_child_bitmap, 512);
};

struct kvm_mmu {
	void (*new_ttbr)(struct kvm_vcpu *vcpu);
	int (*page_fault)(struct kvm_vcpu *vcpu, gva_t gva, u32 err);
	void (*free)(struct kvm_vcpu *vcpu);
	gpa_t (*gva_to_gpa)(struct kvm_vcpu *vcpu, gva_t gva);
	void (*prefetch_page)(struct kvm_vcpu *vcpu,
			      struct kvm_mmu_page *page);
	int (*sync_page)(struct kvm_vcpu *vcpu,
			 struct kvm_mmu_page *sp);
	void (*invlpg)(struct kvm_vcpu *vcpu, gva_t gva);
	hpa_t root_hpa;
	int root_level;
	int shadow_root_level;
	union kvm_mmu_page_role base_role;

	u64 *pae_root;
	u64 rsvd_bits_mask[2][4];
};

struct kvm_vcpu_arch{
	uint32_t regs[16];
	uint32_t shadow_pgd_addr;
	/*vcpu host register*/
	uint32_t host_regs[16];
	uint32_t host_spsr;
	uint32_t host_cpsr;
	uint32_t host_cr;
	uint32_t host_c5_data;
	uint32_t host_c5_insn;
	uint32_t host_c6_data;
  uint32_t host_c6_insn;
	uint32_t host_pgd_addr;
	/* Frequently accessed CPSR bits are stored separately for efficiently.
	   This contains all the other bits.  Use cpsr_{read,write} to access
	   the whole CPSR.  */
	uint32_t cpsr;
	uint32_t spsr;
	uint32_t last_inst;
	/* Banked registers.  */
	uint32_t banked_spsr[6];
	uint32_t banked_r13[6];
	uint32_t banked_r14[6];
	/* These hold r8-r12.  */
	uint32_t usr_regs[5];
	uint32_t fiq_regs[5];
	/* cpsr flag cache for faster execution */
	uint32_t CF; /* 0 or 1 */
	uint32_t VF; /* V is the bit 31. All other bits are undefined */
	uint32_t NF; /* N is bit 31. All other bits are undefined.  */
	uint32_t ZF; /* Z set if zero.  */
	uint32_t QF; /* 0 or 1 */
	uint32_t GE; /* cpsr[19:16] */
	uint32_t thumb; /* cpsr[5]. 0 = arm mode, 1 = thumb mode. */
	uint32_t condexec_bits; /* IT bits.  cpsr[15:10,26:25].  */

	/* System control coprocessor (cp15) */
	struct {
		uint32_t c0_cpuid;
		uint32_t c0_cachetype;
		uint32_t c0_ccsid[16]; /* Cache size.  */
		uint32_t c0_clid; /* Cache level.  */
		uint32_t c0_cssel; /* Cache size selection.  */
		uint32_t c0_c1[8]; /* Feature registers.  */
		uint32_t c0_c2[8]; /* Instruction set registers.  */
		uint32_t c1_sys; /* System control register.  */
		uint32_t c1_coproc; /* Coprocessor access register.  */
		uint32_t c1_xscaleauxcr; /* XScale auxiliary control register.  */
		uint32_t c2_base0; /* MMU translation table base 0.  */
		uint32_t c2_base1; /* MMU translation table base 1.  */
		uint32_t c2_control; /* MMU translation table base control.  */
		uint32_t c2_mask; /* MMU translation table base selection mask.  */
		uint32_t c2_base_mask; /* MMU translation table base 0 mask. */
		uint32_t c2_data; /* MPU data cachable bits.  */
		uint32_t c2_insn; /* MPU instruction cachable bits.  */
		uint32_t c3; /* MMU domain access control register
				MPU write buffer control.  */
		uint32_t c5_insn; /* Fault status registers.  */
		uint32_t c5_data;
		uint32_t c6_region[8]; /* MPU base/size registers.  */
		uint32_t c6_insn; /* Fault address registers.  */
		uint32_t c6_data;
		uint32_t c9_insn; /* Cache lockdown registers.  */
		uint32_t c9_data;
		uint32_t c13_fcse; /* FCSE PID.  */
		uint32_t c13_context; /* Context ID.  */
		uint32_t c13_tls1; /* User RW Thread register.  */
		uint32_t c13_tls2; /* User RO Thread register.  */
		uint32_t c13_tls3; /* Privileged Thread register.  */
		uint32_t c15_cpar; /* XScale Coprocessor Access Register */
		uint32_t c15_ticonfig; /* TI925T configuration byte.  */
		uint32_t c15_i_max; /* Maximum D-cache dirty line index.  */
		uint32_t c15_i_min; /* Minimum D-cache dirty line index.  */
		uint32_t c15_threadid; /* TI debugger thread-ID.  */
	} cp15;

	uint32_t pending_exceptions;
	/* Coprocessor IO used by peripherals */
	/*struct {
	  ARMReadCPFunc *cp_read;
	  ARMWriteCPFunc *cp_write;
	  void *opaque;
	  } cp[15];
	 */
	/* Thumb-2 EE state.  */
	uint32_t teecr;
	uint32_t teehbr;

	/* Internal CPU feature flags.  */
	uint32_t features;

	/* Callback for vectored interrupt controller.  */
	//int (*get_irq_vector)(struct CPUARMState *);
	void *irq_opaque;

	/* VFP coprocessor state.  */
	struct {
		double regs[32];

		uint32_t xregs[16];
		/* We store these fpcsr fields separately for convenience.  */
		int vec_len;
		int vec_stride;

		/* scratch space when Tn are not sufficient.  */
		uint32_t scratch[8];

		//float_status fp_status;
	} vfp;
#if defined(CONFIG_USER_ONLY)
	struct mmon_state *mmon_entry;
#else
	uint32_t mmon_addr;
#endif

	/* iwMMXt coprocessor state.  */
	struct {
		uint64_t regs[16];
		uint64_t val;

		uint32_t cregs[16];
	} iwmmxt;

#if defined(CONFIG_USER_ONLY)
	/* For usermode syscall translation.  */
	int eabi;
#endif

	//CPU_COMMON //alei edit important structure

	/* These fields after the common ones so they are preserved on reset.  */
	struct arm_boot_info *boot_info;
	
	struct kvm_mmu mmu;
	
	struct kvm_mmu_memory_cache mmu_pte_chain_cache;
  struct kvm_mmu_memory_cache mmu_rmap_desc_cache;
  struct kvm_mmu_memory_cache mmu_page_cache;
  struct kvm_mmu_memory_cache mmu_page_header_cache;
  struct kvm_mmu_memory_cache mmu_page_dir_cache;		//page cache for page dir
  
  u32 swi_num;
  gpa_t paddr_accessed;	
  int is_mmio_inst;
  u32 mls_regs;
  u8  is_mls;
  u8  load_style;
  u8  store_style;
  u8  signed_flag;
  u32 write_addr;
  u32 write_val; 
  struct {
		gfn_t gfn;	/* presumed gfn during guest pte update */
		pfn_t pfn;	/* pfn corresponding to that gfn */
		int largepage;
		unsigned long mmu_seq;
	}update_pte;	
	
	u8 io_gpr;
};
//unsigned int *kvm_vcpu_pointer;
#endif /* __ARM_KVM_HOST_H__ */
