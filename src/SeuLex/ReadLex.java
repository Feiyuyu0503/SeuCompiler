package SeuLex;

import java.io.*;

public class ReadLex {
    /**
    public static void main(String[] args) throws IOException {
        Path path = Paths.get("src/SeuLex/minic.l");
        List<String> lines = Files.readAllLines(path);
        //System.out.println("1");
        System.out.println(lines.get(0));
    }
     **/

    private String filePath;
    private BufferedReader input;

    public ReadLex(String filePath)
    {
        this.filePath = filePath;
        prepareToRead();
    }

    private boolean prepareToRead()
    {
        try {
            input = new BufferedReader(new InputStreamReader(new FileInputStream(this.filePath)));
            return input.ready();
        } catch (IOException e) {
            return false;
        }
    }

    public String readLine()
    {
        try {
            return input.readLine();
        } catch (IOException e){
            release();
            return null;
        }
    }

    public void release()
    {
        try {
            if (input != null)
            {
                input.close();
            }
        } catch (IOException e){
            System.out.println(e);
        }
    }

}
