#include "LinearAllocator.h"

namespace alloc
{
LinearAllocator::LinearAlloator(size_t size, void* start)
    : Allocator(size, start)
    , m_current(start)
{
    assert(size > 0);
}

LinearAllocator::~LinearAllocator()
{
    m_current = nullptr;
}

void* LinearAllocator::allocate(size_t size, uint8_t alignment)
{
    assert(size != 0);

    uint8_t adjustment = math::alignForwardAdjustment(m_current, alignment);
    if(m_usedMemory + adjustment + size > m_size)
        return nullptr;
    
    uintptr_t aligned_address = (uintptr_t)m_current + adjustment;

    m_current = (void*)(aligned_address + size);
    m_usedMemory += size + adjustment;
    m_numAllocations++;

    return (void*)aligned_address;
}

void LinearAllocator::deallocate(void*)
{
    assert(false && "Please use clear()!");
}

void LinearAllocator::clear()
{
    m_numAllocations = 0;
    m_usedMemory = 0;

    m_current = m_start;
}

}// namespace alloc