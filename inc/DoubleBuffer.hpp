/**
 * @file DoubleBuffer.hpp
 * @author ZXW2600 (zhaoxinwei74@gmail.com)
 * @brief  简易双缓冲区
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <atomic>

template <typename BufferType>
class DoubleBuffer
{
private:
    BufferType buffer[2];
    std::atomic_uint8_t buffer_index;
    std::atomic_bool unread;

public:
    BufferType &get()
    {
        unread = false;
        return buffer[buffer_index];
    }

    void set(BufferType value)
    {
        buffer[1 - buffer_index] = value;
        buffer_index = 1 - buffer_index;
        unread = true;
    }

    void swap()
    {
        buffer_index = 1 - buffer_index;
        unread = true;
    }

    BufferType &getSetBuffer()
    {
        return (buffer[1 - buffer_index]);
    }

    bool haveUnread()
    {
        return (unread);
    }
    DoubleBuffer() : buffer_index{0}, unread{false} {}
};
