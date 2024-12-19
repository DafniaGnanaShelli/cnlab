
import java.io.*;
import java.net.*;

public class TCPChatClient {
    public static void main(String[] args) {
        String hostname = "localhost"; // Server address
        int port = 12345; // Server port

        try (Socket socket = new Socket(hostname, port)) {
            BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter writer = new PrintWriter(socket.getOutputStream(), true);

            BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in));

            Thread readThread = new Thread(() -> {
                try {
                    String message;
                    while ((message = reader.readLine()) != null) {
                        System.out.println("Server: " + message);
                        if (message.equalsIgnoreCase("exit")) {
                            System.out.println("Chat ended by server.");
                            break;
                        }
                    }
                } catch (IOException e) {
                    System.out.println("Connection closed.");
                }
            });

            readThread.start();

            String text;
            while ((text = consoleReader.readLine()) != null) {
                writer.println(text);
                if (text.equalsIgnoreCase("exit")) {
                    System.out.println("Chat ended.");
                    break;
                }
            }

            socket.close();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }
}

