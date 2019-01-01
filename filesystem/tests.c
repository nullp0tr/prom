#define _DEFAULT_SOURCE

#include "filesystem.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <cmocka.h>

static void test_abs_path(void **state) {
    (void)state;

    const char *path = "filesystem.c";
    char *absolute_path = abs_path(path);
    assert_true(absolute_path[0] == '/');

    struct stat path_stat;
    stat(path, &path_stat);

    struct stat abs_stat;
    stat(absolute_path, &abs_stat);

    assert_true(path_stat.st_ino == abs_stat.st_ino);

    free(absolute_path);
}

static void test_abs_path_unparsable(void **state) {
    (void)state;

    const char *path = "/\n~\"'\".k";
    char *absolute_path = abs_path(path);
    assert_ptr_equal(absolute_path, NULL);
    assert_true(Errored);
    assert_int_equal(errm_error_code(), PARSING_ERROR);
}

static void test_dir_content(void **state) {
    (void)state;

    char tmp[] = "XXXXXX";
    char *tmp_folder = mkdtemp(tmp);
    if (!tmp_folder)
        assert_true(false);

    for (int i = 0; i < 10; i++) {
        char file[sizeof(tmp) + 2];
        snprintf(file, sizeof(file), "%s/%d", tmp_folder, i);
        FILE *fp = fopen(file, "w");
        fclose(fp);
    }

    size_t content_size;
    char **content = dir_content(tmp_folder, &content_size);
    (void)content;
    free_dir_content(content);

    for (int i = 0; i < 10; i++) {
        char file[sizeof(tmp) + 2];
        snprintf(file, sizeof(file), "%s/%d", tmp_folder, i);
        remove(file);
    }
    rmdir(tmp_folder);

    assert_int_equal(content_size, 10);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_abs_path),
        cmocka_unit_test(test_dir_content),
        cmocka_unit_test(test_abs_path_unparsable),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}