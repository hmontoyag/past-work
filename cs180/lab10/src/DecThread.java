/**
 * Created by hmontoya on 10/29/15.
 */
public class DecThread implements Runnable{
    private Counter counter;

    public DecThread(Counter counter) {
        this.counter = counter;
    }

    @Override
    public void run() {
        for (int i = 0; i < 10000; i++) {
            counter.dec();
        }
    }
}
