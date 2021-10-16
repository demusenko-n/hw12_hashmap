#pragma once
#include <utility>
#include <string>
#include <cmath>
#include <vector>

namespace custom_hashmap
{
	template<class Hash = std::hash<std::string>, class Alloc = std::allocator<std::string>>
	class hashmap
	{
		static constexpr size_t _min_table_size = 7;

		struct hash_item
		{
			const std::string name;
			const std::string phone;
			std::string address;
			size_t hash;

			hash_item(std::string name, std::string phone, std::string address, size_t hash);

			hash_item();
		};

		template<class Ty>
		using alloc_traits_type = std::allocator_traits<Ty>;

		template<class Ty>
		using alloc_type = typename alloc_traits_type<Alloc>::template rebind_alloc<Ty>;

		using alloc_hashitem = alloc_type<hash_item>;
		using traits_hashitem = alloc_traits_type<alloc_hashitem>;

		using alloc_ptr = alloc_type<hash_item*>;
		using traits_ptr = alloc_traits_type<alloc_ptr>;


		using hash = Hash;

	public:
		hashmap(const hashmap& other);

		hashmap(hashmap&& other)noexcept;

		[[nodiscard]] size_t size() const;

		[[nodiscard]] size_t count() const;

		hashmap& operator=(const hashmap& other)&;

		hashmap& operator=(hashmap&& other)& noexcept;

	private:
		hash hasher_;

		alloc_hashitem allocator_;
		alloc_ptr allocator_ptr_;

		size_t size_;
		size_t count_;

		hash_item** arr_;


		static size_t get_optimized_size(size_t min_size);

		static bool is_prime(size_t number);

		[[nodiscard]] bool is_ideal_index(size_t hash, size_t current_index)const;

		[[nodiscard]] int get_ideal_index(size_t hash)const;

		void check_for_ideal_index(const size_t free_ideal_index);

		[[nodiscard]] hash_item** find_first_empty()const;

		void move_back(hash_item** new_arr, size_t new_size);
	public:
		~hashmap();

		int find_index(const std::string& name);

		void rehash(size_t min_size);

		explicit hashmap(size_t min_size);
		hashmap();

		void add(std::string name, std::string phone, std::string address);

		bool remove(const std::string& name);

		std::string get_phone(const std::string& name);

		std::string get_address(const std::string& name);

		std::string get_name(const std::string& phone);
	};

	template <class Hash, class Alloc>
	hashmap<Hash, Alloc>::hash_item::hash_item(std::string name, std::string phone, std::string address, size_t hash) :
		name(std::move(name)),
		phone(std::move(phone)),
		address(std::move(address)),
		hash(hash)
	{}

	template <class Hash, class Alloc>
	hashmap<Hash, Alloc>::hash_item::hash_item()
	{
		hash = SIZE_MAX;
	}

	template <class Hash, class Alloc>
	hashmap<Hash, Alloc>::hashmap(const hashmap& other)
	{
		size_ = other.size_;
		count_ = other.count_;

		arr_ = traits_ptr::allocate(allocator_ptr_, size_);
		for (size_t i = 0; i < size_; ++i)
		{
			arr_[i] = traits_hashitem::allocate(allocator_, 1);
			traits_hashitem::construct(allocator_, arr_[i], *other.arr_[i]);
		}
	}

	template <class Hash, class Alloc>
	hashmap<Hash, Alloc>::hashmap(hashmap&& other) noexcept
	{
		size_ = other.size_;
		count_ = other.count_;
		arr_ = other.arr_;
		allocator_ = std::move(other.allocator_);
		allocator_ptr_ = std::move(other.allocator_ptr_);
		other.arr_ = nullptr;
	}

	template <class Hash, class Alloc>
	size_t hashmap<Hash, Alloc>::size() const
	{
		return size_;
	}

	template <class Hash, class Alloc>
	size_t hashmap<Hash, Alloc>::count() const
	{
		return count_;
	}

	template <class Hash, class Alloc>
	hashmap<Hash, Alloc>& hashmap<Hash, Alloc>::operator=(const hashmap& other) &
	{
		if (this == &other)
		{
			return *this;
		}

		this->~hashmap();
		size_ = other.size_;
		count_ = other.count_;

		arr_ = traits_ptr::allocate(allocator_ptr_, size_);
		for (size_t i = 0; i < size_; ++i)
		{
			arr_[i] = traits_hashitem::allocate(allocator_, 1);
			traits_hashitem::construct(allocator_, arr_[i], *other.arr_[i]);
		}

		return *this;
	}

	template <class Hash, class Alloc>
	hashmap<Hash, Alloc>& hashmap<Hash, Alloc>::operator=(hashmap&& other) & noexcept
	{
		this->~hashmap();
		size_ = other.size_;
		count_ = other.count_;
		arr_ = other.arr_;
		allocator_ = std::move(other.allocator_);
		allocator_ptr_ = std::move(other.allocator_ptr_);
		other.arr_ = nullptr;
		return *this;
	}

	template <class Hash, class Alloc>
	size_t hashmap<Hash, Alloc>::get_optimized_size(size_t min_size)
	{
		size_t size = std::max(min_size, _min_table_size);
		while (!is_prime(size))
		{
			size++;
		}
		return size;
	}

	template <class Hash, class Alloc>
	bool hashmap<Hash, Alloc>::is_prime(size_t number)
	{
		const double sqrt = std::sqrt(number);
		for (long long i = 2; i <= sqrt; i++)
			if (number % i == 0)
				return false;
		return true;
	}

	template <class Hash, class Alloc>
	bool hashmap<Hash, Alloc>::is_ideal_index(size_t hash, size_t current_index) const
	{
		return current_index == hash % size_;
	}

	template <class Hash, class Alloc>
	int hashmap<Hash, Alloc>::get_ideal_index(size_t hash) const
	{
		return hash % size_;
	}

	template <class Hash, class Alloc>
	void hashmap<Hash, Alloc>::check_for_ideal_index(const size_t free_ideal_index)
	{
		size_t temp_index = free_ideal_index + 1;

		while (temp_index < size_ && arr_[temp_index] != nullptr
			&& get_ideal_index(arr_[temp_index]) == free_ideal_index)
		{
			++temp_index;
		}
		--temp_index;

		if (temp_index < size_ && arr_[temp_index] != nullptr && get_ideal_index(arr_[temp_index]) == free_ideal_index)
		{
			arr_[free_ideal_index] = arr_[temp_index];
			arr_[temp_index] = nullptr;
			check_for_ideal_index(temp_index);
		}
	}

	template <class Hash, class Alloc>
	typename hashmap<Hash, Alloc>::hash_item** hashmap<Hash, Alloc>::find_first_empty() const
	{
		for (size_t i = 0; i < size_; ++i)
		{
			if (arr_[i] == nullptr)
			{
				return &arr_[i];
			}
		}
		return nullptr;
	}

	template <class Hash, class Alloc>
	void hashmap<Hash, Alloc>::move_back(hash_item** new_arr, size_t new_size)
	{
		for (size_t i = 0; i < new_size; ++i)
		{
			if (new_arr[i] != nullptr)
			{
				*find_first_empty() = new_arr[i];
			}
		}
	}

	template <class Hash, class Alloc>
	hashmap<Hash, Alloc>::~hashmap()
	{
		if (arr_ != nullptr)
		{
			for (size_t i = 0; i < size_; ++i)
			{
				if (arr_[i] != nullptr)
				{
					traits_hashitem::destroy(allocator_, arr_[i]);
					traits_hashitem::deallocate(allocator_, arr_[i], 1);
				}
			}
			traits_ptr::deallocate(allocator_ptr_, arr_, size_);
		}
	}

	template <class Hash, class Alloc>
	int hashmap<Hash, Alloc>::find_index(const std::string& name)
	{
		const size_t hash = hasher_(name);
		size_t index = hash % size_;

		while (index < size_ && arr_[index] != nullptr)
		{
			if (hash == arr_[index]->hash && name == arr_[index]->name)
			{
				return static_cast<int>(index);
			}
			++index;
		}
		return -1;
	}

	template <class Hash, class Alloc>
	void hashmap<Hash, Alloc>::rehash(size_t min_size)
	{
		if (size_ >= min_size)
		{
			return;
		}

		size_t new_size = get_optimized_size(min_size);

		hash_item** new_arr = traits_ptr::allocate(allocator_ptr_, new_size);
		for (size_t i = 0; i < new_size; ++i)
		{
			new_arr[i] = nullptr;
		}

		for (size_t i = 0; i < size_; ++i)
		{
			if (arr_[i] == nullptr) continue;

			std::vector<hash_item*> vec(arr_, arr_ + size_);
			std::vector<hash_item*> vec2(new_arr, new_arr + new_size);

			size_t new_index = arr_[i]->hash % new_size;
			while (new_arr[new_index] != nullptr)
			{
				++new_index;
				if (new_index >= new_size)		//I think this code is unreachable, but I'm not sure.
				{
					move_back(new_arr, new_size);
					traits_ptr::deallocate(allocator_ptr_, new_arr, new_size);
					rehash(new_index + 1);
					return;
				}
			}
			new_arr[new_index] = arr_[i];
			arr_[i] = nullptr;
		}
		traits_ptr::deallocate(allocator_ptr_, arr_, size_);

		arr_ = new_arr;
		size_ = new_size;
	}

	template <class Hash, class Alloc>
	hashmap<Hash, Alloc>::hashmap(size_t min_size)
	{
		size_ = get_optimized_size(min_size);
		arr_ = traits_ptr::allocate(allocator_ptr_, size_);
		for (size_t i = 0; i < size_; ++i)
		{
			arr_[i] = nullptr;
		}
		count_ = 0;
	}

	template <class Hash, class Alloc>
	hashmap<Hash, Alloc>::hashmap() : hashmap(0)
	{}

	template <class Hash, class Alloc>
	void hashmap<Hash, Alloc>::add(std::string name, std::string phone, std::string address)
	{
		const size_t hash = hasher_(name);
		size_t index = hash % size_;

		while (arr_[index] != nullptr)
		{
			++index;
			if (index >= size_)
			{
				rehash(index + 1);
				add(std::move(name), std::move(phone), std::move(address));
				return;
			}
		}
		arr_[index] = traits_hashitem::allocate(allocator_, 1);
		traits_hashitem::construct(allocator_, arr_[index], std::move(name), std::move(phone), std::move(address), hash);
		count_++;
	}

	template <class Hash, class Alloc>
	bool hashmap<Hash, Alloc>::remove(const std::string& name)
	{
		int pos = find_index(name);

		if (pos != -1)
		{
			traits_hashitem::destroy(allocator_, arr_[pos]);
			traits_hashitem::deallocate(allocator_, arr_[pos]);
			arr_[pos] = nullptr;
			check_for_ideal_index(pos);
			--count_;
			return true;
		}

		return false;
	}

	template <class Hash, class Alloc>
	std::string hashmap<Hash, Alloc>::get_phone(const std::string& name)
	{
		int pos = find_index(name);
		if (pos == -1)
		{
			throw std::exception();
		}
		return arr_[pos]->phone;
	}

	template <class Hash, class Alloc>
	std::string hashmap<Hash, Alloc>::get_address(const std::string& name)
	{
		int pos = find_index(name);
		if (pos == -1)
		{
			throw std::exception();
		}
		return arr_[pos]->address;
	}

	template <class Hash, class Alloc>
	std::string hashmap<Hash, Alloc>::get_name(const std::string& phone)
	{
		for (size_t i = 0; i < size_; ++i)
		{
			if (arr_[i] != nullptr && arr_[i]->phone == phone)
			{
				return arr_[i]->name;
			}
		}

		throw std::exception();
	}
}

