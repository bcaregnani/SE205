public class BoundedBufferMain {

    public static void main(String[] args) {
        BoundedBuffer buffer;

        // Check the arguments of the command line
        if (args.length != 1) {
            System.out.println("PROGRAM FILENAME");
            System.exit(1);
        }
        Utils.init(args[0]);

        Consumer consumers[] = new Consumer[(int) Utils.nConsumers];
        Producer producers[] = new Producer[(int) Utils.nProducers];

        // Create a buffer
        if (Utils.sem_impl == 0)
            buffer = new NatBoundedBuffer(Utils.bufferSize);
        else
            buffer = new SemBoundedBuffer(Utils.bufferSize);

        

        // Create producers and then consumers

        // Create producers
        for (int i = 0; i < producers.length; i++) {
            producers[i] = new Producer(i, buffer);
        }

        // Create consumers
        for (int index = 0; index < consumers.length; index++) {
            consumers[index] = new Consumer(index, buffer);
        }



        // Start producers and then consumers

        // Start producers
        for (int i = 0; i < producers.length; i++) {
            producers[i].start();
        }

        // Start consumers
        for (int index = 0; index < consumers.length; index++) {
            consumers[index].start();
        }




        // Wait for producers to end and then wait for consumers to end

        // Wait for end producers
        for (int i = 0; i < producers.length; i++) {
            try {
                producers[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        // Wait for end consumers
        for (int index = 0; index < consumers.length; index++) {
            try {
                consumers[index].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }







    }
}
