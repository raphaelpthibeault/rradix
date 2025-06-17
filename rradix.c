#include <rradix.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/* Return the padding needed by vertex
 * The padding is needed to store the child pointers to aligned addresses
 * Add 4 to the vertex_size because a vertex has a 4-byte header */
#define radix_padding(vertex_size) ((sizeof(void*)-((vertex_size+4) % sizeof(void*))) & (sizeof(void*)-1))

/* Return the current total size of the vertex. 
 * The second line calculates the padding after the string, needed to save ptrs to aligned addresses */
#define radix_vertex_current_size(v) ( \
    sizeof(radix_vertex)+(v)->size+ \
    radix_padding((v)->size)+ \
    ((v)->is_compressed ? sizeof(radix_vertex *) : sizeof(radix_vertex *)*(v)->size)+ \
    (((v)->is_key && !(v)->is_null)*sizeof(void*)) \
)

/* Return the pointer to the last child pointer in a vertex.
 * For a compressed vertex this is the only child pointer  */
#define radix_vertex_last_child_ptr(v) ((radix_vertex **) ( \
			((uint8_t *)(v)) + \
			radix_vertex_current_size(v) - \
			sizeof(radix_vertex *) - \
			(((v)->is_key && (v)->is_null) ? sizeof(void *) : 0) \
))

static radix_vertex *
new_vertex(size_t children, bool datafield)
{
	size_t size = sizeof(radix_vertex) + children + radix_padding(children) + sizeof(radix_vertex*) * children;
	if (datafield)
		size += sizeof(void *);

	radix_vertex *v = malloc(size);
	if (v == NULL) return NULL;

	v->is_key = 0;
	v->is_key = 0;
	v->is_compressed = 0;
	v->size = children;
	return v;
}

radix_tree *
radix_new(void)
{
	radix_tree *t = malloc(sizeof(*t));
	if (t == NULL) return NULL;

	t->num_elements = 0;
	t->num_vertices = 1;
	t->head = new_vertex(0, false);
	
	if (t->head == NULL)
	{
		radix_free(t);
		return NULL;
	}

	return t;
}

static void *
radix_get_data(radix_vertex *v)
{
	if (v->is_null) return NULL;

	void **vdata = (void **)((uint8_t*)v + radix_vertex_current_size(v) - sizeof(void *));
	void *data;
	memcpy(&data, vdata, sizeof(void *)); /* ? v->size ? */
	return data;
}

static void
_radix_free(radix_tree *t, radix_vertex *v, void (*free_callback)(void *))
{
	int num_children = v->is_compressed ? 1 : v->size;	
	radix_vertex **cp = radix_vertex_last_child_ptr(v);

	while (num_children--)
	{
		radix_vertex *c;
		memcpy(&c, cp, sizeof(*c));
		_radix_free(t, c, free_callback);
		--cp;
	}

	if (free_callback != NULL && !v->is_null && v->is_key)
		free_callback(radix_get_data(v));

	free(v);
	--t->num_vertices;
}

void 
radix_free_callback(radix_tree *t, void (*free_callback)(void *))
{
	_radix_free(t, t->head, free_callback);
	assert(t->num_vertices == 0);
	free(t);
}

void 
radix_free(radix_tree *t)
{
	radix_free_callback(t, NULL);
}

