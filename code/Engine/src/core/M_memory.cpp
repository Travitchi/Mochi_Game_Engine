#include "M_memory.h"
#include "logger.h"
#include "Platform.h"
//todo: This is a temporary fix, we need to implement our own string functions to avoid using the C standard library
#include "string.h"
#include "stdio.h"

struct memory_stat
{
	u64 total_allocated;
	u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memory_tag_string[MEMORY_TAG_MAX_TAGS] = {
	"UNKNOWN",
	"ARRAY",
	"DARRAY",
	"DICT",
	"RING_QUEUE",
	"BST",
	"STRING",
	"APPLICATION",
	"JOB",
	"MAT_INST",
	"RENDERER",
	"GAME",
	"TRANSFORM",
	"ENTITY",
	"ENTITY_NODE",
	"SCENE"
};

static struct memory_stat stats;

void initialize_memory()
{
	platform_zero_memory(&stats, sizeof(stats));
}

void shutdown_memory()
{

}

void* Mallocate(u64 size, memory_tag tag)
{
	if (tag == MEMORY_TAG_UNKOWN)
	{
		MWARN("Allocating memory with unknown tag, you need to class this allocation!");
	}

	stats.total_allocated += size;
	stats.tagged_allocations[tag] += size;

	//todo: Memory Allignment
	void* block = platform_allocate(size, FALSE);
	platform_zero_memory(block, size);
	return block;
}

void Mfree(void* block, u64 size, memory_tag tag)
{
	if (tag == MEMORY_TAG_UNKOWN)
	{
		MWARN("Freeing memory with unknown tag, you need to class this allocation!");
	}
	stats.total_allocated -= size;
	stats.tagged_allocations[tag] -= size;

	//todo: Memory Allignment
	platform_free(block, FALSE);

}

void* Mzero_memory(void* block, u64 size)
{
	return platform_zero_memory(block, size);
}

void* Mcopy_memory(void* dest, const void* src, u64 size)
{
	return platform_copy_memory(dest, src, size);
}

void* Mset_memory(void* dest, i32 value, u64 size)
{
	return platform_set_memory(dest, value, size);
}

char* get_memory_usage_str()
{
	const u64 GiB = 1024 * 1024 * 1024;
	const u64 MiB = 1024 * 1024;
	const u64 KiB = 1024;

	char buffer[8000] = "System Memory Usage (tagged): \n";
	u64 offset = strlen(buffer);
	for(u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i)
	{
		char unit[4] = "XiB";
		float amount = 1.0f;
		if (stats.tagged_allocations[i] >= GiB)
		{
			unit[0] = 'G';
			amount = stats.tagged_allocations[i] / (float)GiB;
		}
		else if (stats.tagged_allocations[i] >= MiB)
		{
			unit[0] = 'M';
			amount = stats.tagged_allocations[i] / (float)MiB;
		}
		else if (stats.tagged_allocations[i] >= KiB)
		{
			unit[0] = 'K';
			amount = stats.tagged_allocations[i] / (float)KiB;
		}
		else
		{
			unit[0] = 'B';
			unit[1] = 0;
			amount = (float)stats.tagged_allocations[i];
		}

		i32 length = snprintf(buffer + offset, 8000, " %s: %.2f%s\n", memory_tag_string[i], amount, unit);
		offset += length;
	}

	char* out_string = _strdup(buffer);
	return out_string;
}