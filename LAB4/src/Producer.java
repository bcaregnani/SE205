class Producer extends Thread {
    BoundedBuffer buffer;
    long id;

    Producer (int n, BoundedBuffer buffer) {
        super(String.format("producer %02d", n + Utils.nConsumers));
        this.id = n + Utils.nConsumers;
        this.buffer = buffer;
    }

    public void run() {
        Integer value;
        boolean done;
        long deadline = Utils.startTime;

        for (int i = 0; i < (Utils.nValues/Utils.nProducers); i++) {
            Utils.resynchronize(id);
            deadline = deadline + Utils.producerPeriod;
            value = new Integer ((int)id * 100 + i);
            switch (Utils.sem_producers) {
            case Utils.BLOCKING :
                done = buffer.put(value);
                break;
            case Utils.NONBLOCKING :
                done = buffer.add(value);
                break;
            case Utils.TIMEDOUT :
                done = buffer.offer(value, deadline);
                break;
            default :
                return;
            }
            if (!done) value=null;
            Utils.printLog
                (Utils.elapsedTime(), getName(), 1, Utils.sem_producers, value);
            Utils.delayUntil (deadline);
        }
    }
}

