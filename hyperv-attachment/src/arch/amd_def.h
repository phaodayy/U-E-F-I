#pragma once

#ifndef _INTELMACHINE

#include <cstddef>
#include <cstdint>
#include <ia32-doc/ia32.hpp>

#pragma warning(push)
#pragma warning(disable: 4201)

enum class tlb_control_t : std::uint8_t
{
	do_not_flush = 0,
	flush_entire_tlb = 1, // should only be used on legacy hardware
	flush_guest_tlb_entries = 3,
	flush_guest_non_global_tlb_entries = 7,
};

union vmcb_clean_t
{
	std::uint32_t flags;

	struct
	{
		std::uint32_t i : 1;
		std::uint32_t iopm : 1;
		std::uint32_t asid : 1;
		std::uint32_t tpr : 1;
		std::uint32_t nested_paging : 1;
		std::uint32_t cr : 1;
		std::uint32_t dr : 1;
		std::uint32_t dt : 1;
		std::uint32_t seg : 1;
		std::uint32_t cr2 : 1;
		std::uint32_t lbr : 1;
		std::uint32_t avic : 1;
		std::uint32_t cet : 1;
		std::uint32_t reserved : 19;
	};
};

struct vmcb_control_area_t
{
	std::uint16_t intercept_cr_read;
	std::uint16_t intercept_cr_write;
	std::uint16_t intercept_dr_read;
	std::uint16_t intercept_dr_write;
	std::uint32_t intercept_exception_vector;
	std::uint32_t intercept_misc_vector_3;
	std::uint32_t intercept_misc_vector_4;
	std::uint32_t intercept_misc_vector_5;
	std::uint8_t pad_two[0x28];
	std::uint64_t iopm_base_pa;
	std::uint64_t msrpm_base_pa;
	std::uint64_t tsc_offset;
	std::uint32_t guest_asid;
	tlb_control_t tlb_control;
	std::uint8_t pad_three[0x13];
	std::uint64_t vmexit_reason;
	std::uint64_t first_exit_info;
	std::uint64_t second_exit_info;
	std::uint8_t pad_four[0x28];
	cr3 nested_cr3;
	std::uint8_t pad_five[0x8];
	vmcb_clean_t clean;
	std::uint8_t pad_six[0x4];
	std::uint64_t next_rip;
	std::uint8_t number_of_bytes_fetched;
	std::uint8_t guest_instruction_bytes[15];
	std::uint8_t pad_seven[0x320];
};

struct vmcb_state_save_t
{
	std::uint8_t pad_one[0x148];
	std::uint64_t cr4;
    std::uint64_t cr3;
    std::uint8_t pad_five[0x20];
    std::uint64_t rip;
    std::uint8_t pad_six[0x58];
    std::uint64_t rsp;
    std::uint8_t pad_seven[0x18];
    std::uint64_t rax;
};

struct vmcb_t
{
	vmcb_control_area_t control;
	vmcb_state_save_t save_state;
};

static_assert(offsetof(vmcb_control_area_t, intercept_cr_read) == 0x0);
static_assert(offsetof(vmcb_control_area_t, intercept_exception_vector) == 0x8);
static_assert(offsetof(vmcb_control_area_t, intercept_misc_vector_3) == 0xC);
static_assert(offsetof(vmcb_control_area_t, iopm_base_pa) == 0x40);
static_assert(offsetof(vmcb_control_area_t, msrpm_base_pa) == 0x48);
static_assert(offsetof(vmcb_control_area_t, tsc_offset) == 0x50);
static_assert(offsetof(vmcb_control_area_t, vmexit_reason) == 0x70);
static_assert(offsetof(vmcb_control_area_t, next_rip) == 0xC8);
static_assert(offsetof(vmcb_control_area_t, number_of_bytes_fetched) == 0xD0);
static_assert(sizeof(vmcb_control_area_t) == 0x400);
static_assert(offsetof(vmcb_state_save_t, cr4) == 0x148);
static_assert(offsetof(vmcb_state_save_t, cr3) == 0x150);

union npf_exit_info_1
{
	std::uint64_t flags;

	struct
	{
		std::uint64_t present : 1;
		std::uint64_t write_access : 1;
		std::uint64_t user_access : 1;
		std::uint64_t npte_reserved_set : 1;
		std::uint64_t execute_access : 1;
		std::uint64_t reserved_one : 1;
		std::uint64_t shadow_stack_access : 1;
		std::uint64_t reserved_two : 25;
		std::uint64_t final_gpa_translation : 1;
		std::uint64_t gpt_translation : 1;
		std::uint64_t reserved_three : 3;
		std::uint64_t supervisor_shadow_stack : 1;
		std::uint64_t reserved_four : 26;
	};
};

#pragma warning(pop)

#define SVM_EXIT_REASON_PHYSICAL_NMI 0x61
#define SVM_EXIT_REASON_RDTSC 0x6E
#define SVM_EXIT_REASON_CPUID 0x72
#define SVM_EXIT_REASON_MSR 0x7C
#define SVM_EXIT_REASON_RDTSCP 0x87
#define SVM_EXIT_REASON_NPF 0x400

#define SVM_EXIT_REASON_READ_CR4 0x04
#define SVM_EXIT_REASON_WRITE_CR4 0x14

#define SVM_INTERCEPT_CR4 0x10

#define SVM_INTERCEPT_VECTOR3_RDTSC 0x00004000
#define SVM_INTERCEPT_VECTOR3_MSR_PROT 0x10000000

#define SVM_INTERCEPT_VECTOR4_RDTSCP 0x00000080

#define SVM_EXIT_INFO_MSR_READ 0

#endif
