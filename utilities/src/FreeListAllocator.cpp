#include "FreeListAllocator.h"

namespace alloc
{
FreeListAllocator::FreeListAllocator(size_t size, void* start)
    : Allocator(size, start)
    , m_freeBlocks((FreeBlock*)start)
{
    assert(size > sizeof(FreeBlock));

    m_freeBlocks->size = size;
    m_freeBlocks->next = nullptr;
}

FreeListAllocator::~FreeListAllocator()
{
    m_freeBlocks = nullptr;
}

void* FreeListAllocator::allocate(size_t size, uint8_t alignment)
{
    assert(size != 0 && alignment != 0);

    FreeBlock* prevFreeBlock = nullptr;
    FreeBlock* freeBlock     = m_freeBlocks;

    while (freeBlock != nullptr)
    {
        // Calculate adjustment needed to keep object correctly aligned
        uint8_t adjustment = math::alignForwardAdjustmentWithHeader(freeBlock, alignment, sizeof(AllocationHeader));
        size_t totalSize   = size + adjustment;

        // If allocation doesn't fit in this FreeBlock, try the next
        if (freeBlock->size < totalSize)
        {
            prevFreeBlock = freeBlock;
            freeBlock     = freeBlock->next;
            continue;
        }

        assert(sizeof(AllocationHeader) >= sizeof(FreeBlock) && "sizeof(AllocationHeader) < sizeof(FreeBlock)");

        // If allocations in the remaining memory will be impossible
        if (freeBlock->size - totalSize <= sizeof(AllocationHeader))
        {
            // Increase allocation size instead of creating a new FreeBlock
            totalSize = freeBlock->size;

            if (prevFreeBlock != nullptr)
                prevFreeBlock->next = freeBlock->next;
            else
                m_freeBlocks = freeBlock->next;
        }
        else
        {
            // Else create a new FreeBlock containing remaining memory
            FreeBlock* nextBlock = (FreeBlock*)(math::add(freeBlock, totalSize));
            nextBlock->size      = freeBlock->size - totalSize;
            nextBlock->next      = freeBlock->next;

            if (prevFreeBlock != nullptr)
                prevFreeBlock->next = nextBlock;
            else
                m_freeBlocks = nextBlock;
        }

        uintptr_t alignedAddress = (uintptr_t)freeBlock + adjustment;

        AllocationHeader* header = (AllocationHeader*)(alignedAddress - sizeof(AllocationHeader));
        header->size             = totalSize;
        header->adjustment       = adjustment;

        m_usedMemory += total_size;
        m_numAllocations++;

        assert(math::alignForwardAdjustment((void*)alignedAddress, alignment) == 0);

        return (void*)alignedAddress;
    }

    assert(false && "Couldn't find free block large enough!");

    return nullptr;
}


void FreeListAllocator::deallocate(void* p)
{
    assert(p != nullptr);

    AllocationHeader* header = (AllocationHeader*)pointer_math::subtract(p, sizeof(AllocationHeader));

    uintptr_t blockStart = reinterpret_cast<uintptr_t>(p) - header->adjustment;
    size_t    blockSize  = header->size;
    uintptr_t blockEnd   = blockStart + blockSize;

    FreeBlock* prevFreeBlock = NULL;
    FreeBlock* freeBlock     = _free_blocks;

    while (freeBlock != NULL)
    {
        if ((uintptr_t)freeBlock >= blockEnd)
            break;

        prevFreeBlock = freeBlock;
        freeBlock     = freeBlock->next;
    }

    if (prevFreeBlock == NULL)
    {
        prevFreeBlock       = (FreeBlock*)blockStart;
        prevFreeBlock->size = blockSize;
        prevFreeBlock->next = m_freeBlocks;

        m_freeBlocks = prevFreeBlock;
    }
    else if ((uintptr_t)prevFreeBlock + prevFreeBlock->size == blockStart)
    {
        prevFreeBlock->size += blockSize;
    }
    else
    {
        FreeBlock* temp       = (FreeBlock*)blockStart;
        temp->size            = blockSize;
        temp->next            = prevFreeBlock->next;
        prevFreeBlock->next = temp;

        prevFreeBlock = temp;
    }

    if (freeBlock != NULL && (uintptr_t)freeBlock == blockEnd)
    {
        prevFreeBlock->size += freeBlock->size;
        prevFreeBlock->next = freeBlock->next;
    }

    m_numAllocations--;
    m_usedMemory -= blockSize;
}

} // namespace alloc