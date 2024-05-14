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

type PeerInfo struct {
	NodeId   int
	NodeAddr string
}

type NewConfigurationCommand struct {
	// Новая нода не будет знать, кто уже в кластере, поэтому нужно передавать всю конфигурацию.
	NewConfiguration []PeerInfo
}

const LockTimeout = 20000 // milliseconds

func registerLockCommand() {
	gob.Register(LockCommand{})
	gob.Register(DictCommand{})
	gob.Register(NewConfigurationCommand{})
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

		if rc, ok := command.Command.(NewConfigurationCommand); ok {
			m.processNewConfigCommand(node, rc, true)
		}

		m.mtx.Unlock()
	}
}

func (m *Main) collectReconfig(receiveChan chan raft.CommitEntry, node *Node) {
	for command := range receiveChan {
		if rc, ok := command.Command.(NewConfigurationCommand); ok {
			m.processNewConfigCommand(node, rc, false)
		}
	}
}

func (m *Main) processNewConfigCommand(node *Node, cmd NewConfigurationCommand, isCommit bool) {
	// Лидер удаляет ноды только при коммите
	var canRemove = node.server.State != raft.Leader || isCommit
	var canAdd = !isCommit

	peerMap := make(map[int]bool)

	for _, peer := range cmd.NewConfiguration {
		peerMap[peer.NodeId] = true
	}

	// fmt.Printf("%d -> new config: %v\n", node.server.Id, peerMap)

	for curPeerId := range node.server.PeerRPCs {
		if canRemove && !peerMap[curPeerId] {
			// Was connected to peer but it's not in the new configuration
			// norman you're being kicked out of the blunt rotation
			fmt.Printf("\tdisconnect %d\n", curPeerId)
			err := node.server.DisconnectPeer(curPeerId)
			if err != nil {
				log.Fatalf("err in disconnect %v\n", err)
			}
		}
	}

	for newPeerId := range peerMap {
		if canAdd && node.server.PeerRPCs[newPeerId] == nil && m.findNodeById(newPeerId) != nil && // (читерство...!)
			newPeerId != node.server.Id {
			// Was not connected to the new peer
			fmt.Printf("\tconnect %d\n", newPeerId)
			var info = cmd.NewConfiguration[newPeerId]
			var addr, _ = net.ResolveTCPAddr("tcp", info.NodeAddr)
			err := node.server.ConnectToPeer(info.NodeId, addr)
			if err != nil {
				log.Fatalf("err in connect %v\n", err)
			}
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
	fmt.Printf("processCommand %v %v\n", sender.server.Id, command)
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
	fmt.Printf("attempting to send cmd to leader...\n")
	for idx == -1 {
		time.Sleep(time.Duration(1000) * time.Millisecond)
		idx, _ = m.findLeader()
	}
	fmt.Printf("found (%d), sending\n", idx)
	m.nodes[idx].server.SubmitCommand(cmd)
}
