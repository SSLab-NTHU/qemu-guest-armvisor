/*
 * ARM implementation of KVM hooks
 *
 * Copyright NTHU SSLab 2011-2012
 *
 * Authors:
 *  Chuck Lin <chucklin@sslab.cs.nthu.edu.tw>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/kvm.h>

#include "qemu-common.h"
#include "qemu-timer.h"
#include "sysemu.h"
#include "kvm.h"
#include "cpu.h"
#include "device_tree.h"

//#define DEBUG_KVM

#ifdef DEBUG_KVM
#define dprintf(fmt, ...) \
    do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#else
#define dprintf(fmt, ...) \
    do { } while (0)
#endif

const KVMCapabilityInfo kvm_arch_required_capabilities[] = {
    KVM_CAP_LAST_INFO
};

/* arch arm only */
static int kvmarm_try_push_interrupts(CPUState *env, struct kvm_run *run)
{
    int r;
    //gic_state *s = env->realviewgic_state.gic;
    unsigned int irq;
     
    if (run->ready_for_interrupt_injection &&
       (env->interrupt_request & CPU_INTERRUPT_FIQ) )
    {
            
            irq = CPU_INTERRUPT_FIQ;
            r = kvm_vcpu_ioctl(env, KVM_INTERRUPT, &irq);
            if (r < 0)
                   printf("cpu %d fail inject %x\n", env->cpu_index, irq);
            env->exit_request = 1;
    }
 
    //else if (kvm_is_ready_for_interrupt_injection(env->kvm_cpu_state.vcpu_ctx) &&
     else if( (env->interrupt_request & CPU_INTERRUPT_HARD))  
    {
	    irq = CPU_INTERRUPT_HARD;
            r = kvm_vcpu_ioctl(env, KVM_INTERRUPT, &irq);
	    if (r < 0)
		    printf("cpu %d fail inject %x\n", env->cpu_index, irq);
            env->exit_request = 1;
    }

    return 0;
}

/* arch */

int kvm_arch_init(KVMState *s)
{
    return 0;
}

int kvm_arch_init_vcpu(CPUState *cenv)
{
    return 0;
}

void kvm_arch_reset_vcpu(CPUState *env)
{
}

int kvm_arch_put_registers(CPUState *env, int level)
{
    struct kvm_regs regs;
    int ret;
    int i;

    ret = kvm_vcpu_ioctl(env, KVM_GET_REGS, &regs);
    if (ret < 0)
        return ret;

    regs.cpsr= env->uncached_cpsr;
    regs.spsr = env->spsr;
    /*****synchronize CP15*******/
    regs.cp15.c0_cpuid = env->cp15.c0_cpuid;                                             
    regs.cp15.c0_cachetype = env->cp15.c0_cachetype;
    for(i=0;i<16;i++)
        regs.cp15.c0_ccsid[i] = env->cp15.c0_ccsid[i];
    regs.cp15.c0_clid = env->cp15.c0_clid;
    regs.cp15.c0_cssel = env->cp15.c0_cssel;
    for(i=0;i<8;i++)
        regs.cp15.c0_c1[i] = env->cp15.c0_c1[i];
    for(i=0;i<8;i++)
        regs.cp15.c0_c2[i] = env->cp15.c0_c2[i];
    regs.cp15.c1_sys = env->cp15.c1_sys;
    regs.cp15.c1_coproc = env->cp15.c1_coproc;
    regs.cp15.c1_xscaleauxcr = env->cp15.c1_xscaleauxcr;
    regs.cp15.c2_base0 = env->cp15.c2_base0;
    regs.cp15.c2_base1 = env->cp15.c2_base1;
    regs.cp15.c2_control = env->cp15.c2_control;
    regs.cp15.c2_mask = env->cp15.c2_mask;
    regs.cp15.c2_base_mask = env->cp15.c2_base_mask;
    regs.cp15.c2_data = env->cp15.c2_data;
    regs.cp15.c2_insn = env->cp15.c2_insn;
    regs.cp15.c3 = env->cp15.c3;
    regs.cp15.c5_insn = env->cp15.c5_insn;
    regs.cp15.c5_data = env->cp15.c5_data;
    for(i=0;i<8;i++)
        regs.cp15.c6_region[i] = env->cp15.c6_region[i];
    regs.cp15.c6_insn = env->cp15.c6_insn;
    regs.cp15.c6_data = env->cp15.c6_data;
    regs.cp15.c9_insn = env->cp15.c9_insn;
    regs.cp15.c9_data = env->cp15.c9_data;
    regs.cp15.c13_fcse = env->cp15.c13_fcse;
    regs.cp15.c13_context = env->cp15.c13_context;
    regs.cp15.c13_tls1 = env->cp15.c13_tls1;
    regs.cp15.c13_tls2 = env->cp15.c13_tls2;
    regs.cp15.c13_tls3 = env->cp15.c13_tls3;
    regs.cp15.c15_cpar = env->cp15.c15_cpar;
    regs.cp15.c15_ticonfig = env->cp15.c15_ticonfig;
    regs.cp15.c15_i_max = env->cp15.c15_i_max;
    regs.cp15.c15_i_min = env->cp15.c15_i_min;
    regs.cp15.c15_threadid = env->cp15.c15_threadid;

    for (i = 0;i < 16; i++){
        regs.gpr[i] = env->regs[i];
    }

    ret = kvm_vcpu_ioctl(env, KVM_SET_REGS, &regs);
    if (ret < 0)
        return ret;

    return ret;
}

int kvm_arch_get_registers(CPUState *env)
{
    struct kvm_regs regs;
    int i, ret;

    ret = kvm_vcpu_ioctl(env, KVM_GET_REGS, &regs);
    if (ret < 0)
        return ret;

    env->uncached_cpsr =regs.cpsr;
    env->spsr = regs.spsr;
    /*****synchronize CP15*******/
    env->cp15.c0_cpuid = regs.cp15.c0_cpuid;
    env->cp15.c0_cachetype = regs.cp15.c0_cachetype;
    for(i=0;i<16;i++)
        env->cp15.c0_ccsid[i] = regs.cp15.c0_ccsid[i];
    env->cp15.c0_clid = regs.cp15.c0_clid;
    env->cp15.c0_cssel = regs.cp15.c0_cssel;
    for(i=0;i<8;i++)
        env->cp15.c0_c1[i] = regs.cp15.c0_c1[i];
    for(i=0;i<8;i++)
        env->cp15.c0_c2[i] = regs.cp15.c0_c2[i];
    env->cp15.c1_sys = regs.cp15.c1_sys;
    env->cp15.c1_coproc = regs.cp15.c1_coproc;
    env->cp15.c1_xscaleauxcr = regs.cp15.c1_xscaleauxcr;
    env->cp15.c2_base0 = regs.cp15.c2_base0;
    env->cp15.c2_base1 = regs.cp15.c2_base1;
    env->cp15.c2_control = regs.cp15.c2_control;
    env->cp15.c2_mask = regs.cp15.c2_mask;
    env->cp15.c2_base_mask = regs.cp15.c2_base_mask;
    env->cp15.c2_data = regs.cp15.c2_data;
    env->cp15.c2_insn = regs.cp15.c2_insn;
    env->cp15.c3 = regs.cp15.c3;
    env->cp15.c5_insn = regs.cp15.c5_insn;
    env->cp15.c5_data = regs.cp15.c5_data;
    for(i=0;i<8;i++)
        env->cp15.c6_region[i] = regs.cp15.c6_region[i];
    env->cp15.c6_insn = regs.cp15.c6_insn;
    env->cp15.c6_data = regs.cp15.c6_data;
    env->cp15.c9_insn = regs.cp15.c9_insn;
    env->cp15.c9_data = regs.cp15.c9_data;
    env->cp15.c13_fcse = regs.cp15.c13_fcse;
    env->cp15.c13_context = regs.cp15.c13_context;
    env->cp15.c13_tls1 = regs.cp15.c13_tls1;
    env->cp15.c13_tls2 = regs.cp15.c13_tls2;
    env->cp15.c13_tls3 = regs.cp15.c13_tls3;
    env->cp15.c15_cpar = regs.cp15.c15_cpar;
    env->cp15.c15_ticonfig = regs.cp15.c15_ticonfig;
    env->cp15.c15_i_max = regs.cp15.c15_i_max;
    env->cp15.c15_i_min = regs.cp15.c15_i_min;
    env->cp15.c15_threadid = regs.cp15.c15_threadid;
    /*******************************************************************/
   
    for (i = 0;i < 16; i++){
        env->regs[i] = regs.gpr[i];
    }

    return 0;
}

void kvm_arch_pre_run(CPUState *env, struct kvm_run *run)
{
    kvmarm_try_push_interrupts(env, run);
}

void kvm_arch_post_run(CPUState *env, struct kvm_run *run)
{
}

int kvm_arch_process_async_events(CPUState *env) 
{
    return 0;
}

static int kvmarm_handle_halt(CPUState *env)
{
    return 0;
}

int kvm_arch_handle_exit(CPUState *env, struct kvm_run *run)
{
    int ret = 0;

    switch (run->exit_reason) {
    case KVM_EXIT_HLT:
        dprintf("handle halt\n");
        ret = kvmarm_handle_halt(env);
        break;
    case KVM_EXIT_NEED_INJECT:
        ret = 0;
        break;
    default:
        fprintf(stderr, "KVM: unknown exit reason %d\n", run->exit_reason);
        ret = -1;
        break;
    }

    return ret;
}

bool kvm_arch_stop_on_emulation_error(CPUState *env)
{
    return true;
}

int kvm_arch_on_sigbus_vcpu(CPUState *env, int code, void *addr)
{
    return 1;
}

int kvm_arch_on_sigbus(int code, void *addr)
{
    return 1;
}   
