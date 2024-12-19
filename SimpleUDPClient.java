
import java.net.*;
import java.io.*;

public class SimpleUDPClient {
    public static void main(String[] args) {
        String hostname = "localhost"; // Server address
        int port = 12345; // Server port
        byte[] buffer = new byte[1024];

        try (DatagramSocket socket = new DatagramSocket()) {
            InetAddress serverAddress = InetAddress.getByName(hostname);
            BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in));
            String message;

            System.out.println("Enter messages to send to the server (type 'exit' to quit):");
            while (!(message = consoleReader.readLine()).equalsIgnoreCase("exit")) {
                // Send message
                byte[] messageData = message.getBytes();
                DatagramPacket packet = new DatagramPacket(messageData, messageData.length, serverAddress, port);
                socket.send(packet);

                // Receive response
                DatagramPacket responsePacket = new DatagramPacket(buffer, buffer.length);
                socket.receive(responsePacket);
                String response = new String(responsePacket.getData(), 0, responsePacket.getLength());
                System.out.println(response);
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}

