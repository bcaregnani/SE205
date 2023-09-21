#include <stdatomic.h>
#include <stdio.h>
#include <pthread.h>

#define SIZE 10

// A shared array of unsigned integers (non-atomic, no memory model specified)
static unsigned int x[SIZE] = {0,};

// A shared counter (atomic, with memory order sequential consistency)
static atomic_uint counter = 0;

// The same code for all threads.
void *thread(void *parameter)
{
  while(counter < SIZE)
  {
    x[atomic_fetch_add(&counter, 1)] = (unsigned int)pthread_self();
  }
  return NULL;
}

// Create and start two threads and see how their executions interleave.
// Always returns 0.
int main()
{
  // create the two threads
  // create the two threads.
  pthread_t a,b;
  pthread_create(&a, NULL, &thread, NULL);
  pthread_create(&b, NULL, &thread, NULL);

  // wait for both threads to finish
  pthread_join(a, NULL);
  pthread_join(b, NULL);

  // print the contents of the shared array
  for(unsigned int i = 0; i < SIZE; i++)
  {
    printf("x[%u] = %x\n", i, x[i]);
  }

  return 0;
}


/**
 * 
 * 
 * Q: Is the program race-free? Explain your answer in detail.
 * A: Yes it is. Manipulation on variable "counter" are atomic so no race.
 * A: Manipulations on variable "x" are done in different addresses thanks
 *    to the atomic access to the array.
 * 
 * Q: Which kind of interleavings are possible for this program? Which 
 *    interleavings can you observe? What might be happening here?
 * A: Any kind of interleaving is possible. Particularly in this case 
 *    when "SIZE" is small thread 1 fills completely the array "x" while
 *    thread 2 is being created, so there is no interleaving. On the 
 *    other hand if "SIZE" is large this gives time for thread 2 to be 
 *    created before thread 1 completely fills the array "x", then
 *    we can see an interleaving between the threads filling the array.
 * 
 * 
*/