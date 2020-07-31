import java.util.LinkedList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.Semaphore;
import java.util.Queue;

public class Assignment2 {
    public static void main(String[] args){
        int numUsers = 15;
        int numPrinters = 5;
        int lenQueue = 10;

        // Create PrintManagementCenter for Users and Printers to connect to
        PrintManagementCenter pmc = new PrintManagementCenter(lenQueue);


        // Initialize users and printers array of type User and Printer, respectively
        List<Thread> userThreads = new LinkedList<Thread>();
        List<Thread> printerThreads = new LinkedList<Thread>();

        // 1. Initialize Users and Printers +10
        // 2. Start Users and Prints Threads +10
        for (int i = 1; i <= numPrinters; i++) {
            Thread t = new Thread(new Printer(i, pmc));
            t.start();
            printerThreads.add(t);
        }

        for (int i = 1; i <= numUsers; i++) {
            Thread t = new Thread(new User(i, pmc));
            t.start();
            userThreads.add(t);
        }

		// Wait for the users and printers
        try {
            Thread.sleep(20 * 1000);
        } catch (InterruptedException e){
            System.out.println("Printer Interrupted");
        }

        // 3. Shutdown Users and Printers +10

        for (Thread t : userThreads) {
            t.interrupt();
        }

        for (Thread t : printerThreads) {
            t.interrupt();
        }
    }
}

// No need to change UserFile
class UserFile{
    public int timeCost;
    public int ownerId;
    public int fileId;
    public UserFile(int ownerId, int fileId) throws InterruptedException {
        int minTimeCost = 1000;
        int maxTimeCost = 2000;
        Random random = new Random();
        this.timeCost = random.nextInt(maxTimeCost - minTimeCost + 1) + minTimeCost;
        this.ownerId = ownerId;
        this.fileId = fileId;

        // User will spend some time creating a UserFile
        int creationTime = random.nextInt(maxTimeCost - minTimeCost + 1) + minTimeCost;
        Thread.sleep(creationTime * 2);
    }

}

class User extends Thread{
    private int id;
    private PrintManagementCenter printManagementCenter;

    public User(int id, PrintManagementCenter printManagementCenter){
        this.id = id;
        this.printManagementCenter = printManagementCenter;
    }

    @Override
    public void run(){
        // 4. User Loop +10
        int i = 0;
        while(true) {
            try {
                UserFile file = new UserFile(this.id, i);
                i++;
                this.printManagementCenter.Upload(file);
                System.out.println("User#" + file.ownerId + " uploaded UserFile#" + file.fileId);
                Thread.sleep(1000);
            } catch(InterruptedException e) {
                break;
            } 
        }
    }
}

class PrintManagementCenter{
    private Semaphore sem;
    private Queue<UserFile> q;
    private Semaphore mutex;

    public PrintManagementCenter(int len){
        // 5. Create Semaphore, Mutex, and Queue +10
        this.sem = new Semaphore(len);
        this.mutex = new Semaphore(len);
        this.q = new LinkedList<UserFile>();
    }

    public void Upload(UserFile userFile) throws InterruptedException {
        // 6. Upload Function +20
        try {
            this.sem.acquire();
            this.mutex.acquire();
            this.q.add(userFile);
        } finally {
            this.mutex.release(); 
        }
    }

    public UserFile Download() throws InterruptedException {
        // 7. Download Function +20
        UserFile file = null;
        try {
            this.mutex.acquire();
            file = this.q.poll();
        } finally {
            this.mutex.release();
            if (file != null) {
                this.sem.release();
            }
        }
        return file;
    }
}

class Printer extends Thread{
    private PrintManagementCenter printManagementCenter;
    private int id;
    public Printer(int id, PrintManagementCenter printManagementCenter){
        this.id = id;
        this.printManagementCenter = printManagementCenter;
    }

    private void print(UserFile userFile) throws InterruptedException {
        if(userFile == null){
            return;
        }
        System.out.println("Printer#" + this.id +
                " is printing UserFile#" + userFile.fileId +
                " for User#" + userFile.ownerId);
        Thread.sleep(userFile.timeCost);
    }

    @Override
    @SuppressWarnings("InfiniteLoopStatement")
    public void run(){
        // 8. Printer Loop +10
        while (!Thread.interrupted()) {
            try{
                UserFile file = this.printManagementCenter.Download();
                print(file);
                Thread.sleep(1000);
            }
            catch(InterruptedException e){
                break;
            }
        }
    }
}
