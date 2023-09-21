class Consumer extends Thread {
    BoundedBuffer buffer;
    long id;

    Consumer (int n, BoundedBuffer buffer) {
        super(String.format("consumer %02d", n));
        this.id = n;
        this.buffer = buffer;
    }

    public void run() {
        Integer value;
        long deadline = Utils.startTime;

        for (int i = 0; i < (Utils.nValues/Utils.nConsumers); i++) {
            Utils.resynchronize(id);
            deadline = deadline + Utils.consumerPeriod;
            
            switch (Utils.sem_consumers) {
            case Utils.BLOCKING :
                value = (Integer)buffer.get();
                break;
            case Utils.NONBLOCKING :
                value = (Integer)buffer.remove();
                break;
            case Utils.TIMEDOUT :
                value = (Integer)buffer.poll(deadline);
                break;
            default :
                return;
            }
            Utils.printLog
                (Utils.elapsedTime(), getName(), 0, Utils.sem_consumers, value);
            Utils.delayUntil (deadline);
        }
    }
}
