#pragma once

class ArenaAllocator {
public:
    inline explicit ArenaAllocator(size_t bytes)
        : m_size(bytes)
    {
        m_buffer = static_cast<std::byte*>(malloc(m_size));
        m_offset = m_buffer;
    }

    // CPPCHECK - noCopyConstructor
    inline ArenaAllocator(const ArenaAllocator& other) = delete;

    // CPPCHECK - noOperatorEq
    inline ArenaAllocator operator=(const ArenaAllocator& other) = delete;

    // CPPCHECK - noDestructor
    inline ~ArenaAllocator()
    {
        free(m_buffer); // Free memory on destruction
    }

    template <typename T>
    inline T* alloc()
    {
        void* offset = m_offset;
        m_offset += sizeof(T);
        return static_cast<T*>(offset);
    }

private:
    size_t m_size;
    std::byte* m_buffer;
    std::byte* m_offset;
};