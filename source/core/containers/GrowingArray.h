#pragma once
#include "core/Types.h"
#include "logger/debug.h"

#include <initializer_list>

namespace Core
{
	template <typename T>
	class GrowingArray
	{
	public:
		~GrowingArray();

		GrowingArray(std::initializer_list<T> list)
			: m_Capacity(list.size())
			, m_Data(new T[m_Capacity])
		{
			m_Size = list.size();
			memset(m_Data, 0, sizeof(T) * m_Capacity);
			memcpy(&m_Data[0], list.begin(), m_Size * sizeof(T));
		}

		GrowingArray(int32 size)
			: m_Capacity(size)
			, m_Data(new T[m_Capacity])
		{
			memset(m_Data, 0, sizeof(T) * m_Capacity);
		}

		GrowingArray()
			: m_Data(new T[m_Capacity])
		{
			memset(m_Data, 0, sizeof(T) * m_Capacity);
		}

		GrowingArray(const GrowingArray<T>& other) { *this = other; }

		GrowingArray<T>& operator=(const GrowingArray<T>& other)
		{
			m_Size = other.m_Size;
			m_Capacity = other.m_Capacity;
			m_Data = new T[m_Capacity];
			memcpy(&m_Data[0], &other.m_Data[0], sizeof(T) * m_Size);
			return *this;
		}

		T& operator[](uint32 index)
		{
			ASSERT(index < m_Size, "index has to be less than the size of the array");
			return m_Data[index];
		}
		
		const T& operator[](uint32 index) const 
		{
			ASSERT(index < m_Size, "index has to be less than the size of the array");
			 return m_Data[index];
		}

		uint32 Size() const { return m_Size; }
		uint32 Capacity() const { return m_Capacity; }

		void Add(const T& object)
		{
			if(m_Size + 1 > m_Capacity)
				Grow();

			m_Data[m_Size++] = object;
		}

		T& GetLast() { return m_Data[m_Size - 1]; }
		T& GetFirst() { return m_Data[0]; }

		void RemoveCyclicAtIndex(uint32 index) { m_Data[index--] = GetLast(); }

		void DeleteAll()
		{
			for(uint32 i = 0; i < m_Size; i++)
			{
				delete m_Data[i];
				m_Data[i] = nullptr;
			}
		}

		typedef T* iterator;
		typedef const T* const_iterator;
		iterator begin() { return &m_Data[0]; }
		const_iterator begin() const { return &m_Data[0]; }
		iterator end() { return &m_Data[m_Size - 1]; }
		const_iterator end() const { return &m_Data[m_Size - 1]; }

	private:
		void Grow()
		{
			m_Capacity *= 2;
			T* data = new T[m_Capacity];

			memcpy(data, m_Data, sizeof(T) * m_Size);

			delete[] m_Data;
			m_Data = data;
		}

		uint32 m_Size = 0;
		uint32 m_Capacity = 10;
		T* m_Data = nullptr;
	};

	template <typename T>
	GrowingArray<T>::~GrowingArray()
	{
		delete[] m_Data;
		m_Data = nullptr;
		m_Size = 0;
		m_Capacity = 0;
	}

}; // namespace Core
