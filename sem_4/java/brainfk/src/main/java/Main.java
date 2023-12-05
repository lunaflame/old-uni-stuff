package main.java;

import bfk.BFParser;

public class Main {
  public static void main(String[] args) {
  	if (args.length < 1) {
  		System.out.println("feed me a ~~stray cat~~ file");
  		System.out.println("(ie: brainfk my_cool_code.txt)");
  		return;
  	}
  	BFParser parser = new BFParser();
  	parser.ReadFromFile(args[0]);

	try {
		parser.Parse();
	} catch (Exception e) {
		System.out.println("Exception caught during parsing; exiting.");
		e.printStackTrace();
	}
 
    parser.Execute();
  }
}
