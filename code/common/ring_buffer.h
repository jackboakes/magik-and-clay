#pragma once

#include <array>
#include <cassert>

template <typename T, size_t capacity> requires (capacity > 0)
class RingBuffer
{
private:
    std::array<T, capacity> m_Data;
    size_t m_ReadIndex { 0 };
    size_t m_WriteIndex { 0 };
    size_t m_Size { 0 };
public:
    void Write(const T& value)
    {
        assert(capacity != m_Size && "Ring Buffer is full at write time");
        m_Data[m_WriteIndex] = value;
        m_Size++;
        m_WriteIndex = (m_WriteIndex + 1) % capacity;
    }

    T Read()
    {
        assert(m_Size != 0 && "Ring Buffer is empty at read time");
        T& result = m_Data[m_ReadIndex];
        m_Size--;
        m_ReadIndex = (m_ReadIndex + 1) % capacity;
        return result;
    }

    bool Empty() const
    {
        return m_Size == 0;
    }
};