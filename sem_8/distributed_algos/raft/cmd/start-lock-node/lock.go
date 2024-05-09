package main

import (
	"encoding/gob"
	"raft"
)

type LockCommand struct {
	LockName string
	NewState bool
}

func registerLockCommand() {
	gob.Register(LockCommand{})
}

func (m *Main) collectLocks(commitChan *chan raft.CommitEntry, locks map[string]bool) {
	for command := range *commitChan {
		m.mtx.Lock()
		var dc = command.Command.(LockCommand)
		// log.Printf("\tHT: [%s] = %s\n", dc.Key, dc.Value)
		locks[dc.LockName] = dc.NewState
		m.mtx.Unlock()
	}
}
