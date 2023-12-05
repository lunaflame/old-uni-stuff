package main.java;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import main.java.TorrentSeeder.ExitStatus;

// a swarm here means a single .torrent file's handler
// ie it handles both the leeching and seeding part for a certain .torrent
// this is to allow leecher and seeder to communicate with each other without them knowing about each other

public class TorrentSwarm {
	private TorrentLeecher leecher = new TorrentLeecher() {
		@Override
		public void onPieceReceived(byte[] cont, int num) {
			onPieceDownloaded(cont, num);
			super.onPieceReceived(cont, num);
		}
	};
	
	private TorrentSeeder seeder = new TorrentSeeder() {
		@Override
		public void onFileConfirmed(ArrayList<Integer> pieces) {
			onSeederConfirmed(pieces);
		}
	};
	
	private List<String> servers;
	private String metapath;
	
	public void start(String metainfo, int port, List<String> servers) throws IOException {
		this.servers = servers;
		this.metapath = metainfo;
		
		// start a thread for the seeder
		new Thread() {
			public void run() {
				ExitStatus why = null;
				try {
					why = seeder.start(port, metainfo);
				} catch (IOException e) {
					e.printStackTrace();
				}
				
				onSeederExit(why);
			}
		}.start();
		
		// we'll start the leecher after the seeder does something (either exits
		// 	or confirms correct pieces)
	}
	
	private void startLeecher() {
		// we might not even want the leecher if we weren't given any servers
		if (servers.size() > 0) {
			// start a thread for the leecher
			new Thread() {
				public void run() {
					try {
						leecher.start(metapath, servers);
					} catch (IOException e) {
						e.printStackTrace(); // what the cringe
					}
				}
			}.start();
		} else {
			System.out.println("No server IPs provided - seeding only.");
		}
	}
	
	private void onSeederExit(ExitStatus why) {
		// seeder can't exit by itself, only if it exited early without actually seeding anything
		
		if (why == ExitStatus.NOFILE || why == ExitStatus.NOMETAINFO) {
			for (int i = 0; i < seeder.getPiecesAmt(); i++) {
				leecher.addRequiredPiece(i);
			}
		}
		
		startLeecher();
	}
	
	protected void onSeederConfirmed(ArrayList<Integer> pieces) {
		HashMap<Integer, Boolean> lookup = new HashMap<>();
		for (int pc : pieces) { lookup.put(pc, true); }
		
		
		for (int i = 0; i < seeder.getPiecesAmt(); i++) {
			if (lookup.get(i) == null) {
				leecher.addRequiredPiece(i);
			}
		}
		
		System.out.println("Seeder confirmed file.");
		
		int pc = seeder.getPiecesAmt() - pieces.size();
		
		if (pc > 0) {
			System.out.println("Requesting " + pc + " pieces...");
			startLeecher();
		} else {
			System.out.println("All pieces already downloadeed. Not starting leecher.");
		}
	}

	// called when the leecher receives a piece => we should notify the seeder of this
	private void onPieceDownloaded(byte[] cont, int num) {
		seeder.addAvailablePiece(num);
	}
}
