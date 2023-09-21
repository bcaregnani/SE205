#include "circular_buffer.h"
#include "protected_buffer.h"
#include "utils.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define EMPTY_SLOTS_NAME "/empty_slots"
#define FULL_SLOTS_NAME "/full_slots"

// Initialise the protected buffer structure above.
protected_buffer_t *sem_protected_buffer_init(int length) {
  protected_buffer_t *b;
  b = (protected_buffer_t *)malloc(sizeof(protected_buffer_t));
  b->buffer = circular_buffer_init(length);
  // Initialize the synchronization attributes
  // Use these filenames as named semaphores
  sem_unlink(EMPTY_SLOTS_NAME);
  sem_unlink(FULL_SLOTS_NAME);
  sem_unlink("sm");
  // Open the semaphores using the filenames above
  if ( ( b->emptySlots = sem_open(EMPTY_SLOTS_NAME, O_CREAT, O_RDWR, b->buffer->max_size) ) == SEM_FAILED)
  {
    perror("sem_open() failed to open semaphore emptySlots");
    exit(EXIT_FAILURE);
  };
  if ( ( b->fullSlots = sem_open(FULL_SLOTS_NAME, O_CREAT, O_RDWR, 0) ) == SEM_FAILED)
  {
    perror("sem_open() failed to open semaphore fullSlots");
    exit(EXIT_FAILURE);
  };
  if ( ( b->sem_mutex = sem_open("sm", O_CREAT, O_RDWR, 1) ) == SEM_FAILED)
  {
    perror("sem_open() failed to open semaphore sem_mutex");
    exit(EXIT_FAILURE);
  };

  return b;
}

// Extract an element from buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void *sem_protected_buffer_get(protected_buffer_t *b) {
  void *d;

  // Enforce synchronisation semantics using semaphores.
  if (sem_wait(b->fullSlots) == -1)
  {
    perror("sem_wait() error");
    exit(EXIT_FAILURE);
  };

  // Enter mutual exclusion.
  sem_wait(b->sem_mutex);

  d = circular_buffer_get(b->buffer);
  if (d == NULL)
    mtxprintf(pb_debug, "get (B) - data=NULL\n");
  else
    mtxprintf(pb_debug, "get (B) - data=%d\n", *(int *)d);

  // Leave mutual exclusion.
  sem_post(b->sem_mutex);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->emptySlots);

  return d;
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void sem_protected_buffer_put(protected_buffer_t *b, void *d) {

  // Enforce synchronisation semantics using semaphores.
  sem_wait(b->emptySlots);

  // Enter mutual exclusion.
  sem_wait(b->sem_mutex);

  circular_buffer_put(b->buffer, d);
  if (d == NULL)
    mtxprintf(pb_debug, "put (B) - data=NULL\n");
  else
    mtxprintf(pb_debug, "put (B) - data=%d\n", *(int *)d);

  // Leave mutual exclusion.
  sem_post(b->sem_mutex);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->fullSlots);
}

// Extract an element from buffer. If the attempted operation is not
// possible immedidately, return NULL. Otherwise, return the element.
void *sem_protected_buffer_remove(protected_buffer_t *b) {
  void *d = NULL;
  int rc = -1;

  // Enforce synchronisation semantics using semaphores.
  rc = sem_trywait(b->fullSlots);

  if (rc != 0) {
    if (d == NULL)
      mtxprintf(pb_debug, "remove (U) - data=NULL\n");
    else
      mtxprintf(pb_debug, "remove (U) - data=%d\n", *(int *)d);
    return d;
  }

  // Enter mutual exclusion.
  sem_wait(b->sem_mutex);

  d = circular_buffer_get(b->buffer);
  if (d == NULL)
    mtxprintf(pb_debug, "remove (U) - data=NULL\n");
  else
    mtxprintf(pb_debug, "remove (U) - data=%d\n", *(int *)d);

  // Leave mutual exclusion.
  sem_post(b->sem_mutex);

  // Enforce synchronisation semantics using semaphores.
  if (rc == 0)
  {
    sem_post(b->emptySlots);
  };
  
  
  return d;
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, return 0. Otherwise, return 1.
int sem_protected_buffer_add(protected_buffer_t *b, void *d) {
  int rc = -1;

  // Enforce synchronisation semantics using semaphores.
  rc = sem_trywait(b->emptySlots);

  if (rc != 0) {
    d = NULL;
    if (d == NULL)
      mtxprintf(pb_debug, "add (U) - data=NULL\n");
    else
      mtxprintf(pb_debug, "add (U) - data=%d\n", *(int *)d);
    return 0;
  }

  // Enter mutual exclusion.
  sem_wait(b->sem_mutex);

  circular_buffer_put(b->buffer, d);
  if (d == NULL)
    mtxprintf(pb_debug, "add (U) - data=NULL\n");
  else
    mtxprintf(pb_debug, "add (U) - data=%d\n", *(int *)d);

  // Leave mutual exclusion.
  sem_post(b->sem_mutex);

  // Enforce synchronisation semantics using semaphores.
  if (rc == 0)
  {
    sem_post(b->fullSlots);
  };


  return 1;
}


// Extract an element from buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return the element if
// successful. Otherwise, return NULL.
void *sem_protected_buffer_poll(protected_buffer_t *b,
                                struct timespec *abstime) {
  void *d = NULL;
  int rc = -1;

  // Enforce synchronisation semantics using semaphores.
  rc = sem_timedwait(b->fullSlots, abstime);

  if (rc != 0) {
    if (d == NULL)
      mtxprintf(pb_debug, "poll (T) - data=NULL\n");
    else
      mtxprintf(pb_debug, "poll (T) - data=%d\n", *(int *)d);
    return d;
  }

  // Enter mutual exclusion.
  sem_wait(b->sem_mutex);

  d = circular_buffer_get(b->buffer);
  if (d == NULL)
    mtxprintf(pb_debug, "poll (T) - data=NULL\n");
  else
    mtxprintf(pb_debug, "poll (T) - data=%d\n", *(int *)d);

  // Leave mutual exclusion.
  sem_post(b->sem_mutex);

  // Enforce synchronisation semantics using semaphores.
  if (rc == 0)
  {
    sem_post(b->emptySlots);
  };
  
  return d;
}

// Insert an element into buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return 0 if not
// successful. Otherwise, return 1.
int sem_protected_buffer_offer(protected_buffer_t *b, void *d,
                               struct timespec *abstime) {
  int rc = -1;

  // Enforce synchronisation semantics using semaphores.
  rc = sem_timedwait(b->emptySlots, abstime);

  if (rc != 0) {
    d = NULL;
    if (d == NULL)
      mtxprintf(pb_debug, "offer (T) - data=NULL\n");
    else
      mtxprintf(pb_debug, "offer (T) - data=%d\n", *(int *)d);
    return 0;
  }

  // Enter mutual exclusion.
  sem_wait(b->sem_mutex);

  circular_buffer_put(b->buffer, d);
  if (d == NULL)
    mtxprintf(pb_debug, "offer (T) - data=NULL\n");
  else
    mtxprintf(pb_debug, "offer (T) - data=%d\n", *(int *)d);

  // Leave mutual exclusion.
  sem_post(b->sem_mutex);

  // Enforce synchronisation semantics using semaphores.
  if (rc == 0)
  {
    sem_post(b->fullSlots);
  };
  

  return 1;
}
