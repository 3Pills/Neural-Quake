/*
Copyright (C) 2012-2016 Emil Hernvall
Licensed under MIT license
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

vector* vector_init()
{
	vector* v = malloc(sizeof(vector));

	v->data = NULL;
	v->size = 0;
	v->count = 0;

	return v;
}

int vector_count(vector *v)
{
	return v->count;
}

void vector_add(vector *v, void *e)
{
	if (v->size == 0) {
		v->size = 10;
		v->data = calloc(v->size, sizeof(void*));
	}

	if (v->size == v->count) {
		v->size *= 2;
		v->data = realloc(v->data, sizeof(void*) * v->size);
	}

	v->data[v->count] = e;
	v->count++;
}

void vector_insert(vector *v, int index, void *e)
{
	if (v->size <= v->count + 1) 
	{
		v->size *= 2;
		v->data = realloc(v->data, sizeof(void*) * v->size);
	}

	for (int i = v->count; i > index; i--)
	{
		v->data[i] = v->data[i - 1];
	}

	v->data[index] = e;
	v->count++;
}

void vector_set(vector *v, int index, void *e)
{
	if (index >= v->count) {
		return;
	}

	v->data[index] = e;
}

void *vector_get(vector *v, int index)
{
	if (index >= v->count) {
		return NULL;
	}

	return v->data[index];
}

void *vector_delete(vector *v, int index)
{
	if (index >= v->count) {
		return 0;
	}

	for (int i = index, j = index; i < v->count; i++) {
		v->data[j] = v->data[i];
		j++;
	}

	v->count--;

	return v->data[index];
}

void vector_clear(vector *v)
{
	for (int i = 0; i < v->count; i++)
		v->data[i] = 0;

	v->count = 0;
}

void vector_free(vector *v)
{
	free(v->data);
}

void vector_free_all(vector *v)
{
	free(v->data);
	free(v);
}