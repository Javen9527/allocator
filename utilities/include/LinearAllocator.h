#pragma once

#include "Allocator.h"
#include <cstdint>

namespace alloc
{
class LinearAllocator : public Allocator
{
public:
    LinearAlloator(size_t size, void* start);
    ~LinearAlloator();

    void* allocate(size_t size, uint8_t alignment) override;
    void deallocate(void* p) override;

    void clear();

    // prevent copy
    LinearAlloator(const LinearAlloator&) = delete;
    LinearAlloator& operator=(const LinearAlloator&) = delete;

private:
    void* m_current; // pointer to allocate next time
}; // class LinearAllocator

} // namespace alloc