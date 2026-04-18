#pragma once
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>

namespace mapper {

    /**
     * @brief Thực hiện Manual Mapping driver từ mảng byte trong RAM
     * @param driver_buffer: Mảng byte của driver.sys (đã giải mã)
     * @param driver_size: Kích thước mảng byte
     * @return true nếu nạp thành công, false nếu thất bại
     */
    bool MapDriver(const std::vector<uint8_t>& driver_buffer);

    /**
     * @brief Giải mã XOR mảng byte
     */
    void XorBuffer(std::vector<uint8_t>& buffer, const std::string& key);

    /**
     * @brief Các lỗi thường gặp khi nạp
     */
    enum class MapperError {
        Success = 0,
        VulnerableDriverFailed = 1,
        AllocatePoolFailed = 2,
        EntryPointFailed = 3,
        InvalidBuffer = 4
    };

} // namespace mapper
