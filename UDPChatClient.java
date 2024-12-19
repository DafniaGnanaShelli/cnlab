import java.net.*;
import java.io.*;

public class UDPChatClient {
    public static void main(String[] args) {
        String hostname = "localhost"; // Server address
        int port = 12345; // Server port
        byte[] buffer = new byte[1024];

        try (DatagramSocket socket = new DatagramSocket()) {
            InetAddress serverAddress = InetAddress.getByName(hostname);

            Thread receiveThread = new Thread(() -> {
                try {
                    while (true) {
                        DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                        socket.receive(packet);
                        String received = new String(packet.getData(), 0, packet.getLength());
                        System.out.println("Server: " + received);
                        if (received.equalsIgnoreCase("exit")) {
                            System.out.println("Chat ended by server.");
                            break;
                        }
                    }
                } catch (Exception e) {
                    System.out.println("Connection closed.");
                }
            });

            receiveThread.start();

            BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in));
            while (true) {
                String message = consoleReader.readLine();
                byte[] data = message.getBytes();
                DatagramPacket packet = new DatagramPacket(data, data.length, serverAddress, port);
                socket.send(packet);
                if (message.equalsIgnoreCase("exit")) {
                    System.out.println("Chat ended.");
                    break;
                }
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}

