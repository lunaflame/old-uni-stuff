package main.java;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.BitSet;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.Callable;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Consumer;
import java.util.regex.Pattern;

import com.dampcake.bencode.Bencode;
import com.dampcake.bencode.Type;

import Connections.Connection;
import Connections.Negotiator;
import main.java.TorrentUtils.Message;

// 1 leecher per torrent file
public class TorrentLeecher {
	
	public static enum State {
		WAIT_HANDSHAKE, // we're currently handshaking with this socket
		WAIT_AVAILABILITY, // we're waiting on availability data from this socket (ie we have nothing to request)
		
		WAIT_READY, // ready to request; waiting for the event loop...
		WAIT_PIECE, // we requested a piece and are waiting on a reply
	}
	
	public State getSocketState(SocketChannel sock) {
		return socketStates.get(sock);
	}
	
	public void setSocketState(SocketChannel sock, State to) {
		socketStates.put(sock, to);
	}
	
	static final Bencode bencode = new Bencode(StandardCharsets.UTF_8, true);
	
	static final int PieceFetch = 1;
	
	static ThreadPoolExecutor executor =  (ThreadPoolExecutor) Executors.newFixedThreadPool(4);
	static ThreadPoolExecutor writer =  (ThreadPoolExecutor) Executors.newFixedThreadPool(1);
	
	public void onPieceReceived(byte[] cont, int num) {
		doWrite(cont, num, writeTo);
		donePieces.put(num, true);
		if (wantPieceList.get(num) != null) {
			wantPieceList.remove(num);
		}
	}
	
	public void addRequiredPiece(int num) {
		if (donePieces.get(num) == null) {
			wantPieceList.put(num, true);
		}
	}
	
	private synchronized void doWrite(byte[] dat, int piece, RandomAccessFile fl) {
		try {
			fl.seek(piece * pieceLength);
			fl.write(dat);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		int curPc = pieceWritten.addAndGet(PieceFetch);

		if (curPc == totalPieces) {
			try {
				fl.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	private void requestPiece(SocketChannel con) {
		int toGet = getPieceToRequest(con);
		pieceWaiting.put(con, toGet);
		if (toGet == -1) {
			setSocketState(con, State.WAIT_READY);
			pieceWaiting.put(con, null);
			return;
		}
		
		ByteBuffer bb = ByteBuffer.allocate(8);
		
		int need = toGet;
		bb.putInt(need);
		bb.putInt(Math.min(totalPieces, need + PieceFetch));

		
		try {
			System.out.println("Requesting piece #%s from %s".formatted(need, con.getRemoteAddress()));
			// request the pieces we need and start accepting
			TorrentUtils.sendMessage(TorrentUtils.Type.REQUEST, bb.array(), con);
			
			setSocketState(con, State.WAIT_PIECE);
		
			pieceResponse.put(con, (Message msg) -> {
				onPieceReceived(msg.cont, need);
				new java.util.Timer().schedule( 
				        new java.util.TimerTask() {
				            @Override
				            public void run() {
				            	requestPiece(con); // request the next piece afterwards
				            }
				        }, 
				        (int)(Math.random() * 400) 
				);
				
			});
			
		} catch (IOException e) {
			// e
			e.printStackTrace();
		}
	}
	
	private void onAvailabilityReceived(SocketChannel con) {
		System.out.println("Availability received... queueing requester.");
		executor.submit(() -> {
			var pieces = availability.get(con);
			if (pieces == null) { throw new NullPointerException("!? wtf"); }
			
			setSocketState(con, State.WAIT_READY);

			return null;
		});
	}
	
	private void onConnection(SocketChannel con) throws IOException {
		System.out.println("Established connection w/ " + con.getRemoteAddress());
		
		// setup data
		availability.put(con, new HashMap<> ());
		
		// wait for availability data so we know what pieces to request
		setSocketState(con, State.WAIT_AVAILABILITY);
	}

	private void onSocketClosed(SocketChannel chan) {
		var wait = pieceWaiting.get(chan);
		if (wait != null) {
			// we were waiting on a piece from this socket... put it back in the "want" list
			wantPieceList.put(wait, true);
		}
	}
	
	private boolean readFromSocket(SocketChannel chan) throws Exception {
		byte[] seededSHA;
		
		if (getSocketState(chan) == State.WAIT_HANDSHAKE) {
			ByteBuffer buf = ByteBuffer.allocate(512); // if 512 aint enough for your handshake you dont deserve cooperation
			int read = chan.read(buf);
			if (read == -1) { System.out.println("nothing..."); return true; }

			if (read < 20) {
				throw new Exception("Bad handshake (length: %s, expected: %s)".formatted(read, 20));
			}
			
			seededSHA = Negotiator.validateResponse(buf.array());
			if (!Arrays.equals(seededSHA, hsSHA)) {
				return false; // wrong SHA; not exception-worthy but we should yeet them
			}
			
			onConnection(chan);
			return true;
		}
		
		if (getSocketState(chan) == State.WAIT_READY) {
			// request a piece if we're ready
			requestPiece(chan);
		}
	
		// past the handshake state we use the message system
		
		Message[] msges = null;
		try {
			msges = TorrentUtils.recvMessage(chan);
		} catch (Exception e) {
			// probably socket closed, try cleaning up
			return false;
		}
		
		if (msges == null) { return true; }
		
		for (Message msg : msges) {
			// now handle what kind of message this is
			if (msg.typ == TorrentUtils.Type.PIECE) {
				// received piece, if we have a consumer: run it
				
				if (pieceResponse.get(chan) != null) {
					pieceResponse.get(chan).accept(msg);
				}
				
				return true;
			}
			
			if (msg.typ == TorrentUtils.Type.AVAILABILITY) {
				// received availability data; parse it out
				int expectLen = (int) Math.ceil(totalPieces / 8); 
				if (msg.cont.length < expectLen) {
					throw new Exception(
							"Incorrect availability data size received (size: %s, expected at least %s)"
							.formatted(msg.cont.length, expectLen)
						);
				}
				
				BitSet bs = BitSet.valueOf(msg.cont);
				bs.stream().forEach((int bitPos) -> {
					availability.get(chan).put(bitPos, true);
				});
				
				if (getSocketState(chan) == State.WAIT_AVAILABILITY) {
					setSocketState(chan, State.WAIT_READY);
					onAvailabilityReceived(chan);
				}
			}
			
			if (msg.typ == TorrentUtils.Type.NEWPIECE) {
				int piece = ByteBuffer.wrap(msg.cont).getInt();
				availability.get(chan).put(piece, true);
				System.out.printf("Channel %s announced new available piece %s\n", chan.getRemoteAddress(), piece);
			}
		}
		
		return true;
	}
	
	private void writeToSocket(SocketChannel chan) {

	}

	public void start(String torrent, List<String> svs) throws IOException {
		Thread.currentThread().setName("Torrent - Client");
		System.out.println("dl'ing torrent " + torrent);
		for (String s : svs) {
			System.out.println(" from " + s);
		}
		
		Path path = Paths.get(torrent);
		byte[] read;
	
	    try {
			read = Files.readAllBytes(path);
		} catch (IOException e) {
			System.out.println("Failed to read file: " + path + ". Exiting.");
			return;
		}
	    
	    Map<String, Object> dict = bencode.decode(read, Type.DICTIONARY);
	    metamap = dict;
	    
	    HashMap<String, Object> info = (HashMap<String, Object>) dict.get("info");
	    if (info == null) {
	    	throw new IllegalArgumentException("Torrent file didn't contain info.");
	    }
	    
	    byte[] infoBytes = bencode.encode(info);
	    
	    hsSHA = TorrentUtils.getSHA1(infoBytes);
	    
		pieceLength = ((Long)info.get("piece length")).intValue();
		totalPieces = (int)Math.ceil(
			(double)
				((Long)info.get("length")).intValue() / pieceLength
		);
		
	    // initialize sockets for each server
	    for (String addr : svs) {
	    	var m = ipPortRx.matcher(addr);
	    	String ip = null;
	    	int port = 0;
	    	while (m.find()) {
	    		ip = m.group(1);
	    		port = Integer.parseInt(m.group(2));
	    	}
	    	
	    	if (ip == null) {
	    		System.out.println("Failed to parse IP " + addr);
	    		continue;
	    	}
	    	
	    	try {
				socks.put(addr, new Connection(ip, port));
			} catch (UnknownHostException e) {
				System.out.println("Unknown host exception caught for address " + addr);
				continue;
			} catch (IOException e) {
				System.out.println("Couldn't open socket for " + addr + ": " + e.getMessage());
				continue;
			}
	    }

	    String fPath = path.getParent()
	    		.resolve("files")
	    		.resolve(new String( ((ByteBuffer)info.get("name")).array() ))
	    		.toString();
	    
	    RandomAccessFile flIn = new RandomAccessFile(fPath, "rw");
	    writeTo = flIn;
	   
	    // handshake with everyone
	    byte[] hs = Negotiator.getHandshake(hsSHA).array();
	   
	    Selector selector = Selector.open();
	    
	    socks.forEach((addr, con) -> {
	    	try {
	    		con.getSocket().configureBlocking(false);
				con.getSocket().register(selector, SelectionKey.OP_WRITE | SelectionKey.OP_READ);
			} catch (IOException e) {
				e.printStackTrace();
			}
	    	
	    	try {
				con.writeBytes(hs); // send our handshake to them and wait for a response in the selector
				setSocketState(con.getSocket(), State.WAIT_HANDSHAKE);
			} catch (IOException e) {
				e.printStackTrace(); // i dont like java
			} 
	    });
	    
	    while (true) {
			selector.select();
			Iterator<SelectionKey> iter = selector.selectedKeys().iterator();

			while (iter.hasNext()) {
				SelectionKey key = iter.next();
				if (!key.isValid()) {
					continue;
				}
				
				SocketChannel chan = (SocketChannel) key.channel();
				
				if (key.isReadable()) {
					try {
						boolean ok = readFromSocket(chan);
						
						if (!ok) { // request close
							System.out.println("-- Reader requested disconnect. --");
							onSocketClosed(chan);
							chan.close();
							key.cancel();
							break;
						}
					} catch (Exception e) {
						System.out.println("-- Exception while reading from socket; disconnecting. --");
						e.printStackTrace();
						onSocketClosed(chan);
						chan.close();
						key.cancel();
						break;
					}
				}
				
				if (key.isWritable()) {
					try {
						writeToSocket(chan);
					} catch (Exception e) {
						System.out.println("-- Exception while writing to socket; disconnecting. --");
						onSocketClosed(chan);
						chan.close();
						key.cancel();
						break;
					}
				}
			}
	    }
	}
	
	private synchronized int getPieceToRequest(SocketChannel chan) {
		var available = availability.get(chan);
		
		Iterator<Entry<Integer, Boolean>> iter = wantPieceList.entrySet().iterator();
		
		while (iter.hasNext()) {
			Entry<Integer, Boolean> num = iter.next();

			if (available.get(num.getKey()) != null) {
				iter.remove();
				return num.getKey();
			}
		}
	
		return -1;
	}
	
	private int totalPieces = 0;
	private int pieceLength = 0;
	private byte[] hsSHA; // SHA we'll use for the handshake
	private Map<String, Object> metamap;
	private RandomAccessFile writeTo;
	
	// [address_string] = Connection
	private HashMap<String, Connection> socks = new HashMap<>();
	
	// [Channel] = ThingToDoWhenPieceReplyArrives
	private HashMap<SocketChannel, Consumer/*of the cum chalice*/<Message>> pieceResponse = new HashMap<>();
	
	// [Channel] = WhichPieceWaitingOn
	private HashMap<SocketChannel, Integer> pieceWaiting = new HashMap<>();
	
	// [Channel] = { [0] = true, [1] = true, ... }
	// basically tracks what channel has what pieces that we can request
	private HashMap<SocketChannel, HashMap<Integer, Boolean>> availability = new HashMap<>();
	
	private HashMap<Integer, Boolean> wantPieceList = new HashMap<>();
	private HashMap<Integer, Boolean> donePieces = new HashMap<>();
	
	// [Channel] = State
	private HashMap<SocketChannel, State> socketStates = new HashMap<>();
	
	static Pattern ipPortRx = Pattern.compile("(\\d{1,3}?\\.\\d{1,3}?\\.\\d{1,3}?\\.\\d{1,3}?):(\\d{5})");	
	
	static AtomicInteger pieceWritten = new AtomicInteger(0); // the latest written piece
	
}
