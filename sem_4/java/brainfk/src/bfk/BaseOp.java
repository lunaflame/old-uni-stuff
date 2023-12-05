package bfk;

public interface BaseOp {
	public void Execute(BFContext ctx); // Called when this op is encountered during execution
	default void Parse(BFParser prs) {}; // Optional: called when this op is encountered during parsing
	default void FinishParse(BFParser prs) {};
}