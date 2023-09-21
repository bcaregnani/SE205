#include<stdio.h>
#include<pthread.h>

void * myThread ( void * parameter )
{
    printf( "Hello Thread %ld ! \n ", pthread_self() );
    return NULL ;
}

int main ( )
{
    pthread_t th1, th2 ; // create and start thread
    pthread_create(& th1 , NULL , myThread , NULL );
    pthread_join( th1 , NULL );

    pthread_create(& th2 , NULL , myThread , NULL );

    //pthread_join( th1 , NULL );
    pthread_join( th2 , NULL ); // wait for thread to terminate

    printf( " Hello Main ! \n " ) ;
    return 0;
}

/**
 * 
 * Q: What is pthread_join() actually doing here?
 * A: pthread_join() is waiting that the thread finishes, we tried 2 situations and see the results:
 * A: 1) Create first thread and wait to finish, and then create and wait to finish the second.
 * A: In this situation both threads have same ID.
 * A: 2) Create both threads and then wait for them to finish. In this situation they have differents IDs.
 * 
 * 
*/