/**
 * Created by hmontoya on 10/29/15.
 */
public class IncThread implements Runnable {

    private Counter counter;

    public IncThread(Counter counter) {
        this.counter = counter;
    }

    @Override
    public void run() {
        for (int i = 0; i < 10000; i++) {
            counter.inc();
        }
    }

}
