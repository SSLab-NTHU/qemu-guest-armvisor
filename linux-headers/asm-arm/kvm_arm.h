/**
 *  kvm_arm.h
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *  Copyright (c) 2009~2012  SSLab, NTHU
 *
 */

#include <linux/kvm_types.h>
#include <linux/kvm_host.h>
#include <asm/ptrace.h>
extern const unsigned char exception_priority[];
extern const unsigned char priority_exception[];
extern int __kvmarm_vcpu_run(struct kvm_vcpu *vcpu);
enum emulation_result {
	EMULATE_DONE,         /* no further processing */
	EMULATE_DO_MMIO,      /* kvm_run filled with MMIO request */
	EMULATE_FAIL,         /* can't emulate this instruction */
};
extern void kvmarm_check_and_deliver_interrupts(struct kvm_vcpu *vcpu);
static inline void kvmarm_clear_exception(struct kvm_vcpu *vcpu, int exception)
{
	vcpu->arch.pending_exceptions = 0;
}
static inline void kvmarm_set_cpsr(struct kvm_vcpu *vcpu, u32 new_cpsr_mask)
{

	vcpu->arch.cpsr=(vcpu->arch.cpsr  & 0xffffff40 )|new_cpsr_mask;

	if (vcpu->arch.cpsr & IRQ_MODE)
		kvm_vcpu_block(vcpu);
}
extern void write_banked_regs(struct kvm_vcpu *vcpu,u32 index,u32 value);

extern int kvmarm_emulate_instruction(struct kvm_vcpu *vcpu);

static inline void kvmarm_queue_exception(struct kvm_vcpu *vcpu,unsigned int exception)
{
	vcpu->arch.pending_exceptions = exception;
}
