/*
Copyright (C) 2012-2016 Emil Hernvall
Licensed under MIT license
*/

#ifndef VECTOR_H__
#define VECTOR_H__

typedef struct vector_ {
	void** data;
	int size;
	int count;
} vector;

vector* vector_init();
int vector_count(vector*);
void vector_add(vector*, void*);
void vector_insert(vector*, int, void*);
void vector_set(vector*, int, void*);
void *vector_get(vector*, int);
void *vector_delete(vector*, int);
void vector_free(vector*);
void vector_free_all(vector*);

#endif // !VECTOR_H__