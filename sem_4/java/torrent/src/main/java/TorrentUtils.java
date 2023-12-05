package main.java;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.SocketChannel;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Formatter;
import java.util.HashMap;

public class TorrentUtils {
	private static MessageDigest crypt;

	static {
		// i dont have a better way to do this this is extremely lame
		try {
			crypt = MessageDigest.getInstance("SHA-1");
		} catch (NoSuchAlgorithmException e) {
			System.out.println("this is literally not supposed to happen under any circumstance, wtf");
			e.printStackTrace();
		}
	}
	
	public static synchronized byte[] getSHA1(byte[] arr, int len) {
		crypt.reset();
		crypt.update(arr, 0, len);
		
		return crypt.digest();
	}
	
	public static byte[] getSHA1(byte[] arr) {
		return getSHA1(arr, arr.length);
	}
	
	public static String SHA1toHex(byte[] sha) {
		Formatter formatter = new Formatter();
		for (byte b : sha) {
			formatter.format("%02x", b);
		}
		
		String hex = formatter.toString();
		formatter.close();
		
		return hex;
	}
	
	public static void reset() {
		crypt.reset();
	}
	
	public static void addBytes(byte[] arr) {
		crypt.update(arr);
	}
	
	private static ByteBuffer bb = ByteBuffer.allocate(1024);
	
	private static void realloc(int atleast, boolean saveContent) {
		if (bb.capacity() < atleast) {
			int newSize = Math.max(atleast, bb.capacity() * 2);
			
			if (saveContent) {
				byte[] curCont = Arrays.copyOf(bb.array(), bb.array().length);
				int sz = bb.position();
				bb = ByteBuffer.allocate(newSize);
				bb.put(curCont, 0, sz);
			} else {
				bb = ByteBuffer.allocate(newSize);
			}
		}
	}
	
	public static enum Type {
		NEWPIECE (4),
		REQUEST (5),
		PIECE (6),
		AVAILABILITY (7),
		
		;
		
		public final byte num;
		private Type (int n) {
			num = (byte)n;
		}
	}
	
	public static synchronized void sendMessage(Type type, byte[] cont, SocketChannel sock) throws IOException {
		realloc(cont.length + 5, false);
		
		bb.clear();
		bb.putInt(cont.length);
		bb.put((byte)type.num);
		bb.put(cont);
		
		bb.flip();

		sock.write(bb);
		//System.out.println("sending " + bb.limit() + "bytes to " + sock.getRemoteAddress());
		bb.limit(bb.capacity());
	}
	
	public static class Message {
		Type typ;
		byte[] cont;
	}
	
	public static ArrayList<Integer> collectPiecesFromFile(FileChannel fl, ByteBuffer pieces,
			int pieceAmt, int pieceSize) throws IOException {
		
		long fLength = fl.size();
		
		ByteBuffer bbuf = ByteBuffer.allocate(pieceSize);
		
		byte[] shaBuf = new byte[20]; // buf for reading every piece SHA from the torrent file
		
		ArrayList<Integer> out = new ArrayList<>(pieceAmt);
		System.out.println("getting pieces " + pieceAmt);
		
		for (int i = 0; i < pieceAmt; i++) {
    		if (fLength <= i * pieceSize) {
    			break; // EOF; obviously cant have any more pieces...
    		}
    		
    		bbuf.rewind();
	    	int read = fl.read(bbuf, i * pieceSize);
	    	
	    	byte[] sha = TorrentUtils.getSHA1(bbuf.array(), read);
	    	pieces.get(i * 20, shaBuf, 0, 20);
	    	
	    	// shaBuf contains SHA from the .torrent
	    	// sha contains SHA from the actual file
	    	if (Arrays.equals(shaBuf, sha)) {
	    		out.add(i);
	    	}
	    }
		
		return out;
	}
	
	public static synchronized Message[] recvMessage(SocketChannel sock) throws IOException {
		bb.clear();
		ArrayList<Message> queue = new ArrayList<>();
		
		sock.socket().setSoTimeout(500);
		int len = sock.read(bb);
		if (len == 0) { return null; }
		
		// prints here
		if (len <= 4) {
			return null;
		}

		// System.out.println("recv msg: len is " + len);
		bb.position(0);
		
		
		// try to split this huge chunk of SHIT into a bunch of messages
		int leftUnsplit = len - 5;
		
		while (leftUnsplit > 0) {
			int contLen = bb.getInt(); // how long is the current message?

			byte btyp = bb.get(); // message type
			Type typ = null;
			
			for (Type t : Type.values()) {
				// cry about it
				if (btyp == t.num) { typ = t; break; } 
			}
			
			if (typ == null) {
				System.out.println("Unrecognized type enum from %s (%s); ignoring message".formatted(sock.getRemoteAddress(), btyp));
				System.out.println("	(length: " + contLen + ")");
				return null;
			}
			
			// if the message length is more than what we've read, then
			// we're probably missing something... keep reading more
			if (len < contLen) {
				while (true) {
					bb.position(len);
					realloc(contLen + 5, true);
					
					int readMore = sock.read(bb);
					
					if (readMore <= 0) { break; }
					len += readMore;
					leftUnsplit += readMore;
					if (len >= contLen + 5) { break; }
				}
			}
			
			leftUnsplit -= contLen;
			//System.out.println("Read msg #%s, left unsplit: %s".formatted(queue.size(), leftUnsplit));
			
			Message ret = new Message();
			ret.cont = Arrays.copyOfRange(bb.array(), 5, 5 + contLen); // TODO: this fucking sucks
			ret.typ = typ;
			
			queue.add(ret);
		}
		
		return queue.toArray(new Message[0]);
	}
}
