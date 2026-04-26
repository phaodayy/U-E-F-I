#pragma once

// Syscall operation types
enum class e_syscall : unsigned int {
  null = 0,
  read_process_memory,
  write_process_memory,
  query_process_data,
  hide_process_memory,
  bulk_read_process_memory,
  decrypt_pointer,
  initialize_decryption_shellcode,
  setup_shared_memory,
  mouse_move
};

struct init_decrypt_shellcode_packet {
  unsigned char shellcode[1024];
  unsigned long long key_ptr;
};

enum class e_decrypt_type : unsigned int {
  xenuine = 0,
  cindex,
  bone,
  health
};

struct decrypt_pointer_packet {
  unsigned int process_id = 0;
  e_decrypt_type type = e_decrypt_type::xenuine;
  unsigned long long value = 0;
  unsigned long long result = 0;
};

struct bulk_read_entry {
  unsigned long long source = 0;
  void *dest = nullptr;
  unsigned long long size = 0;
};

struct bulk_copy_packet {
  unsigned int process_id = 0;
  unsigned int count = 0;
  bulk_read_entry *entries = nullptr;
};

// Packet wrapper — passed via KTRAP_FRAME->Rdx
class c_packet {
public:
  c_packet() = default;
  c_packet(e_syscall syscall, void *buffer, unsigned long long size)
      : m_syscall(syscall), m_buffer(buffer), m_size(size) {}
  ~c_packet() = default;

  template <typename T> T *get() const {
    if (!m_buffer || !m_size)
      return nullptr;
    if (m_buffer == (void*)0x12345678) return nullptr; // Safety
    
    return (T *)m_buffer;
  }

  const e_syscall get_syscall() const { return m_syscall; }
  unsigned long long get_size() const { return m_size; }

private:
  e_syscall m_syscall = e_syscall::null;
  void *m_buffer = nullptr;
  unsigned long long m_size = 0;
};

struct shared_communication {
    volatile int magic; // 0x5E4C7A02
    volatile int request; // 1 = pending request, 0 = done (idle)
    volatile NTSTATUS status;
    c_packet packet; // Embedded packet
    unsigned char buffer[8192];
};

// Read/Write memory packet
struct copy_process_memory_packet {
  unsigned int process_id = 0;
  unsigned long long source = 0;
  void *dest = nullptr;
  unsigned long long size = 0;
};

// Query process info packet (PID, PEB, base, CR3)
struct query_process_data_packet {
  unsigned int process_id = 0;
  void *peb = nullptr;
  void *base_address = nullptr;
  unsigned long long cr3 = 0;
};

struct hide_process_packet {
  unsigned int process_id = 0;
};

struct setup_shared_memory_packet {
  void* user_ptr = nullptr;
  unsigned int overlay_pid = 0;
};

struct mouse_move_packet {
    long x = 0;
    long y = 0;
    unsigned short button_flags = 0;
};