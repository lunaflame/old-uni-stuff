package main.java;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Main {
	
	private static void printUsage(String[] args) {
		System.out.println("Usage: \n" +
				"	creating a torrent file: torrent.java create [file]\n" +
				"	get into the swarm: torrent.java swarm [torrent_file] [seeding_port] [ip:port list to leech]\n"
		);
	}
	
	static List<String> servers = new ArrayList<>();
	static String seedFile = "?";
	static String leechFile = "?";

	public static void main(String[] args) {
		if (args.length == 0) {
			printUsage(args);
			return;
		}
		
		boolean isSwarm = false;
		int port = 0;
	
		if (args[0].equals("create")) {
			seedFile = args[1];
			try {
				byte[] sha = TorrentSeeder.encFile(seedFile);
				System.out.println("Created .torrent file, SHA: " + TorrentUtils.SHA1toHex(sha));
			} catch (Exception ex) {
				System.out.println("failed to encode file " + seedFile);
				System.out.println(ex.getMessage());
				
				ex.printStackTrace();
			}
			
			return;
		} else if (args[0].equals("swarm")) {
			
			if (args.length < 3) {
				printUsage(args);
				return;
			}

			leechFile = args[1]; // torrent file
			try {
				port = Integer.parseInt(args[2]);
			} catch (Exception e) {
				System.out.println("Failed to parse port.");
				printUsage(args);
			}
			
			servers.addAll(Arrays.asList(args).subList(3, args.length));
			isSwarm = true;
		}
		
		if (!isSwarm) {
			printUsage(args);
			return;
		}

		try {
			// todo: threads
			TorrentSwarm swarm = new TorrentSwarm();
			swarm.start(leechFile, port, servers);
		} catch (Exception e) {
			System.out.println("main - caught exception");
			e.printStackTrace();
			return;
		}
    }

}