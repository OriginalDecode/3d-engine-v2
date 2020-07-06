#pragma once
#include "Core/Types.h"
#include "logger/Debug.h"

namespace Core
{
	template <typename T, uint32 ArrSize = 10>
	class Array
	{
	public:
		Array(const T(&list)[ArrSize]) {
			m_Capacity = ArrSize;
			m_Size = ArrSize;
			m_Data = new T[m_Capacity];
			memcpy(m_Data, &list[0], sizeof(T) * m_Capacity);
		}

		Array() { m_Data = new T[m_Capacity]; };
		~Array()
		{
			delete[] m_Data;
			m_Data = nullptr;
		};

		T& operator[](uint32 index) { return m_Data[index]; }
		const T& operator[](uint32 index) const { return m_Data[index]; }

		uint32 Size() const { return m_Size; }
		uint32 Capacity() const { return m_Capacity; }

		void DeleteAll()
		{
			for(uint32 i = 0; i < m_Size; i++)
			{
				delete m_Data[i];
				m_Data[i] = nullptr;
			}
		}

		Array<T>& operator=(const Array<T>& other)
		{
			m_Size = other.m_Size;
			m_Capacity = other.m_Capacity;
			m_Data = new T[m_Capacity];
			memcpy(m_Data, other.m_Data, sizeof(T) * m_Size);
			return *this;
		}

		void Add(const T& object)
		{
			ASSERT(m_Size != m_Capacity, "Array is full.");
			m_Data[m_Size++] = object;
		}

		typedef T* iterator;
		typedef const T* const_iterator;
		iterator begin() { return &m_Data[0]; }
		const_iterator begin() const { return &m_Data[0]; }
		iterator end() { return &m_Data[m_Size]; }
		const_iterator end() const { return &m_Data[m_Size]; }

	private:
		uint32 m_Size = 0;
		uint32 m_Capacity = ArrSize;
		T* m_Data = nullptr;
	};

}; // namespace Core
