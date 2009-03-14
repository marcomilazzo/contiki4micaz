import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.UnknownHostException;


public class Main {

	public static void main(String[] args) {
		if(args.length != 2) {
			System.out.println("Error : Usage \n");
			System.out.println("        <ip address> <port>\n");
			System.exit(1);
		}
		Socket clientSocket;
		InputStream in;
		try {
			
			clientSocket = new Socket(args[0].trim(), Integer.parseInt(args[1].trim()));
			in = clientSocket.getInputStream();
			
			byte data[] = new byte[1];
			for(;;){
				int len = in.read(data, 0, 1);
				if(len > 0)
					System.out.print((char)data[0]);
				else
				{
					in.close();
					break;
				}
			}
		
		} catch (UnknownHostException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
			

	}

}
