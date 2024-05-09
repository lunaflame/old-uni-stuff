package main

import (
	"encoding/gob"
	"raft"
)

type DictCommand struct {
	Key   string
	Value string
}

func registerDictCommand() {
	gob.Register(DictCommand{})
}

func (m *Main) collectIntoMap(commitChan *chan raft.CommitEntry, into map[string]string) {
	for command := range *commitChan {
		m.mtx.Lock()
		var dc = command.Command.(DictCommand)
		// log.Printf("\tHT: [%s] = %s\n", dc.Key, dc.Value)
		into[dc.Key] = dc.Value
		m.mtx.Unlock()
	}
}
