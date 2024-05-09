package raft

type RPCMessage struct {
	Term int
}

type CommitEntry struct {
	Command interface{}
	Index   int
	Term    int
}

// "Raft has two kinds of RPCs peers send each other:"
// RequestVotes - used only in the candidate state
// AppendEntries - used only in the leader state

type RequestVotesRequest struct {
	RPCMessage
	CandidateId int

	// Узел получит голос только если его лог новее,
	// чем лог хотя бы половины других
	LastLogIndex int
	LastLogTerm  int
}

type RequestVotesResponse struct {
	RPCMessage
	VotedForReceiver bool
}

type AppendEntriesRequest struct {
	RPCMessage

	// So follower can redirect clients
	LeaderId int

	// Index of log entry immediately preceding new ones
	PrevLogIndex int

	// Term of prevLogIndex entry
	PrevLogTerm int

	// Log entries to store. Empty for heartbeat.
	Entries []CommitEntry

	// Leader's commitIndex
	LeaderCommit int
}

type AppendEntriesResponse struct {
	RPCMessage

	// true if follower contained entry matching prevLogIndex and
	// prevLogTerm
	Success bool
}
