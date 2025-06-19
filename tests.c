#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <rradix.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

static void
radix_new_should_init(void **state)
{
	(void)state;

	radix_tree *t = radix_new();

	assert_non_null(t);
	assert_non_null(t->head);
	assert_int_equal(t->num_elements, 0);
	assert_int_equal(t->num_vertices, 1);

	radix_free(t);
}

static void
radix_insert_should_insert(void **state)
{
	(void)state;

	radix_tree *t = radix_new();
	int ret;
	void *val;

	ret = radix_insert(t, (uint8_t *)"foo", 3, (void *)(long)1, NULL);
	assert_int_equal(ret, 1);
	val = radix_find(t, (uint8_t *)"foo", 3);

	assert_true(val == (void*)(long)1);

	ret = radix_insert(t, (uint8_t *)"foo", 3, (void *)(long)2, NULL);
	assert_int_equal(ret, 0); // key and overwrite, updates value to 2 and returns 0
	val = radix_find(t, (uint8_t *)"foo", 3);
	assert_true(val == (void*)(long)2);

	radix_free(t);
}

static void
radix_insert_should_compress(void **state)
{
	(void)state;

	radix_tree *t = radix_new();
	radix_insert(t, (uint8_t *)"foo", 3, (void *)(long)1, NULL);
	radix_insert(t, (uint8_t *)"foobar", 6, (void *)(long)2, NULL);
	radix_insert(t, (uint8_t *)"footer", 6, (void *)(long)3, NULL);
	radix_insert(t, (uint8_t *)"first", 5, (void *)(long)4, NULL);

	assert_int_equal(t->num_elements, 4);
	assert_int_equal(t->num_vertices, 10);

#ifdef DEBUG
	radix_print(t);
#endif

	radix_free(t);
}

static void
radix_del_vertex_with_no_children_should_cleanup(void **state)
{
	(void)state;

	radix_tree *t = radix_new();
	radix_insert(t, (uint8_t *)"foo", 3, (void *)(long)1, NULL);
	radix_insert(t, (uint8_t *)"foobar", 6, (void *)(long)2, NULL);

#ifdef DEBUG
	radix_print(t);
#endif

	assert_int_equal(t->num_elements, 2);
	assert_int_equal(t->num_vertices, 3);

	radix_del(t, (uint8_t *)"foobar", 6, NULL);

	assert_int_equal(t->num_elements, 1);
	assert_int_equal(t->num_vertices, 2);

#ifdef DEBUG
	radix_print(t);
#endif

	radix_free(t);
}

static void
radix_del_vertex_with_children_should_compress(void **state)
{
	(void)state;	

	radix_tree *t = radix_new();
	radix_insert(t, (uint8_t *)"foobar", 6, (void *)(long)2, NULL);
	radix_insert(t, (uint8_t *)"footer", 6, (void *)(long)3, NULL);

#ifdef DEBUG
	radix_print(t);
#endif

	assert_int_equal(t->num_elements, 2);
	assert_int_equal(t->num_vertices, 6);

	radix_del(t, (uint8_t *)"footer", 6, NULL);

	assert_int_equal(t->num_elements, 1);
	assert_int_equal(t->num_vertices, 2);

#ifdef DEBUG
	radix_print(t);
#endif

	radix_free(t);
}

int
main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(radix_new_should_init),
		cmocka_unit_test(radix_insert_should_insert),
		cmocka_unit_test(radix_insert_should_compress),
		cmocka_unit_test(radix_del_vertex_with_no_children_should_cleanup),
		cmocka_unit_test(radix_del_vertex_with_children_should_compress),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
