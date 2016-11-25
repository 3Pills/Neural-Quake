/*
Copyright (C) 2012-2016 Emil Hernvall
Licensed under MIT license
*/

#ifndef VECTOR_H__
#define VECTOR_H__

typedef struct vector_ {
	void** data;
	unsigned int size;
	unsigned int count;
} vector;

vector* vector_init();
int vector_count(vector*);
void vector_push(vector*, void*);
void *vector_pop(vector*);
void vector_insert(vector*, unsigned int, void*);
void vector_set(vector*, unsigned int, void*);
void *vector_get(vector*, unsigned int);
void *vector_delete(vector*, unsigned int);
void vector_clear(vector*);
void vector_free(vector*);
void vector_free_all(vector*);

#endif // !VECTOR_H__