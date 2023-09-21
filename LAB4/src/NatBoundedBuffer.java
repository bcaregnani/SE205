import java.util.concurrent.Semaphore;
import java.lang.InterruptedException;
import java.util.concurrent.TimeUnit;

class NatBoundedBuffer extends BoundedBuffer {

   // Initialise the protected buffer structure above. 
   NatBoundedBuffer (int maxSize) {
      super(maxSize);
   }

   // Extract an element from buffer. If the attempted operation is
   // not possible immediately, the method call blocks until it is.
   Object get() {
      // Object value;

      // Enter mutual exclusion
      synchronized (this) {

      
         // Wait until there is a full slot available.
         while (this.size == 0) {
            try {
               wait();
            } catch (InterruptedException e) {
               e.printStackTrace();
            }
         }

         // Signal or broadcast that an empty slot is available (if needed)
         if (this.size == maxSize) {
            notify();
         }


         return super.get();

      // Leave mutual exclusion
      }
   }

   // Insert an element into buffer. If the attempted operation is
   // not possible immedidately, the method call blocks until it is.
   boolean put(Object value) {
      
   // Enter mutual exclusion
   synchronized (this) {
      
   // Wait until there is a empty slot available.
   while (this.size == this.maxSize) {
      try {
         wait();
      } catch (InterruptedException e) {
         e.printStackTrace();
      }
   }

   // Signal or broadcast that a full slot is available (if needed)
   if (this.size == 0) {
      notify();
   }

   super.put(value);

   // Leave mutual exclusion
   }

   return true;
   }

    // Extract an element from buffer. If the attempted operation is not
    // possible immedidately, return NULL. Otherwise, return the element.
    Object remove() {

      // Enter mutual exclusion
      synchronized (this) {
            
         // Signal or broadcast that an empty slot is available (if needed)
         if (this.size == this.maxSize) {
            notify();
         }

         return super.get();

      // Leave mutual exclusion
      }
    }

    // Insert an element into buffer. If the attempted operation is
    // not possible immedidately, return 0. Otherwise, return 1.
    boolean add(Object value) {
      // boolean done;

         // Enter mutual exclusion
         synchronized (this) {
         
         // Signal or broadcast that a full slot is available (if needed)
         if (this.size == 0) {
            notify();
         }

         return super.put(value);

         // Leave mutual exclusion
         }
    }

   // Extract an element from buffer. If the attempted operation is not
   // possible immedidately, the method call blocks until it is, but
   // waits no longer than the given deadline. Return the element if
   // successful. Otherwise, return NULL.
   Object poll(long deadline) {
      // long    timeout;

      // Enter mutual exclusion
      synchronized (this) {

         // Wait until a full slot is available but wait
         // no longer than the given deadline
         try {
            wait(deadline - System.currentTimeMillis());
         } catch (InterruptedException e) {
            e.printStackTrace();
         }

         if (size == 0) return null;

         // Signal or broadcast that an empty slot is available (if needed)
         if (this.size == this.maxSize) {
            notify();
         }

         return super.get();

      // Leave mutual exclusion 
      }
   }

    // Insert an element into buffer. If the attempted operation is not
    // possible immedidately, the method call blocks until it is, but
    // waits no longer than the given deadline. Return 0 if not
    // successful. Otherwise, return 1.
    boolean offer(Object value, long deadline) {
      // long    timeout;

      // Enter mutual exclusion
      synchronized (this) {

         // Wait until a empty slot is available but wait
         // no longer than the given deadline
         try {
            wait(deadline - System.currentTimeMillis());
         } catch (InterruptedException e) {
            e.printStackTrace();
         }
   
         if (size == maxSize) return false;

         // Signal or broadcast that a full slot is available (if needed)
         if (this.size == 0) {
            notify();
         }

         super.put(value);

      // Leave mutual exclusion
      }
      return true;
    }
}
