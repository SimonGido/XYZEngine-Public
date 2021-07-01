#pragma once

namespace XYZ {

	struct Block
	{
		uint8_t* Data = nullptr;
		uint32_t NextAvailableIndex = 0;
	};
	struct Chunk
	{
		Chunk(uint32_t size, uint32_t chunkIndex, uint8_t blockIndex)
			: Size(size), ChunkIndex(chunkIndex), BlockIndex(blockIndex)
		{}
		uint32_t Size;		 // Size together with user data and metadata
		uint32_t ChunkIndex; // Index in block memory ( starts at the position of metadata )
		uint8_t  BlockIndex; // Index of block
	};

	template <uint32_t BlockSize, bool StoreSize = false>
	class MemoryPool
	{
	public:
		MemoryPool() = default;
		MemoryPool(MemoryPool&& other) noexcept;
		~MemoryPool();


		void* AllocateRaw(uint32_t size);
		void  DeallocateRaw(void* val);

		template <typename T, typename ...Args>
		T* Allocate(Args&&... args);

		template <typename T>
		void Deallocate(T* val);

		template <typename T>
		T* Get(uint32_t chunkIndex, uint8_t blockIndex);

		template <typename T>
		void ExtractMetaData(T* ptr, uint32_t* values) const;

		void ExtractMetaData(void* ptr, uint32_t* values) const;
	private:
		template <typename T>
		uint32_t toChunkSize() const;
		uint32_t toChunkSize(uint32_t size) const;

		void     cleanUp();
		void     sortFreeChunks();
		void     mergeFreeChunks();
		void     reverseMergeFreeChunks();
		Block*   createBlock();

		void storeMetaData(uint32_t memoryStart, uint8_t blockIndex, uint32_t size);
		bool findIndicesInFreeChunks(uint32_t sizeRequirement, uint8_t& blockIndex, uint32_t& dataIndex);
		std::pair<uint8_t, uint32_t> findAvailableIndex(uint32_t size);

		static constexpr uint32_t metaDataSize();
	private:
		std::vector<Block> m_Blocks;
		std::vector<Chunk> m_FreeChunks;

		bool m_Dirty = false;
		static constexpr size_t sc_MaxNumberOfBlocks = 255;
	};

	template<uint32_t BlockSize, bool StoreSize>
	inline MemoryPool<BlockSize, StoreSize>::MemoryPool(MemoryPool&& other) noexcept
		:
		m_Blocks(std::move(other.m_Blocks)),
		m_FreeChunks(std::move(other.m_FreeChunks)),
		m_Dirty(other.m_Dirty)
	{
	}
	template<uint32_t BlockSize, bool StoreSize>
	inline MemoryPool<BlockSize, StoreSize>::~MemoryPool()
	{
		for (auto& block : m_Blocks)
		{
			if (block.Data)
				delete[]block.Data;
		}
	}

	template<uint32_t BlockSize, bool StoreSize>
	inline Block* MemoryPool<BlockSize, StoreSize>::createBlock()
	{
		m_Blocks.push_back(Block());
		m_Blocks.back().Data = new uint8_t[BlockSize];
		memset(m_Blocks.back().Data, 0, BlockSize);
		return &m_Blocks.back();
	}

	template<uint32_t BlockSize, bool StoreSize>
	inline void MemoryPool<BlockSize, StoreSize>::cleanUp()
	{
		sortFreeChunks();
		mergeFreeChunks();
		reverseMergeFreeChunks();
		m_Dirty = false;
	}

	template<uint32_t BlockSize, bool StoreSize>
	inline bool MemoryPool<BlockSize, StoreSize>::findIndicesInFreeChunks(uint32_t sizeRequirement, uint8_t& blockIndex, uint32_t& dataIndex)
	{
		uint32_t counter = 0;
		for (auto& ch : m_FreeChunks)
		{
			if (ch.Size > sizeRequirement)
			{
				blockIndex = ch.BlockIndex;
				ch.Size -= sizeRequirement;
				// We make free chunk smaller, and we take required memory
				dataIndex = ch.ChunkIndex + ch.Size;
				return true;
			}
			else if (ch.Size == sizeRequirement)
			{
				blockIndex = ch.BlockIndex;
				dataIndex = ch.ChunkIndex;
				// Whole chunk is used, so we remove it
				m_FreeChunks.erase(m_FreeChunks.begin() + counter);
				return true;
			}
			counter++;
		}
		return false;
	}

	template<uint32_t BlockSize, bool StoreSize>
	inline std::pair<uint8_t, uint32_t> MemoryPool<BlockSize, StoreSize>::findAvailableIndex(uint32_t size)
	{
		uint32_t sizeRequirement = toChunkSize(size);
		uint32_t dataIndex = 0;
		uint8_t  blockIndex = 0;
		// First check free chunks, if there is any suitable chunk to hold memory
		if (findIndicesInFreeChunks(sizeRequirement, blockIndex, dataIndex))
		{
			storeMetaData(dataIndex, blockIndex, size);
			dataIndex += metaDataSize();
			return { blockIndex, dataIndex };
		}
		// Check if last block of memory can hold required size
		Block* last = &m_Blocks.back();
		if (last->NextAvailableIndex + sizeRequirement > BlockSize)
			last = createBlock(); // Last block can not, create new one

		// Store meta data
		storeMetaData(last->NextAvailableIndex, (uint8_t)m_Blocks.size() - 1, size);

		// Index for user memory is after meta data
		dataIndex = last->NextAvailableIndex + metaDataSize();

		// block index is index of the last block
		blockIndex = (uint8_t)m_Blocks.size() - 1;

		// Move next avilable index by size requirement
		last->NextAvailableIndex += sizeRequirement;
		return { blockIndex, dataIndex };
	}

	template<uint32_t BlockSize, bool StoreSize>
	inline void* MemoryPool<BlockSize, StoreSize>::AllocateRaw(uint32_t size)
	{
		XYZ_ASSERT(StoreSize, "Store size must be enabled");
		if (!m_Blocks.size())
			createBlock();
		if (m_Dirty)
			cleanUp();
		auto [blockIndex, chunkIndex] = findAvailableIndex(size);
		return &m_Blocks[blockIndex].Data[chunkIndex];
	}
	template<uint32_t BlockSize, bool StoreSize>
	inline void MemoryPool<BlockSize, StoreSize>::DeallocateRaw(void* val)
	{
		XYZ_ASSERT(StoreSize, "Store size must be enabled");
		// TODO: StoreSize must be enabled
		uint32_t indices[3];
		ExtractMetaData(val, indices);
		m_FreeChunks.emplace_back(toChunkSize(indices[2]), indices[1], (uint8_t)indices[0]);
		m_Dirty = true;
	}
	
	template<uint32_t BlockSize, bool StoreSize>
	inline void MemoryPool<BlockSize, StoreSize>::sortFreeChunks()
	{
		std::sort(m_FreeChunks.begin(), m_FreeChunks.end(), [](const Chunk& a, const Chunk& b) {
			if (a.BlockIndex == b.BlockIndex)
				return a.ChunkIndex < b.ChunkIndex;
			return a.BlockIndex < b.BlockIndex;
		});
	}
	template<uint32_t BlockSize, bool StoreSize>
	inline void MemoryPool<BlockSize, StoreSize>::mergeFreeChunks()
	{
		uint32_t sizeOffset = 0;
		for (auto it = m_FreeChunks.begin(); it != m_FreeChunks.end(); ++it)
		{
			for (auto it2 = it + 1; it2 != m_FreeChunks.end();)
			{
				// Not from the same block
				if (it->BlockIndex != it2->BlockIndex)
					break;
				// Chunks are connected so merge them

				if (it->ChunkIndex + it->Size == it2->ChunkIndex || it2->ChunkIndex + it2->Size == it->ChunkIndex)
				{
					it->ChunkIndex = std::min(it2->ChunkIndex, it->ChunkIndex);
					it->Size = it->Size + it2->Size;
					it2 = m_FreeChunks.erase(it2);
					continue;
				}
				else
				{
					// Chunks are not connected after their indices where sort so continue to check other chunks
					break;
				}
				it2++;
			}
		}
	}
	template<uint32_t BlockSize, bool StoreSize>
	inline void MemoryPool<BlockSize, StoreSize>::reverseMergeFreeChunks()
	{
		for (int64_t i = m_FreeChunks.size() - 1; i >= 0; --i)
		{
			Chunk& chunk = m_FreeChunks[i];
			Block& block = m_Blocks[chunk.BlockIndex];
			uint32_t chunkSpace = chunk.ChunkIndex + chunk.Size;
			if (chunkSpace != block.NextAvailableIndex)
				break;
			block.NextAvailableIndex -= chunk.Size;
			m_FreeChunks.erase(m_FreeChunks.begin() + i);
		}
	}
	

	template<uint32_t BlockSize, bool StoreSize>
	inline void MemoryPool<BlockSize, StoreSize>::storeMetaData(uint32_t memoryStart, uint8_t blockIndex, uint32_t size)
	{
		Block* block = &m_Blocks[(size_t)blockIndex];
		uint32_t chunkIndex = memoryStart;
		size_t blockIndexInMemory = (size_t)memoryStart;
		size_t chunkIndexInMemory = (size_t)memoryStart + sizeof(uint8_t);

		memcpy(&block->Data[blockIndexInMemory], &blockIndex, sizeof(uint8_t)); // Store block index
		memcpy(&block->Data[chunkIndexInMemory], &chunkIndex, sizeof(uint32_t)); // Store chunk index

		if (StoreSize)
		{
			size_t sizeIndexInMemory = chunkIndexInMemory + sizeof(uint32_t);
			memcpy(&block->Data[sizeIndexInMemory], &size, sizeof(uint32_t)); // Store size
		}
	}

	
	template<uint32_t BlockSize, bool StoreSize>
	inline constexpr uint32_t MemoryPool<BlockSize, StoreSize>::metaDataSize()
	{
		if (StoreSize)
			return (uint32_t)sizeof(uint32_t) + (uint32_t)sizeof(uint8_t) + sizeof(uint32_t);
		return (uint32_t)sizeof(uint8_t) + sizeof(uint32_t);
	}

	template<uint32_t BlockSize, bool StoreSize>
	template<typename T, typename ...Args>
	inline T* MemoryPool<BlockSize, StoreSize>::Allocate(Args && ...args)
	{
		if (!m_Blocks.size())
			createBlock();
		if (m_Dirty)
			cleanUp();
		auto [blockIndex, chunkIndex] = findAvailableIndex((uint32_t)sizeof(T));
		return new (&m_Blocks[blockIndex].Data[chunkIndex])T(std::forward<Args>(args)...);
	}

	template<uint32_t BlockSize, bool StoreSize>
	template<typename T>
	inline void MemoryPool<BlockSize, StoreSize>::Deallocate(T* val)
	{
		val->~T();
		uint32_t indices[3];
		ExtractMetaData(val, indices);
		Chunk chunk(toChunkSize<T>(), indices[1], (uint8_t)indices[0]);
		m_FreeChunks.push_back(chunk);
		m_Dirty = true;
	}

	template<uint32_t BlockSize, bool StoreSize>
	template<typename T>
	inline T* MemoryPool<BlockSize, StoreSize>::Get(uint32_t chunkIndex, uint8_t blockIndex)
	{
		return reinterpret_cast<T*>(&m_Blocks[blockIndex].Data[chunkIndex]);
	}

	template<uint32_t BlockSize, bool StoreSize>
	template<typename T>
	inline void MemoryPool<BlockSize, StoreSize>::ExtractMetaData(T* ptr, uint32_t* value) const
	{
		uint8_t* blockPtr = (uint8_t*)ptr - metaDataSize();
		value[0] = *blockPtr;
		uint8_t* chunkPtr = blockPtr + sizeof(uint8_t);
		value[1] = *(uint32_t*)(chunkPtr);

		if (StoreSize)
		{
			// size is stored in previous 4 bytes
			uint8_t* sizePtr = (uint8_t*)chunkPtr + sizeof(uint32_t);
			value[2] = (uint32_t)(*sizePtr);
		}
	}

	template<uint32_t BlockSize, bool StoreSize>
	template<typename T>
	inline uint32_t MemoryPool<BlockSize, StoreSize>::toChunkSize() const
	{
		return metaDataSize() + (uint32_t)sizeof(T);
	}

	template<uint32_t BlockSize, bool StoreSize>
	inline void MemoryPool<BlockSize, StoreSize>::ExtractMetaData(void* ptr, uint32_t* values) const
	{
		uint8_t* blockPtr = (uint8_t*)ptr - metaDataSize();
		values[0] = *blockPtr;
		uint8_t* chunkPtr = blockPtr + sizeof(uint8_t);
		values[1] = *(uint32_t*)(chunkPtr);

		if (StoreSize)
		{
			// size is stored in previous 4 bytes
			uint8_t* sizePtr = (uint8_t*)chunkPtr + sizeof(uint32_t);
			values[2] = (uint32_t)(*sizePtr);
		}
	}

	template<uint32_t BlockSize, bool StoreSize>
	uint32_t MemoryPool<BlockSize, StoreSize>::toChunkSize(uint32_t size) const
	{
		return metaDataSize() + size;
	}
}