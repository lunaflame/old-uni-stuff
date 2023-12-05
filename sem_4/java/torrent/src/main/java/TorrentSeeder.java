package main.java;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.BitSet;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.dampcake.bencode.Bencode;
import com.dampcake.bencode.Type;

import Connections.Negotiator;
import main.java.TorrentUtils.Message;

public class TorrentSeeder {
	List<SocketChannel> acceptedClients = new ArrayList<>(); // clients with whom we already handshaked
	List<SocketChannel> hsClients = new ArrayList<>(); // clients on whose handshake we're waiting
	
	Map<Integer, Boolean> availablePieces = new HashMap<>();
	Map<SocketChannel, Boolean> needHSSent = new HashMap<>();
	
	static final Bencode bencode = new Bencode(true);
	
	static int DefaultPieceSize = 1 << 16;
	
	public enum ExitStatus {
		NOFILE,
		NOSOCKET,
		NOMETAINFO,
		OK,
	}
	
	public void addAvailablePiece(int num) {
		availablePieces.put(num, true);
		//System.out.println("    seeder: made #" + num + " available");
		
		byte[] bruh = ByteBuffer.allocate(4)
				.putInt(num)
				.array();
		
		acceptedClients.forEach((SocketChannel ch) -> {
			try {
				TorrentUtils.sendMessage(TorrentUtils.Type.NEWPIECE, bruh, ch);
			} catch (IOException e) {
				e.printStackTrace();
			}
		});
	}
	
	public void onFileConfirmed(ArrayList<Integer> goodPieces) {
		
	}
	
	public static byte[] encFile(String fn) throws IOException {
		Path path = Paths.get(fn);
		byte[] fBytes;

		try {
			fBytes = Files.readAllBytes(path);
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}

		HashMap<String, Object> benmap = new HashMap<String, Object>();
		HashMap<String, Object> info = new HashMap<String, Object>();
		
		int pieces = (int)Math.ceil((double)fBytes.length / DefaultPieceSize);
		ByteArrayOutputStream bout = new ByteArrayOutputStream(pieces * 20);
		
		MessageDigest crypt = null;
		
		try {
			crypt = MessageDigest.getInstance("SHA-1");
		} catch (NoSuchAlgorithmException e) { } // why is this an exception even
		
		for (int i = 0; i < fBytes.length; i += DefaultPieceSize) {
			crypt.reset();
			crypt.update(fBytes, i, Math.min(DefaultPieceSize, fBytes.length - i));
			
			bout.writeBytes(crypt.digest());
		}
		
		info.put("pieces", ByteBuffer.wrap(bout.toByteArray()));
		info.put("length", fBytes.length);
		info.put("name", path.getFileName().toString());
		info.put("piece length", DefaultPieceSize);

		benmap.put("info", info);
		benmap.put("created by", "me :)");
		benmap.put("creation date", 0); // ew
		
		byte[] out = bencode.encode(benmap);
		Path outPath = Paths.get(fn + ".torrent");
		Files.write(outPath, out);

		byte[] infoBytes = bencode.encode(info);

		return TorrentUtils.getSHA1(infoBytes); // we will use the SHA of info for our handshake
	}

	private void acceptNewConnection(SocketChannel con) throws IOException {
		System.out.println("New client: accepting...");
		con.configureBlocking(false);
		hsClients.add(con); // only add at the end so we know it didnt throw
	}
	
	private byte[] getFilePieces(RandomAccessFile fl, int start, int end) throws IOException {
		
		// check if we actually have the pieces
		for (int i = start; i < end; i++) {
			if (!availablePieces.containsKey(i)) {
				return null; // leecher requested piece we don't actually have; silently ignore
			}
		}
		
		fl.seek(start * getPieceSize());
		
		int endPos = (int)Math.min(end * getPieceSize(), fl.length());
		int startPos = (int)Math.min(endPos, start * getPieceSize());
		int toRead = endPos - startPos;
		if (toRead <= 0) { return new byte[0]; }
		
		byte[] buf = new byte[toRead];
		fl.readFully(buf, 0, toRead);
		
		System.out.println("sending file piece " + start * getPieceSize() + "+" + endPos + " >" + toRead);
		// System.out.println("first bytes: " + new String(buf).substring(0, 16));
		
		return buf;
	}
	
	private boolean readHandshake(byte[] ourSHA, SocketChannel con) throws IOException {
		if (!con.finishConnect()) {
			return false; // not ok; just wait
		}

		ByteBuffer buf = ByteBuffer.allocate(64); // cry about it
		
		int read = -1;
		
		try {
			read = con.read(buf);
		} catch (SocketException e) {
			System.out.println("Socket exception for " + con.getRemoteAddress());
			return false;
		}
		
		if (read == -1) {
			// errer
			con.close();
			hsClients.remove(con);
			System.out.println("read -1?");
			return false;
		}

		byte[] sha = Negotiator.validateResponse(buf.array());
		if (!Arrays.equals(sha, ourSHA)) {
			// eff you, hashes are different
			con.close();
			hsClients.remove(con);
			System.out.println("different info hashes; denying. (" + TorrentUtils.SHA1toHex(sha) + " vs. " + TorrentUtils.SHA1toHex(ourSHA) + ")");
			System.out.println(new String(buf.array()));
			return false;
		}
		
		System.out.println("Correct handshake from " + con.getRemoteAddress() + "; accepting...");
		
		hsClients.remove(con);
		acceptedClients.add(con);
		needHSSent.put(con, true);
		return true;
	}
	
	private void sendHandshake(SocketChannel cl, byte[] SHA) throws IOException {
		// send handshake
		ByteBuffer buf = Negotiator.getHandshake(SHA);
		cl.write(buf);
		System.out.println("responded with our own handshake");
		needHSSent.remove(cl);
		
		// immediately also send our availability data
		BitSet availDat = new BitSet();
		for (int piece : availablePieces.keySet()) {
			availDat.set(piece, true);
		}
		
		System.out.println("also sending availability (" + availDat.cardinality() + " pieces available)");
		
		new java.util.Timer().schedule( 
		        new java.util.TimerTask() {
		            @Override
		            public void run() {
		            	try {
							TorrentUtils.sendMessage(TorrentUtils.Type.AVAILABILITY, availDat.toByteArray(), cl);
						} catch (IOException e) {
							e.printStackTrace();
						}
		            }
		        }, 
		        500 
		);
		
	}
	
	public HashMap<String, Object> readTorrent(String fn) {
		Path path = Paths.get(fn);
		byte[] fBytes;

		try {
			fBytes = Files.readAllBytes(path);
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
		
		Map<String, Object> dict = bencode.decode(fBytes, Type.DICTIONARY);
		HashMap<String, Object> info = (HashMap<String, Object>) dict.get("info");
	    if (info == null) {
	    	throw new IllegalArgumentException("Torrent file didn't contain info.");
	    }
	    
	    // see which pieces we actually have
	    String pieceFn = new String( ((ByteBuffer)info.get("name")).array() );
	    Path tpath = path.getParent()
	    		.resolve("files")
	    		.resolve(pieceFn);
	    

	    try {
	    	File bro = tpath.toFile();
	    	bro.getParentFile().mkdirs();
	    	bro.createNewFile();
	    }
	    catch (FileAlreadyExistsException ignore) { }
	    catch (IOException e) {
	    	System.out.println("Failed to create file for seeding: " + e.getMessage());
			e.printStackTrace();
		}
		
	    RandomAccessFile lefiel;
	    
		try {
			lefiel = new RandomAccessFile(tpath.toString(), "r");
		} catch (IOException e) {
			System.out.println("Failed to open file for seeding: " + e.getMessage());
			return null;
		}
	    
	    FileSize = ((Long)info.get("length")).intValue();
	    Pieces = (int)Math.ceil((double)FileSize / ((Long)info.get("piece length")).intValue());

	    int pl = ((Long)info.get("piece length")).intValue();
	    ArrayList<Integer> correctPieces;
	    
	    try {
	    	correctPieces = TorrentUtils.collectPiecesFromFile(lefiel.getChannel(), (ByteBuffer)info.get("pieces"), getPiecesAmt(), pl);

	    	for (int pc : correctPieces) {
	    		addAvailablePiece(pc);
	    	}
	    } catch (IOException ex) {
	    	System.out.println("Failed to retrieve correct pieces for seeding (somehow): " + ex.getMessage());
	    	return null;
	    }
	    
	    onFileConfirmed(correctPieces);
	    
	    return info;
	}
	
	public ExitStatus start(int port, String tfn) throws IOException {
		Thread.currentThread().setName("Torrent - Seeding");
		System.out.println("seeding: " + tfn);

		HashMap<String, Object> info = readTorrent(tfn);
		
		if (info == null) {
			return ExitStatus.NOMETAINFO;
		}
		
		byte[] infoBytes = bencode.encode(info);
	    byte[] OSHA = TorrentUtils.getSHA1(infoBytes);
	
		String fn = new String( ((ByteBuffer)info.get("name")).array() );
		ServerSocketChannel serverChannel;
		Selector selector;
		
		Path tpath = Path.of(tfn).getParent().resolve("files");
		
		RandomAccessFile lefiel = null;
		try {
			lefiel = new RandomAccessFile(tpath.resolve(fn).toString(), "r");
		} catch (FileNotFoundException ex) {
			return ExitStatus.NOFILE;
		}
		
		try {
			serverChannel = ServerSocketChannel.open();
			serverChannel.socket().bind(new InetSocketAddress(port));
			serverChannel.configureBlocking(false);
			selector = Selector.open();
			serverChannel.register(selector, SelectionKey.OP_ACCEPT);
		} catch (IOException e) {
			System.out.println("Seeder - couldnt bind to socket");
			e.printStackTrace();
			if (lefiel != null) {
				lefiel.close();
			}
			return ExitStatus.NOSOCKET;
		}

		while (true) {
			selector.select();
			Iterator<SelectionKey> iter = selector.selectedKeys().iterator();

			while (iter.hasNext()) {
				SelectionKey key = iter.next();
				if (!key.isValid()) {
					continue;
				}

				/*
				 * Accepting a connection
				 */
				if (key.isAcceptable())
					AcceptingState: {
						SocketChannel newCl = serverChannel.accept();
						if (newCl == null) {
							break AcceptingState;
						} // exit early

						acceptNewConnection(newCl);
						newCl.register(selector, SelectionKey.OP_READ);
						//continue;
					}

				/*
				 * Reading from a connection (= performing handshake) if they fail, we yeet them
				 */

				if (key.isReadable()) ReadState: {
					// got something to read

					SocketChannel cl = (SocketChannel) key.channel();
					
					HandshakeState: {
						if (!hsClients.contains(cl)) { // cant handshake if we're not expecting it
							break HandshakeState;
						}
						
						boolean ok = false;
						try {
							ok = readHandshake(OSHA, cl);
						} catch (Exception ex) {
							ex.printStackTrace();
							hsClients.remove(cl);
						}
	
						if (!ok) { // thrown connection out; dont try to do anything else
							continue;
						}
						
						cl.register(selector, SelectionKey.OP_WRITE);
						continue;
					}
					
					// past handshake we communicate using types
					Message[] msges = TorrentUtils.recvMessage(cl);
					if (msges == null) { break ReadState; }
					
					for (Message msg : msges) {
						if (msg.typ == TorrentUtils.Type.REQUEST)
							PieceRequestState: {
								if (!acceptedClients.contains(cl)) { // cant request if we didnt handshake
									break PieceRequestState;
								}
								
								ByteBuffer pcDat = ByteBuffer.wrap(msg.cont);
								
								int start = pcDat.getInt(), end = pcDat.getInt();
	
								if (lefiel != null) {
									byte[] fileCont = getFilePieces(lefiel, start, end);
									if (fileCont == null) {
										System.out.println("File piece denied " + start + "->" + end);
										break PieceRequestState;
									}
									TorrentUtils.sendMessage(TorrentUtils.Type.PIECE, fileCont, cl);
								}
							}
					}
				}

				/*
				 * Writing to a connection (ie the actual seeding process)
				 */

				if (key.isWritable()) {
					SocketChannel cl = (SocketChannel) key.channel();
					
					// we still need to respond with a handshake
					if (needHSSent.get(cl) != null) {
						sendHandshake(cl, OSHA);
						continue;
					}
				}
			}
			
			try {
				Thread.sleep(10);
			} catch (InterruptedException e) {
				System.out.println("Can i at least sleep without a FUCKING exception jesus christ");
			}
		}
	}
	
	
	public int getPieceSize() {
		return PieceSize;
	}

	public int getPiecesAmt() {
		return Pieces;
	}

	private int PieceSize = DefaultPieceSize;
	private int FileSize = 0;
	private int Pieces = 0;
}
