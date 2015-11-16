#include <kconf/config.h>
#include <kdebug/debug.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kcommon/ordered_table.h>

char standard_lessthan_predicate(type_t a, type_t b)
{
	return (a<b) ? 1:0;
}

struct ordered_array place_ordered_array(void *addr, unsigned int max_size, lessthan_predicate_t less_than)
{
	struct ordered_array to_ret;
	to_ret.array = (type_t*)addr;
	memset(to_ret.array, NULL, max_size*sizeof(type_t)); //nullify
	to_ret.size = 0;
	to_ret.max_size = max_size;
	to_ret.less_than = less_than;
	return to_ret;
}

void destroy_ordered_array(struct ordered_array *array)
{
//    kfree(array->array);
}

void insert_ordered_array(void *item, struct ordered_array *array)
{
	//KASSERT(array->less_than);
	unsigned int iterator = 0;
	while(iterator < array->size && array->less_than(array->array[iterator], item))
	{
		iterator++;
	}

	if(iterator == array->size)
	{
		array->array[array->size++] = item;
	}
	else
	{
		void *tmp = array->array[iterator];
		array->array[iterator] = item;

		while(iterator < array->size)
		{
			iterator++;
			void *tmp2 = array->array[iterator];
			array->array[iterator] = tmp;
			tmp = tmp2;
		}
		array->size++;
	}
}

void *lookup_ordered_array(unsigned int i, struct ordered_array *array)
{
	//KASSERT(i < array->size);
	return array->array[i];
}

void remove_ordered_array(unsigned int i, struct ordered_array *array)
{
	while (i < array->size)
	{
		array->array[i] = array->array[i+1];
		i++;
	}
	array->size--;
}
