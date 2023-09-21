import java.util.concurrent.Semaphore;
import java.lang.InterruptedException;
import java.util.concurrent.TimeUnit;

class SemBoundedBuffer extends BoundedBuffer {
    Semaphore emptySlots, fullSlots;

    // Initialise the protected buffer structure above. 
    SemBoundedBuffer (int maxSize) {
        super(maxSize);
        // Initialize the synchronization attributes
        emptySlots = new Semaphore(maxSize);
        fullSlots = new Semaphore(0);
    }

    // Extract an element from buffer. If the attempted operation is
    // not possible immedidately, the method call blocks until it is.
    Object get() {
        Object value;

        // Enter mutual exclusion and enforce synchronisation semantics
        // using semaphores.

        try {
            fullSlots.acquire();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        synchronized (this) {


            value = super.get();

        // Leave mutual exclusion and enforce synchronisation semantics
        // using semaphores.
        }
        emptySlots.release();

        return value;
    }

    // Insert an element into buffer. If the attempted operation is
    // not possible immedidately, the method call blocks until it is.
    boolean put(Object value) {
        boolean done;

        // Enter mutual exclusion and enforce synchronisation semantics
        // using semaphores.

        try {
            emptySlots.acquire();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        synchronized (this) {


            done = super.put(value);

            // Leave mutual exclusion and enforce synchronisation semantics
            // using semaphores.
        }
        fullSlots.release();

        return done;
    }

    // Extract an element from buffer. If the attempted operation is not
    // possible immedidately, return NULL. Otherwise, return the element.
    Object remove() {
        boolean done = false;
        Object value = null;

        // Enter mutual exclusion and enforce synchronisation semantics
        // using semaphores.
        done = fullSlots.tryAcquire();

        synchronized (this) {

            if (done) value = super.get();
            

        // Leave mutual exclusion and enforce synchronisation semantics
        // using semaphores.
        }
        if (done) emptySlots.release();
        return value;
    }

    // Insert an element into buffer. If the attempted operation is
    // not possible immedidately, return 0. Otherwise, return 1.
    boolean add(Object value) {
        boolean done = false;

        // Enter mutual exclusion and enforce synchronisation semantics
        // using semaphores.
        done = emptySlots.tryAcquire();

        synchronized (this) {
            if (done) super.put(value);

        // Leave mutual exclusion and enforce synchronisation semantics
        // using semaphores.
        }
        if (done) fullSlots.release();

        return done;
    }

    
    // Extract an element from buffer. If the attempted operation is not
    // possible immedidately, the method call blocks until it is, but
    // waits no longer than the given deadline. Return the element if
    // successful. Otherwise, return NULL.
    Object poll(long deadline) {
        Object value = null;
        long    timeout = deadline - System.currentTimeMillis();
        boolean done = false;
        boolean interrupted = true;

        // Enter mutual exclusion and enforce synchronisation semantics
        // using semaphores.
        try {
            done = fullSlots.tryAcquire(timeout, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        synchronized (this) {

            if (done) value = super.get();

        // Leave mutual exclusion and enforce synchronisation semantics
        // using semaphores.
        }
        if (done) emptySlots.release();
        return value;
    }

    // Insert an element into buffer. If the attempted operation is not
    // possible immedidately, the method call blocks until it is, but
    // waits no longer than the given deadline. Return 0 if not
    // successful. Otherwise, return 1.
    boolean offer(Object value, long deadline) {
        long    timeout = deadline - System.currentTimeMillis();
        boolean done = false;
        boolean interrupted = true;

        // Enter mutual exclusion and enforce synchronisation semantics
        // using semaphores.
        try {
            done = emptySlots.tryAcquire(timeout, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        synchronized (this) {

            if (done) super.put(value);

        // Leave mutual exclusion and enforce synchronisation semantics
        // using semaphores.
        }
        if (done) fullSlots.release();
        return done;
    }
}
