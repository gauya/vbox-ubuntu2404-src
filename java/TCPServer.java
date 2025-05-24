import java.io.*;
import java.net.*;

public class TCPServer {
    public static void main(String[] args) {
        final int port = 5000;

        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("서버 시작됨. 포트: " + port);

            while (true) {
                Socket clientSocket = serverSocket.accept();
                System.out.println("클라이언트 연결됨: " + clientSocket.getInetAddress());

                BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true);

                String inputLine;
                while ((inputLine = in.readLine()) != null) {
                    System.out.println("받은 메시지: " + inputLine);
                    out.println("서버 응답: " + inputLine);
                }

                clientSocket.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
