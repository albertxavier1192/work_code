/* Test SunRPC service. -*- C -*-
 */

program TESTPROG {
  version TESTPROG_VERS1 {
    /* Return the string, with "Hello " prepended. */
    string test_hello (string) = 1;
    /* Return the current date on the server, as a string. */
    string test_ctime (void) = 2;
    /* Echo (for performance testing). */
    void test_echo (void) = 3;
    /* Return number of calls processed by client. */
    int test_num_calls (void) = 4;
  } = 1;
} = 0x20008000;
