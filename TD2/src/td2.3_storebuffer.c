#include <stdatomic.h>
#include <stdio.h>
#include <pthread.h>

// Two shared variables (neither atomic, nor with a well specified memory order)
static int x = 0;
static int y = 0;

// A shared counter (atomic, with memory order sequential consistency)
static atomic_int counter = 0;

// First thread, accessing shared variables
void *threadA(void *parameter)
{
  x = 1;
  atomic_thread_fence( memory_order_acq_rel );
  if (y == 0)
    counter++;

  return NULL;
}

// Second thread, accessing shared variables
void *threadB(void *parameter)
{
  y = 1;
  atomic_thread_fence( memory_order_acq_rel );
  if (x == 0)
    counter++;

  return NULL;
}

// Create and start two threads over and over again and check the outcome.
// Always returns 0.
int main()
{
  unsigned int i = 0;
  while(++i)
  {
    // create the two threads
    pthread_t a,b;
    pthread_create(&a, NULL, &threadA, NULL);
    pthread_create(&b, NULL, &threadB, NULL);

    // wait for both threads to finish
    pthread_join(a, NULL);
    pthread_join(b, NULL);

    // Can this happen? Should this happen?
    if (counter == 2)
    {
      printf("He! (%d)\n", i);
      return 0;
    }

    // reset shared variables before restarting the experiment.
    x = 0;
    y = 0;
    counter = 0;
  }

  return 0;
}

/**
 * 
 * Q: Will this program ever terminate? What do you observe when you run the program
 *    (maybe try running it several times)?
 * A: It is not assured it will terminate, under the sequentially consisten model it 
 *    certanly doesn't terminate but it could do it in my PCs because of the memory 
 *    model TSO. What we observe is that every time we run it we obtain a different 
 *    result.
 * 
 * Q: Call atomic_thread_fence() after the assignments to x and y for both threads. What
 *    happens now?
 * A: The result depends on what parameter we use in the function.
 * 
 * 
 * Q: Is the program race-free? Explain your answer in detail.
 * A: It is not race free. "x" and "y" have race.
 * 
 * Q: What does this example demonstrate?
 * A: This example demonstrate that the programmer has to be very careful at the moment
 *    of writing code because depending on the memory model there can be bugs introduced
 *    that are very difficult to detect.
 * 
 * 
*/