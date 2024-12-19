
import java.net.*;
import java.io.*;

public class UDPChatServer {
    public static void main(String[] args) {
        int port = 12345; // Server port
        byte[] buffer = new byte[1024];

        try (DatagramSocket socket = new DatagramSocket(port)) {
            System.out.println("Server is listening on port " + port);

            // Use mutable containers to store client address and port
            final InetAddress[] clientAddress = {null};
            final int[] clientPort = {-1};

            Thread receiveThread = new Thread(() -> {
                try {
                    while (true) {
                        DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                        socket.receive(packet);

                        // Capture client address and port
                        InetAddress receivedAddress = packet.getAddress();
                        int receivedPort = packet.getPort();

                        String received = new String(packet.getData(), 0, packet.getLength());
                        System.out.println("Client: " + received);

                        // Save client address and port for future messages
                        if (clientAddress[0] == null || clientPort[0] == -1) {
                            clientAddress[0] = receivedAddress;
                            clientPort[0] = receivedPort;
                        }

                        // If the message is "exit", break the loop
                        if (received.equalsIgnoreCase("exit")) {
                            System.out.println("Chat ended by client.");
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

                // Send message to the client
                if (clientAddress[0] != null && clientPort[0] != -1) {
                    byte[] data = message.getBytes();
                    DatagramPacket packet = new DatagramPacket(data, data.length, clientAddress[0], clientPort[0]);
                    socket.send(packet);
                }

                // If the message is "exit", stop the chat
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

