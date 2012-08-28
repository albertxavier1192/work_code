/* Test SunRPC service. -*- C -*-
 */

program TESTPROG {
  version TESTPROG_VERS1 {
    /* Test batching these calls together.  Batched calls must
     * return void.
     */
    void test_batch1 (string) = 1;
    void test_batch2 (string) = 2;
    void test_batch3 (string) = 3;

    /* This call is not batched: it returns the number of calls
     * processed by the server.
     */
    int test_num_calls (void) = 4;
  } = 1;
} = 0x20008000;
