package main

import (
	"encoding/gob"
	"fmt"
	"log"
	"net"
	"raft"
	"time"
)

type LockCommand struct {
	LockName string
	LockedBy int
	NewState bool
	LockTime int64
}

type LockData struct {
	LockedBy int
	LockTime int64
}

type DictCommand struct {
	Key   string
	Value string
}

type AddNodeCommand struct {
	NodeId   int
	NodeAddr string
}

type RemoveNodeCommand struct {
	NodeId int
}

const LockTimeout = 20000 // milliseconds

func registerLockCommand() {
	gob.Register(LockCommand{})
	gob.Register(DictCommand{})
	gob.Register(AddNodeCommand{})
	gob.Register(RemoveNodeCommand{})
}

func (m *Main) collectLocks(commitChan chan raft.CommitEntry, node *Node) {
	for command := range commitChan {
		m.mtx.Lock()

		if lc, ok := command.Command.(LockCommand); ok {
			m.processLock(node, lc)
		}

		if dc, ok := command.Command.(DictCommand); ok {
			m.processDictCommand(node, dc)
		}

		if rc, ok := command.Command.(RemoveNodeCommand); ok && node.server.State == raft.Leader {
			m.processRemoveNodeCommand(node, rc)
		}

		m.mtx.Unlock()
	}
}

func (m *Main) collectReconfig(receiveChan chan raft.CommitEntry, node *Node) {
	for command := range receiveChan {
		m.mtx.Lock()
		fmt.Printf("%d recv config cmd %v\n", node.server.Id, command.Command)
		if ac, ok := command.Command.(AddNodeCommand); ok {
			m.processAddNodeCommand(node, ac)
		} else if rc, ok := command.Command.(RemoveNodeCommand); ok {
			m.processRemoveNodeCommand(node, rc)
		}
		m.mtx.Unlock()
	}
}

func (m *Main) processAddNodeCommand(node *Node, cmd AddNodeCommand) {
	var addr, _ = net.ResolveTCPAddr("tcp", cmd.NodeAddr)
	node.server.ConnectToPeer(cmd.NodeId, addr)
}

func (m *Main) processRemoveNodeCommand(node *Node, cmd RemoveNodeCommand) {
	if node.server.Id == cmd.NodeId {
		if node.server.State != raft.Leader {
			// oh you cant do this to me
			// I started this cluster
			// YOU KNOW HOW MUCH I SACRIFICED

			// node.Stop() // you're out norman
			m.removeNode(cmd.NodeId)
		}
	} else {
		err := node.server.DisconnectPeer(cmd.NodeId)
		if err != nil {
			log.Fatalf("err in disconnectpeer: %s\n", err.Error())
		}
	}
}

func (m *Main) processDictCommand(node *Node, cmd DictCommand) {
	node.values[cmd.Key] = cmd.Value
}

func (m *Main) processLock(node *Node, lc LockCommand) {
	// fmt.Printf("%v processing lock %v by %v (%v)\n", node.server.Id, lc.LockName, lc.LockedBy, node.lockWaiting)
	ld, has := node.locks[lc.LockName]

	ok := false

	var isExpired = lc.LockTime-ld.LockTime > LockTimeout
	if !lc.NewState {
		// Unlock
		ok = true
		delete(node.locks, lc.LockName)
	} else if !has || lc.LockedBy == ld.LockedBy || isExpired {
		ok = true
		node.locks[lc.LockName] = LockData{
			LockedBy: lc.LockedBy,
			LockTime: lc.LockTime,
		}
	}

	if node.lockWaiting {
		// log.Printf("%v unlocked w/ %v (%v, %v, %v)\n", node, ok, !has, ld.LockedBy == node.server.Id, isExpired)
		node.lockChan <- ok
	}
}

func (m *Main) tryLock(sender *Node, command LockCommand) bool {
	m.mtx.Lock()
	m.sendLeaderCommand(sender, command)

	sender.lockWaiting = true
	m.mtx.Unlock()
	ok := <-sender.lockChan
	sender.lockWaiting = false

	return ok
}

func (m *Main) processCommand(sender *Node, command interface{}) {
	m.mtx.Lock()
	m.sendLeaderCommand(sender, command)

	if lc, ok := command.(LockCommand); ok && lc.NewState {
		// Попытка взять лок; блокируем до того или иного исхода
		sender.lockWaiting = true
		m.mtx.Unlock()
		ok := <-sender.lockChan
		sender.lockWaiting = false

		if !ok {
			log.Printf("Lock failed; already taken probably\n")
		} else {
			log.Printf("Lock success!\n")
		}
	} else {
		m.mtx.Unlock()
	}
}

func (m *Main) sendLeaderCommand(sender *Node, cmd interface{}) {
	var idx, _ = m.findLeader()
	for idx == -1 {
		time.Sleep(time.Duration(1000) * time.Millisecond)
		idx, _ = m.findLeader()
	}

	m.nodes[idx].server.SubmitCommand(cmd)
}
