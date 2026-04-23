#pragma once

struct query_process_data_packet {
  unsigned int process_id = 0;
  void *peb = nullptr;
  void *base_address = nullptr;
  unsigned long long cr3 = 0;
  bool is_debugged = false;
  unsigned long long kernel_time = 0;
};
