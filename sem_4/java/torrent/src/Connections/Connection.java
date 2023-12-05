package Connections;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.Arrays;

public class Connection {
	private SocketChannel sock = null;
	private ByteBuffer buf = ByteBuffer.allocate(512);

	public Connection(String ip, int port) throws UnknownHostException, IOException {
		setSocket(SocketChannel.open(new InetSocketAddress(ip, port)));
	}
	
	public void writeBytes(byte[] bytes) throws IOException {
		getSocket().write(ByteBuffer.wrap(bytes)); // i am dead
	}
	
	public void writeLn(String str) throws IOException {
		getSocket().write(ByteBuffer.wrap(str.getBytes()));
	}
	
	public String readLn() throws IOException {
		buf.clear();
		int read = getSocket().read(buf);
		if (read == -1) {
			return null;
		}
		
		if (read == 0) {
			return "";
		}
		
		System.out.println("readLn: recv " + read);
		return new String(buf.array()).trim();
	}
	
	public byte[] read() throws IOException {
		int read = getSocket().read(buf);
		if (read == -1) {
			return null;
		}
		
		return Arrays.copyOf(buf.array(), read);
	}

	public SocketChannel getSocket() {
		return sock;
	}

	public void setSocket(SocketChannel sock) {
		this.sock = sock;
	}
}
