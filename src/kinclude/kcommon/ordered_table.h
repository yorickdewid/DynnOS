#ifndef __ORDERED_TABLE_H
#define __ORDERED_TABLE_H

typedef void* type_t;

typedef char (*lessthan_predicate_t)(type_t,type_t);

struct ordered_array
{
	type_t *array;
	unsigned int size;
	unsigned int max_size;
	lessthan_predicate_t less_than;
};

extern char standard_lessthan_predicate(type_t a, type_t b);
extern struct ordered_array create_ordered_array(unsigned int max_size, lessthan_predicate_t less_than);
extern struct ordered_array place_ordered_array(void *addr, unsigned int max_size, lessthan_predicate_t less_than);
extern void destroy_ordered_array(struct ordered_array *array);
extern void insert_ordered_array(type_t item, struct ordered_array *array);
extern void *lookup_ordered_array(unsigned int i, struct ordered_array *array);
extern void remove_ordered_array(unsigned int i, struct ordered_array *array);

#endif
