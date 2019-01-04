#include "errm.h"
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/wait.h>
#include <threads.h>
#include <unistd.h>

#include <cmocka.h>

#define SIG_DEATH_TEST(FUNC, SIG, ...)                                         \
    do {                                                                       \
        int pid = fork();                                                      \
        if (!pid) {                                                            \
            FUNC(__VA_ARGS__);                                                 \
        } else if (pid > 0) {                                                  \
            int state;                                                         \
            wait(&state);                                                      \
            assert_true(WIFSIGNALED(state) && WTERMSIG(state) == SIG);         \
        } else {                                                               \
            assert_true(false && "Internal error in sig_death_test().");       \
        }                                                                      \
    } while (0)

thread_local char *p = NULL;

static void *mocked_malloc(size_t size) {
    (void)size;
    p += 1;
    return p;
}

static void mocked_free(void *_p) {
    (void)_p;
    p -= 1;
}

edf_func1(int, t_defer_frees, size_t, n) {
    for (size_t i = 0; i < n; i++) {
        void *p = mocked_malloc(i);
        defer(mocked_free, p);
    }
    return 0;
}

edf_func2(int, t_defer_frees_two_args, size_t, n, int, dummy) {
    (void)dummy;
    for (size_t i = 0; i < n; i++) {
        void *p = mocked_malloc(i);
        defer(mocked_free, p);
    }
    return 0;
}

static void test_edf_func1_defers_under_max(void **state) {
    (void)state;
    t_defer_frees(MAX_DYN_DEFERS - 1);
    assert_ptr_equal(p, 0);
}

static void test_edf_func2_defers_under_max(void **state) {
    (void)state;
    t_defer_frees_two_args(MAX_DYN_DEFERS - 1, /* dummy */ 0);
    assert_ptr_equal(p, 0);
}

static void test_edf_func1_defers_over_max(void **state) {
    (void)state;

    SIG_DEATH_TEST(t_defer_frees, SIGABRT, MAX_DYN_DEFERS + 1);
}

static void test_edf_func2_defers_over_max(void **state) {
    (void)state;

    SIG_DEATH_TEST(t_defer_frees_two_args, SIGABRT, MAX_DYN_DEFERS + 1,
                   /*dummy*/ 0);
}

static void test_edf_func1_defers_max(void **state) {
    (void)state;

    t_defer_frees(MAX_DYN_DEFERS);
    assert_ptr_equal(p, 0);
}

static void test_edf_func2_defers_max(void **state) {
    (void)state;

    t_defer_frees_two_args(MAX_DYN_DEFERS, /* dummy */ 0);
    assert_ptr_equal(p, 0);
}

void do_nothing(void *dummy) {
    (void)dummy;
    for (size_t i = 0; i < STACK_SIZE + 2; i++) {
        defer(do_nothing, NULL);
    }
}

static void test_defers_stack_max(void **state) {
    (void)state;

    SIG_DEATH_TEST(do_nothing, SIGABRT, NULL);
}

struct foo {
    void *bar;
};

void foo_init(struct foo *f) { f->bar = mocked_malloc(10); }
void foo_free(struct foo *f) { mocked_free(f->bar); }

edf_func1(int, t_static_defer_frees, size_t, n) {
    for (size_t i = 0; i < n; i++) {
        struct foo f;
        foo_init(&f);
        static_defer((DeferHandle)foo_free, sizeof(struct foo), &f);
    }
    return 0;
}

static void test_static_defers_under_max(void **state) {
    (void)state;

    t_static_defer_frees(MAX_STATIC_DEFERS - 1);
    assert_ptr_equal(p, NULL);
}

static void test_static_defers_max(void **state) {
    (void)state;

    t_static_defer_frees(MAX_STATIC_DEFERS);
    assert_ptr_equal(p, NULL);
}

static void test_static_defers_over_max(void **state) {
    (void)state;

    SIG_DEATH_TEST(t_static_defer_frees, SIGABRT, MAX_STATIC_DEFERS + 1);
}

int main() {

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_edf_func1_defers_over_max),
        cmocka_unit_test(test_edf_func2_defers_over_max),
        cmocka_unit_test(test_edf_func1_defers_under_max),
        cmocka_unit_test(test_edf_func2_defers_under_max),
        cmocka_unit_test(test_edf_func1_defers_max),
        cmocka_unit_test(test_edf_func2_defers_max),
        cmocka_unit_test(test_defers_stack_max),
        cmocka_unit_test(test_static_defers_under_max),
        cmocka_unit_test(test_static_defers_over_max),
        cmocka_unit_test(test_static_defers_max),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
