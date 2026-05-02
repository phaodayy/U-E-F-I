#pragma once
#include <Windows.h>
#include <winternl.h>
#include <cstdint>
#include <mutex>
#include "../../.shared/shared.hpp"

// Forward declaration if NTSTATUS is missing
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((long)(Status)) >= 0)
#endif

namespace Driver {
    typedef long(NTAPI* NtQueryAuxFunc_t)(uint64_t*, c_packet*);
    inline NtQueryAuxFunc_t g_NtQueryAux = nullptr;
    inline shared_communication* g_SharedComm = nullptr;
    inline std::mutex g_SyscallMutex;

    inline long issue_syscall(c_packet* packet) {
        std::lock_guard<std::mutex> lock(g_SyscallMutex);
        
        if (!g_SharedComm) {
            auto* ntdll = GetModuleHandleA("ntdll.dll");
            if (!ntdll) return 0xC0000135L; // STATUS_DLL_NOT_FOUND
            
            g_NtQueryAux = (NtQueryAuxFunc_t)GetProcAddress(ntdll, "NtQueryAuxiliaryCounterFrequency");
            if (!g_NtQueryAux) return 0xC0000139L; // STATUS_ENTRYPOINT_NOT_FOUND
            
            setup_shared_memory_packet p = {0};
            p.overlay_pid = GetCurrentProcessId();
            
            c_packet setup_pkt(e_syscall::setup_shared_memory, &p, sizeof(p));
            uint64_t x = 0;
            long st = g_NtQueryAux(&x, &setup_pkt);
            
            if (st >= 0 && p.user_ptr != nullptr) {
                g_SharedComm = (shared_communication*)p.user_ptr;
                g_SharedComm->magic = 0x5E4C7A02;
                g_SharedComm->request = 0;
            } else {
                return 0xC0000001L; // STATUS_UNSUCCESSFUL
            }
        }
        
        while (g_SharedComm->request == 1) { 
            Sleep(0); 
        }

        if (packet->get<void>()) {
            memcpy((void*)g_SharedComm->buffer, packet->get<void>(), (size_t)packet->get_size());
        }

        g_SharedComm->packet = c_packet(packet->get_syscall(), (void*)g_SharedComm->buffer, packet->get_size());
        g_SharedComm->request = 1;

        int spin = 0;
        while (g_SharedComm->request == 1) {
            if (spin < 5000) {
                YieldProcessor();
                spin++;
            } else {
                Sleep(1);
                spin = 0; // Prevent overflow but keep checking
            }
        }

        if (packet->get<void>()) {
            memcpy(packet->get<void>(), (void*)g_SharedComm->buffer, (size_t)packet->get_size());
        }

        return (long)g_SharedComm->status;
    }

    inline bool ReadMemory(uint32_t pid, uint64_t src, void* dest, uint64_t size) {
        if (!pid) return false;
        copy_process_memory_packet input = { pid, src, dest, size };
        auto packet = c_packet(e_syscall::read_process_memory, &input, sizeof(input));
        return issue_syscall(&packet) >= 0;
    }

    inline bool QueryProcess(uint32_t pid, query_process_data_packet* output) {
        output->process_id = pid;
        auto packet = c_packet(e_syscall::query_process_data, output, sizeof(query_process_data_packet));
        return issue_syscall(&packet) >= 0;
    }

    template <typename T>
    inline T Read(uint32_t pid, uint64_t address) {
        T buffer = {};
        ReadMemory(pid, address, &buffer, sizeof(T));
        return buffer;
    }

    inline bool MoveMouse(long x, long y, unsigned short flags = 0) {
        mouse_move_packet input = { x, y, flags };
        c_packet packet(e_syscall::mouse_move, &input, sizeof(input));
        return issue_syscall(&packet) >= 0;
    }

    inline bool Click() {
        MoveMouse(0, 0, 0x0001); // MOUSE_LEFT_BUTTON_DOWN
        Sleep(5);
        MoveMouse(0, 0, 0x0002); // MOUSE_LEFT_BUTTON_UP
        return true;
    }
}
