#ifndef __AgePriorityQueue_h__
#define __AgePriorityQueue_h__

#pragma once


template<typename ValueType, typename AgeType = float, typename PriorityType = float>
struct AgePriorityQueue
{
	typedef uint32 id_type;
	typedef ValueType value_type;
	typedef AgeType age_type;
	typedef PriorityType priority_type;

	typedef AgePriorityQueue<ValueType, AgeType, PriorityType> type;

private:
	enum { user_bits	= 16, };
	enum { user_shift	= (sizeof(id_type) << 3) - user_bits, };

	struct container_type
	{
		container_type()
			: age((age_type)0)
			, priority((priority_type)0)
			, free(false)
			, user((id_type)0)
			, value()
		{
		}

		explicit container_type(const value_type& _value)
			: age((age_type)0)
			, priority((priority_type)0)
			, free(false)
			, user((id_type)0)
			, value(_value)
		{
		}

		void reuse(const value_type& _value)
		{
			free = false;
			++user;
			age = (age_type)0;
			priority = (priority_type)0;
			value = _value;
		}

		value_type value;
		age_type age;
		priority_type priority;
		uint16 user : user_bits;
		uint16 free : 1;
	};

private:
	typedef std::vector<container_type> Slots;
	typedef std::deque<id_type> FreeSlots;
	typedef std::deque<id_type> Queue;

public:
	inline void clear()
	{
		m_slots.clear();
		m_free.clear();
		m_queue.clear();
	}

	inline bool empty() const
	{
		return m_queue.empty();
	}

	inline size_t size() const
	{
		return m_queue.size();
	}

	inline void swap(type& other)
	{
		m_slots.swap(other.m_slots);
		m_free.swap(other.m_free);
		m_queue.swap(other.m_queue);
	}

	inline const id_type& front_id() const
	{
		return m_queue.front();
	}

	inline value_type& front()
	{
		container_type& slot = m_slots[_slot(front_id())];
		return slot.value;
	}

	inline const value_type& front() const
	{
		container_type& slot = m_slots[_slot(front_id())];
		return slot.value;
	}

	inline const id_type& back_id() const
	{
		return m_queue.back();
	}

	inline value_type& back()
	{
		container_type& slot = m_slots[_slot(back_id())];
		return slot.value;
	}

	inline const value_type& back() const
	{
		container_type& slot = m_slots[_slot(back_id())];
		return slot.value;
	}

	inline void pop_front()
	{
		assert(!m_slots[_slot(front_id())].free);
		assert(m_slots[_slot(front_id())].user == _user(front_id()));

		id_type id = front_id();
		_free_slot(id);

		m_queue.pop_front();
	}

	inline id_type push_back(const value_type& value)
	{
		if (m_free.empty())
		{
			m_slots.push_back(container_type(value));
			id_type id = m_slots.size();
			m_queue.push_back(id);

			return id;
		}

		id_type id = m_free.front();
		m_free.pop_front();

		container_type& slot = m_slots[_slot(id)];
		assert(slot.free);
		slot.reuse(value);
		assert(slot.user != _user(id));

		id = _id(slot.user, _slot(id));
		m_queue.push_back(id);

		return id;
	}

	inline void erase(const id_type& id)
	{
		if (stl::find_and_erase(m_queue, id))
		{
			_free_slot(id);
		}
		else
		{
			assert(!"unknown id!");
		}
	}

	inline bool has(const id_type& id) const
	{
		size_t slot = _slot(id);
		return ((slot < m_slots.size()) && (m_slots[slot].user == _user(id)) && (!m_slots[slot].free));
	}

	struct DefaultUpdate
	{
		priority_type operator()(const age_type& age, value_type& value)
		{
			return age;
		}
	};

	struct DefaultCompare
	{
		bool operator()(const id_type& lid, const priority_type& lpriority, const id_type& rid, const priority_type& rpriority) const
		{
			return lpriority > rpriority;
		}
	};

	template<typename ElementCompare>
	struct QueueCompare
	{
		QueueCompare(const Slots& _slots, ElementCompare& _compare)
			: slots(_slots)
			, compare(_compare)
		{
		}

		bool operator()(const id_type& left, const id_type& right) const
		{
			const container_type& leftSlot = slots[type::_slot(left)];
			const container_type& rightSlot = slots[type::_slot(right)];

			return compare(left, leftSlot.priority, right, rightSlot.priority);
		}

		const Slots& slots;
		ElementCompare& compare;
	};

	template<typename Update, typename Compare>
	inline void update(const age_type& aging, Update& update, Compare& compare)
	{
		if (!m_queue.empty())
		{
			Queue::iterator qit = m_queue.begin();
			Queue::iterator qend = m_queue.end();

			for ( ; qit != qend; )
			{
				container_type& slot = m_slots[_slot(*qit)];
				assert(!slot.free);
				assert(slot.user == _user(*qit));

				slot.age += aging;
				slot.priority = update(slot.age, slot.value);
				++qit;
			}

			std::stable_sort(m_queue.begin(), m_queue.end(), QueueCompare<Compare>(m_slots, compare));
		}
	}

	template<typename Update, typename Compare>
	inline void partial_update(uint32 count, const age_type& aging, Update& update, Compare& compare)
	{
		if (!m_queue.empty())
		{
			Queue::iterator qit = m_queue.begin();
			Queue::iterator qend = m_queue.end();

			for ( ; qit != qend; )
			{
				container_type& slot = m_slots[_slot(*qit)];
				assert(!slot.free);
				assert(slot.user == _user(*qit));

				slot.age += aging;
				slot.priority = update(slot.age, slot.value);
				++qit;
			}

			count = std::min<uint32>(count, m_queue.size());
			std::partial_sort(m_queue.begin(), m_queue.begin() + count, m_queue.end(), QueueCompare<Compare>(m_slots, compare));
		}
	}


	inline value_type& operator[](const id_type& id)
	{
		assert(m_slots[_slot(id)].user == _user(id));
		return m_slots[_slot(id)].value;
	}

	inline const value_type& operator[](const id_type& id) const
	{
		assert(m_slots[_slot(id)].user == _user(id));
		return m_slots[_slot(id)].value;
	}

	inline const age_type& age(const id_type& id) const
	{
		assert(m_slots[_slot(id)].user == _user(id));
		return m_slots[_slot(id)].age;
	}

	inline const priority_type& priority(const id_type& id) const
	{
		assert(m_slots[_slot(id)].user == _user(id));
		return m_slots[_slot(id)].priority;
	}

protected:
	inline void _free_slot(const id_type& id)
	{
		container_type& slot = m_slots[_slot(id)];
		assert(slot.user == _user(id));
		assert(!slot.free);

		if (!slot.free && (slot.user == _user(id)))
		{
			slot.free = true;
			m_free.push_back(id);
		}
	}

	inline static id_type _id(const id_type& user, const id_type& slot)
	{
		return (user << user_shift) | ((slot + 1) & ((1 << user_shift) - 1));
	}

	inline static id_type _slot(const id_type& id)
	{
		return ((id & ((1 << user_shift) - 1)) - 1);
	}

	inline static id_type _user(const id_type& id)
	{
		return (id >> user_shift);
	}

	Slots m_slots;
	FreeSlots m_free;
	Queue m_queue;
};


#endif
