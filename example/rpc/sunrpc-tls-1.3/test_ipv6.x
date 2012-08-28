/* Test SunRPC service. -*- C -*-
 */

program TESTPROG {
  version TESTPROG_VERS1 {
    /* Return the string, with "Hello " prepended. */
    string test_hello (string) = 1;
    /* Return the current date on the server, as a string. */
    string test_ctime (void) = 2;
  } = 1;
} = 0x20008000;
