package raft

import (
	"time"
)

func (sv *Server) RequestVote(args RequestVotesRequest, reply *RequestVotesResponse) {
	sv.mtx.Lock()
	defer sv.mtx.Unlock()
	if sv.State == Dead {
		return
	}

	// fmt.Printf("%d: reqvote from %d received; last heartbeat %v\n", sv.Id, args.CandidateId, time.Now().Sub(sv.lastHeartbeatReceive))

	// Запрос на голос хотя наш текущий лидер всё ещё жив
	if time.Now().Sub(sv.lastHeartbeatReceive) < sv.minElectionTimeout() {
		reply.Term = sv.Term
		reply.VotedForReceiver = false
		return
	}

	lastLogIndex, lastLogTerm := sv.lastLogIndexAndTerm()
	vote := false

	if args.Term > sv.Term {
		sv.becomeFollower(args.Term)
	}

	if args.Term == sv.Term &&
		(sv.VotedFor == -1 || sv.VotedFor == args.CandidateId) &&
		(args.LastLogTerm > lastLogTerm ||
			(args.LastLogTerm == lastLogTerm && args.LastLogIndex >= lastLogIndex)) {
		vote = true
		sv.VotedFor = args.CandidateId
		sv.lastElectionReset = time.Now()
	}

	reply.VotedForReceiver = vote
	reply.Term = sv.Term
	sv.savePersistentState()
}

func (sv *Server) AppendEntries(args AppendEntriesRequest, reply *AppendEntriesResponse) {
	sv.mtx.Lock()

	if sv.State == Dead {
		sv.mtx.Unlock()
		return
	}

	if args.Term > sv.Term {
		sv.becomeFollower(args.Term)
	}

	var newEntries []CommitEntry

	reply.Success = false
	if args.Term == sv.Term {
		if sv.State != Follower {
			sv.becomeFollower(args.Term)
		}

		sv.lastElectionReset = time.Now()
		sv.lastHeartbeatReceive = time.Now()

		// Если мы получили обновление с прошлым индексом, который у нас тоже есть,
		// и терм лидера совпадает, то мы обновляем наш лог. Иначе команда невалидна
		if args.PrevLogIndex == -1 ||
			(args.PrevLogIndex < len(sv.Entries) && args.PrevLogTerm == sv.Entries[args.PrevLogIndex].Term) {
			reply.Success = true

			/*
				if len(args.Entries) > 0 {
					log.Printf("%d: Получено %d энтрей\n", sv.Id, len(args.Entries))
					log.Printf("||| %d > %d?\n", args.LeaderCommit, sv.commitIndex)
					for i := 0; i < len(args.Entries); i++ {
						log.Printf("\t%s\n", args.Entries[i].Command)
					}
				}
			*/

			logInsertIndex := args.PrevLogIndex + 1
			newEntriesIndex := 0

			// Находим точку вставки, где сменился терм между
			// нашим логом и новыми командами (либо кладём на самый верх)
			for {
				if logInsertIndex >= len(sv.Entries) || newEntriesIndex >= len(args.Entries) {
					break
				}
				if sv.Entries[logInsertIndex].Term != args.Entries[newEntriesIndex].Term {
					break
				}
				logInsertIndex++
				newEntriesIndex++
			}

			if newEntriesIndex < len(args.Entries) {
				sv.Entries = append(sv.Entries[:logInsertIndex], args.Entries[newEntriesIndex:]...)
				newEntries = args.Entries[newEntriesIndex:]
			}

			if args.LeaderCommit > sv.commitIndex {
				sv.commitIndex = min(args.LeaderCommit, len(sv.Entries)-1)
				sv.newCommitReadyChan <- struct{}{}
			}
		}

		reply.Success = true
	}

	reply.Term = sv.Term
	sv.savePersistentState()
	sv.mtx.Unlock()

	// Полученные записи издаём вне мутекса, поскольку действия могут повторно захватить его и вызвать дедлок
	if newEntries != nil {
		for _, entry := range newEntries {
			sv.receiveChan <- entry
		}
	}
}
