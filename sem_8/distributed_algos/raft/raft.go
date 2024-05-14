// боже храни туториалы по рафту 😎😎😎😎😎
package raft

import (
	"log"
	"math/rand"
	"time"
)

type ServerState int

const (
	Follower ServerState = iota
	Candidate
	Leader
	Dead
)

func (sv *Server) minElectionTimeout() time.Duration {
	return time.Duration(500+rand.Intn(500)) * time.Millisecond
}
func (sv *Server) electionTimeout() time.Duration {
	return sv.minElectionTimeout() + time.Duration(rand.Intn(500))*time.Millisecond
}

func (sv *Server) followerHeartbeatCheckTimer() time.Duration {
	return time.Duration(100) * time.Millisecond
}

func (sv *Server) leaderHeartbeatTimer() time.Duration {
	return time.Duration(200) * time.Millisecond
}

func (sv *Server) Status() (id int, term int, leader bool) {
	sv.mtx.Lock()
	defer sv.mtx.Unlock()
	return sv.Id, sv.Term, sv.State == Leader
}

func (sv *Server) runElectionTimer() {
	timeoutDuration := sv.electionTimeout()
	termStarted := sv.Term

	ticker := time.NewTicker(sv.followerHeartbeatCheckTimer())

	defer ticker.Stop()
	for {
		<-ticker.C

		sv.mtx.Lock()

		if sv.State != Candidate && sv.State != Follower {
			sv.mtx.Unlock()
			return
		}

		if termStarted != sv.Term {
			sv.mtx.Unlock()
			return
		}

		if time.Since(sv.lastElectionReset) >= timeoutDuration {
			log.Printf("%d: %.1f passed without a heartbeat; electing!\n", sv.Id, time.Since(sv.lastElectionReset).Seconds())
			sv.startElection()
			sv.mtx.Unlock()
			return
		}

		sv.mtx.Unlock()
		continue
	}
}

func (sv *Server) lastLogIndexAndTerm() (int, int) {
	if len(sv.Entries) == 0 {
		return -1, -1
	}

	lastIndex := len(sv.Entries) - 1
	return lastIndex, sv.Entries[lastIndex].Term
}

func (sv *Server) startElection() {
	sv.State = Candidate
	sv.Term += 1
	savedTerm := sv.Term
	sv.lastElectionReset = time.Now()
	sv.VotedFor = sv.Id

	votesReceived := 1

	for peerId := range sv.PeerRPCs {
		go func(peerId int) {
			sv.mtx.Lock()
			savedLastLogIndex, savedLastLogTerm := sv.lastLogIndexAndTerm()
			sv.mtx.Unlock()

			req := RequestVotesRequest{
				RPCMessage:  RPCMessage{Term: savedTerm},
				CandidateId: sv.Id,

				LastLogIndex: savedLastLogIndex,
				LastLogTerm:  savedLastLogTerm,
			}

			var resp RequestVotesResponse

			if err := sv.RPC(peerId, "Raft.RequestVote", req, &resp); err == nil {
				// log.Printf("%d is requesting vote from %d", sv.Id, peerId)
				sv.mtx.Lock()
				defer sv.mtx.Unlock()

				if sv.State != Candidate {
					return
				}

				if resp.Term > savedTerm {
					// Received term higher than what we had => we *can't* be the leader anymore
					sv.becomeFollower(resp.Term)
					return
				} else if resp.Term == savedTerm { // can we receive a term *less* than current...?
					if resp.VotedForReceiver {
						// we swindled another one bois
						votesReceived++
						if votesReceived*2 > len(sv.PeerRPCs)+1 {
							// i'm the captain now
							sv.becomeLeader()
							return
						}
					}
				}
			}
		}(peerId)
	}

	go sv.runElectionTimer()
}

func (sv *Server) becomeFollower(term int) {
	sv.State = Follower
	sv.Term = term
	sv.VotedFor = -1
	sv.lastElectionReset = time.Now()

	go sv.runElectionTimer()
}

func (sv *Server) becomeLeader() {
	sv.State = Leader

	go func() {
		timeout := sv.leaderHeartbeatTimer()
		ticker := time.NewTicker(timeout)
		defer ticker.Stop()

		for {
			send := false

			select {
			case <-ticker.C:
				send = true
			case _, ok := <-sv.sendAppendEntriesChan:
				if !ok {
					return
				}

				send = true
				ticker.Reset(timeout)
			}

			if send {
				sv.mtx.Lock()
				if sv.State != Leader {
					sv.mtx.Unlock()
					return
				}
				sv.mtx.Unlock()
				sv.leaderBroadcastEntries()
			}
		}
		sv.sendAppendEntriesChan <- struct{}{}
	}()
}

func (sv *Server) commitChanSender() {
	for range sv.newCommitReadyChan {
		sv.mtx.Lock()
		savedTerm := sv.Term
		savedLastApplied := sv.lastAppliedIndex

		var entries []CommitEntry
		// log.Printf("%d: %d > %d?\n", sv.Id, sv.commitIndex, sv.lastAppliedIndex)
		if sv.commitIndex > sv.lastAppliedIndex {
			entries = sv.Entries[sv.lastAppliedIndex+1 : sv.commitIndex+1]
			sv.lastAppliedIndex = sv.commitIndex
		}
		sv.mtx.Unlock()

		// log.Printf("Издаём %d эвентов\n", len(entries))

		for i, entry := range entries {
			// log.Printf("\tEmit #%d %s: [%s] = %s\n", sv.Id, sv.State, i, entry.Command)
			sv.commitChan <- CommitEntry{
				Command: entry.Command,
				Index:   savedLastApplied + i + 1,
				Term:    savedTerm,
			}
		}
	}
}

func (sv *Server) leaderBroadcastEntries() {
	sv.mtx.Lock()
	if sv.State != Leader {
		sv.mtx.Unlock()
		return
	}
	wasTerm := sv.Term
	sv.mtx.Unlock()

	for peerId := range sv.PeerRPCs {
		sv.mtx.Lock()

		ni := sv.nextIndex[peerId]
		prevLogIndex := ni - 1
		prevLogTerm := -1
		if prevLogIndex >= 0 {
			prevLogTerm = sv.Entries[prevLogIndex].Term
		}
		entries := sv.Entries[ni:]

		args := AppendEntriesRequest{
			RPCMessage: RPCMessage{
				Term: wasTerm,
			},
			LeaderId:     sv.Id,
			PrevLogIndex: prevLogIndex,
			PrevLogTerm:  prevLogTerm,
			Entries:      entries,
			LeaderCommit: sv.commitIndex,
		}
		sv.mtx.Unlock()

		go func(peerId int) {
			var reply AppendEntriesResponse

			err := sv.RPC(peerId, "Raft.AppendEntries", args, &reply)

			if err != nil {
				log.Printf("Error in AppendEntries: %s\n", err)
				return
			}
			// log.Printf("AE: %d to %d\n", sv.Id, peerId)

			sv.mtx.Lock()
			defer sv.mtx.Unlock()

			if reply.Term > wasTerm {
				sv.becomeFollower(reply.Term)
				return
			}

			if sv.State == Leader && wasTerm == reply.Term {
				if reply.Success {
					sv.nextIndex[peerId] = ni + len(entries)
					sv.matchIndex[peerId] = sv.nextIndex[peerId] - 1

					savedCommitIndex := sv.commitIndex
					// log.Printf("successful entry append %d/%d\n", sv.commitIndex+1, len(entries))
					for i := sv.commitIndex + 1; i < len(sv.Entries); i++ {
						// log.Printf("\t%d = %d (%d)\n", i, sv.Entries[i].Term, sv.Term)
						if sv.Entries[i].Term <= sv.Term {
							matchCount := 1
							for peerId := range sv.PeerRPCs {
								if sv.matchIndex[peerId] >= i {
									matchCount++
								}
							}

							// Если больше половины реплицировали лог, то
							// продвигаем commitIndex
							if matchCount > len(sv.PeerRPCs)/2 {
								sv.commitIndex = i
							}
						}
					}

					if sv.commitIndex != savedCommitIndex {
						sv.newCommitReadyChan <- struct{}{}
						sv.sendAppendEntriesChan <- struct{}{}
					}
				} else {
					sv.nextIndex[peerId] = ni - 1
				}
			}
		}(peerId)
	}
}

func (sv *Server) SubmitCommand(command interface{}) bool {
	sv.mtx.Lock()

	if sv.State == Leader {
		sv.Entries = append(sv.Entries, CommitEntry{Command: command, Term: sv.Term})
		sv.savePersistentState()
		sv.mtx.Unlock()
		sv.sendAppendEntriesChan <- struct{}{}
		sv.receiveChan <- sv.Entries[len(sv.Entries)-1]
		return true
	}

	sv.mtx.Unlock()
	return false
}
