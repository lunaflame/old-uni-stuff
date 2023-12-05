package Connections;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Random;

public class Negotiator {
	static private boolean isHex(String str) {
        for (char c: str.toCharArray()) {
        	if (!(c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
        		return false;
        	}
        }
        
        return true;
    }
	
	static Random why = new Random();
	static final int PeerIDLength = 20;
	static final String ProtocolString = "BitTorrent protocol";
	static final String PeerID = "-BT0000-";
	static final int ReservedLength = 8;
	static final int InfoHashLength = 20;
	static final byte[] Reserved = (new String("\000").repeat(8).getBytes());
	
	// ` the recipient must respond as soon as it sees the info_hash part of the handshake `
	// ` (the peer id will presumably be sent after the recipient sends its own handshake) `

	public static byte[] validateResponse(byte[] resp) {
		byte len = resp[0];
		
		// unacceptable: data makes no sense
		if (len > (resp.length - 1) || len == 0) {
			throw new IllegalArgumentException( String.format(
					"Invalid handshake received (received header length '%d' makes no sense)",
					len
				));
		}
		
		if (resp.length < (49 + len - PeerIDLength)) { // we dont need PeerID _yet_
			throw new IllegalArgumentException( String.format(
				"Invalid handshake received (expected length of at least %d; received %d)",
				49 + len - PeerIDLength,
				resp.length
			));
		}
		
		String pName = new String(Arrays.copyOfRange(resp, 1, ProtocolString.length() + 1));
		
		if (!pName.equals(ProtocolString)) {
			return null; // acceptable failure; we just dont support this protocol
		}
		
		// ignore reserved bytes, just extract the info hash
		return Arrays.copyOfRange(resp, 1 + len + ReservedLength,
				1 + len + ReservedLength + InfoHashLength);
	}
	
	public static ByteBuffer getHandshake(byte[] sha) {
		assert sha.length == 20;
		
		ByteArrayOutputStream buf = new ByteArrayOutputStream();
		
		// https://wiki.theory.org/BitTorrentSpecification#Handshake
		// #1: length of #2
		// #2: protocol name
		// #3: reserved 8bytes of 0's
		// #4: SHA1 from the metainfo
		// #5: peerID (formatted as "-CCnnnn-..." usually,
		//     where CC is a client code and `nnnn` is the version number,
		//     and ... are random numbers that make the total length of the string to 20 (am good at explain yes?)
		
		try {
			buf.write((byte)ProtocolString.length()); 	// #1
			buf.write(ProtocolString.getBytes());		// #2
			buf.write(Reserved);						// #3
			buf.write(sha);								// #4
			buf.write(PeerID.getBytes());				// #5
			
			for (int i = 0; i < (PeerIDLength - PeerID.length()); i++) {
				buf.write((byte)(why.nextInt(0, 9) + '0'));
			}
		} catch (Exception ex) {
			System.out.println("How\n\n\nBottom Text"); // literally how does this happen
			ex.printStackTrace();
			return null;
		}
		
		return ByteBuffer.wrap(buf.toByteArray());
	}
	
}