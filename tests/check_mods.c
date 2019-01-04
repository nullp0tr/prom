#include "tests.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <cmocka.h>

static void test_mod_names(void **state) { (void)state; }

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mod_names),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
