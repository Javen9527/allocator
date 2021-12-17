#pragma once

#inlcude "Allocator.h"

namespace alloc
{
/// @brief 
/// useage example:
/// const uint32_t kmem = 16 * 1024; // 16k
/// void* memory = new char [kmem];
/// FreeListAllocator* allocator = 
///     new (memory) FreeListAllocator(
///         kmem - sizof(FreeListAllocator), 
///         math::add(memory, sizeof(FreeListAllocator)));
/// 
class FreeListAllocator : public Allocator
{
public:
    FreeListAllocator(size_t size, void* start);
    ~FreeListAllocator();

    void* allocate(size_t size, uint8_t alignment) override;
    void deallocate(void*) override;

    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;

private:
    struct AllocationHeader
    {
        size_t size;
        uint8_t adjustment;
    };

    struct FreeBlock
    {
        size_t size;
        FreeBlock* next;
    };

private:
    FreeBlock* m_freeBlocks;
};

}// namespace alloc