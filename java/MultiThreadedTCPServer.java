import java.io.*;
import java.net.*;

public class MultiThreadedTCPServer {

    public static void main(String[] args) {
        int port = 5000;
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("서버 시작됨. 포트: " + port);

            while (true) {
                Socket clientSocket = serverSocket.accept();
                System.out.println("클라이언트 연결됨: " + clientSocket.getInetAddress());

                // 새 클라이언트를 위한 스레드 생성
                new ClientHandler(clientSocket).start();
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    // 클라이언트 처리를 위한 내부 클래스
    static class ClientHandler extends Thread {
        private final Socket socket;

        public ClientHandler(Socket socket) {
            this.socket = socket;
        }

        public void run() {
            try (
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true)
            ) {
                String inputLine;
                while ((inputLine = in.readLine()) != null) {
                    System.out.println("[" + socket.getInetAddress() + "] 메시지: " + inputLine);
                    out.println("서버 응답: " + inputLine);
                }
            } catch (IOException e) {
                System.out.println("클라이언트 연결 종료됨: " + socket.getInetAddress());
            } finally {
                try {
                    socket.close();
                } catch (IOException e) {
                    // 무시
                }
            }
        }
    }
}
