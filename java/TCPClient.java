import java.io.*;
import java.net.*;

public class TCPClient {
    public static void main(String[] args) {
        final String serverIP = "localhost"; // 또는 서버 IP 주소
        final int port = 5000;

        try (Socket socket = new Socket(serverIP, port);
             BufferedReader keyboard = new BufferedReader(new InputStreamReader(System.in));
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true)) {

            System.out.println("서버에 연결됨. 메시지를 입력하세요:");

            String userInput;
            while ((userInput = keyboard.readLine()) != null) {
                out.println(userInput); // 서버로 전송
                String response = in.readLine(); // 서버 응답 수신
                System.out.println("서버 응답: " + response);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
