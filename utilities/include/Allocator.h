
//  Created by Qin, Jianbo on 2021/12/17.
//  Copyright © 2021 Qin, Jianbo. All rights reserved.

#pragma once

#include <new>
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace alloc
{

///@brief Allocator class only help you manage
/// the memory. such as construct or alignment.
/// you need to create the memory outside.
class Allocator
{
public:
    Allocator(size_t size, void* start)
    {
        m_start = start;
        m_size = size;

        m_usedMemory = 0;
        m_numAllocations = 0;
    }
    ~Allocator()
    {
        assert(m_numAllocations == 0 && m_usedMemory == 0);

        m_start = nullptr;
        m_size = 0;
    }

    virtual void* allocate(size_t size, uint8_t alignment = 4) = 0;
    virtual void deallocate(void* p) = 0;

    void* getStart() const 
    {
        return m_start;
    }

    size_t getSize() const 
    {
        return m_size;
    }

    size_t getUsedMemory() const 
    {
        return m_usedMemory;
    }

    size_t getNumAllocations() const 
    {
        return m_numAllocations;
    }

protected:
    void*  m_start;
    size_t m_size;

    size_t m_usedMemory;
    size_t m_numAllocations;
}; // class Allocator

namespace math
{
    inline void* alignForward(void* address, uint8_t alignment)
    {
        return (void*)((reinterpret_cast<uintptr_t>(address) + static_cast<uintptr_t>(alignment - 1)) &
                       static_cast<uintptr_t>(~(alignment - 1)));
    }
    inline const void* alignForward(const void* address, uint8_t alignment)
    {
        return (void*)((reinterpret_cast<uintptr_t>(address) + static_cast<uintptr_t>(alignment - 1)) &
                       static_cast<uintptr_t>(~(alignment - 1)));
    }
    
    inline void* alignBackward(void* address, uint8_t alignment)
    {
        return (void*)(reinterpret_cast<uintptr_t>(address) & static_cast<uintptr_t>(~(alignment - 1)));
    }
    inline const void* alignBackward(const void* address, uint8_t alignment)
    {
        return (void*)(reinterpret_cast<uintptr_t>(address) & static_cast<uintptr_t>(~(alignment - 1)));
    }
    
    inline uint8_t alignForwardAdjustment(const void* address, uint8_t alignment)
    {
        uint8_t adjustment = alignment - (reinterpret_cast<uintptr_t>(address) & static_cast<uintptr_t>(alignment - 1));

        if (adjustment == alignment)
            return 0;  // already aligned

        return adjustment;
    }
    inline uint8_t alignForwardAdjustmentWithHeader(const void* address, uint8_t alignment, uint8_t headerSize)
    {
        uint8_t adjustment = alignForwardAdjustment(address, alignment);

        uint8_t neededSpace = headerSize;

        if (adjustment < neededSpace)
        {
            neededSpace -= adjustment;

            // Increase adjustment to fit header
            adjustment += alignment * (neededSpace / alignment);

            if (neededSpace % alignment > 0)
                adjustment += alignment;
        }

        return adjustment;
    }
    inline uint8_t alignBackwardAdjustment(const void* address, uint8_t alignment)
    {
        uint8_t adjustment = reinterpret_cast<uintptr_t>(address) & static_cast<uintptr_t>(alignment - 1);

        if (adjustment == alignment)
            return 0;  // already aligned

        return adjustment;
    }

    inline void* add(void* p, size_t x)
    {
        return (void*)(reinterpret_cast<uintptr_t>(p) + x);
    }

    inline const void* add(const void* p, size_t x)
    {
        return (const void*)(reinterpret_cast<uintptr_t>(p) + x);
    }

    inline void* subtract(void* p, size_t x)
    {
        return (void*)(reinterpret_cast<uintptr_t>(p) - x);
    }

    inline const void* subtract(const void* p, size_t x)
    {
        return (const void*)(reinterpret_cast<uintptr_t>(p) - x);
    }
} // namespace math

namespace allocate
{
    template <class T>
    T* allocateNew(Allocator& allocator)
    {
        return new (allocator.allocate(sizeof(T), alignof(T))) T;
    }
    template <class T>
    T* allocateNew(Allocator& allocator, const T& t)
    {
        return new (allocator.allocate(sizeof(T), alignof(T))) T(t);
    }
    template <class T>
    T* allocateArray(Allocator& allocator, size_t length)
    {
        assert(length != 0);

        uint8_t headerSize = sizeof(size_t) / sizeof(T);

        if (sizeof(size_t) % sizeof(T) > 0)
            headerSize += 1;

        T* p = ((T*)allocator.allocate(sizeof(T) * (length + headerSize), alignof(T))) + headerSize;

        *(((size_t*)p) - 1) = length; // store length before the array

        for (size_t i = 0; i < length; i++)
            new (&p[i]) T;

        return p;
    }

    template <class T>
    void deallocateDelete(Allocator& allocator, T& object)
    {
        object.~T();
        allocator.deallocate(&object);
    }
    template <class T>
    void deallocateArray(Allocator& allocator, T* array)
    {
        ASSERT(array != NULL);

        size_t length = *(((size_t*)array) - 1);

        for (size_t i = 0; i < length; i++)
            array[i].~T();

        // Calculate how much extra memory was allocated to store the length before
        // the array
        uint8_t headerSize = sizeof(size_t) / sizeof(T);

        if (sizeof(size_t) % sizeof(T) > 0)
            headerSize += 1;

        allocator.deallocate(array - headerSize);
    }
} // namespacee allocate
} // namespace alloc