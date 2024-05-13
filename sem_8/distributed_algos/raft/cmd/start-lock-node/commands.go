package main

import (
	"log"
	"raft"
)

func newNode() *Node {
	return &Node{
		commitChan: make(chan raft.CommitEntry),

		locks:    map[string]LockData{},
		values:   map[string]string{},
		lockChan: make(chan bool),
	}
}

func (node *Node) Stop() {
	node.server.Shutdown()
	close(node.commitChan)
}

func (main *Main) addNode(idx int) *Node {
	if idx < 0 || idx > main.NumServers {
		log.Fatalf("addNode: index out of range (%d not in %d-%d)\n", idx, 0, main.NumServers)
		return nil
	}

	readyChan := make(chan interface{})

	var node = newNode()
	isFirst := false

	if idx == main.NumServers {
		main.nodes = append(main.nodes, node)
		main.NumServers++
		isFirst = true
	} else {
		main.nodes[idx] = node
	}

	main.nodes[idx].server = raft.NewServer(idx, readyChan, node.commitChan, getRoseStorage(idx, isFirst))
	main.nodes[idx].server.Serve()

	go main.collectLocks(node.commitChan, main.nodes[idx])

	for j := 0; j < main.NumServers; j++ {
		if idx != j && main.nodes[j].server.State != raft.Dead {
			if err := main.nodes[idx].server.ConnectToPeer(j, main.nodes[j].server.GetListenAddr()); err != nil {
				log.Fatal(err)
			}
			if err := main.nodes[j].server.ConnectToPeer(idx, main.nodes[idx].server.GetListenAddr()); err != nil {
				log.Fatal(err)
			}
		}
	}

	close(readyChan)

	return main.nodes[idx]
}

func (main *Main) stopNode(idx int) {
	if idx < 0 || idx >= main.NumServers {
		log.Fatalf("addNode: index out of range (%d not in %d-%d)\n", idx, 0, main.NumServers)
		return
	}

	if main.nodes[idx].server.State == raft.Dead {
		return
	}

	main.nodes[idx].Stop()
}

func (main *Main) removeNode(idx int) {
	if idx < 0 || idx >= main.NumServers {
		log.Fatalf("addNode: index out of range (%d not in %d-%d)\n", idx, 0, main.NumServers)
		return
	}

	main.stopNode(idx)
	main.nodes = append(main.nodes[:idx], main.nodes[idx+1:]...) // WTF GO
}
