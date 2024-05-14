package main

import (
	"log"
	"raft"
)

func newNode() *Node {
	return &Node{
		commitChan:  make(chan raft.CommitEntry),
		receiveChan: make(chan raft.CommitEntry),

		locks:    map[string]LockData{},
		values:   map[string]string{},
		lockChan: make(chan bool),
	}
}

func (node *Node) Stop() {
	node.server.Shutdown()
	close(node.commitChan)
	close(node.receiveChan)
}

func (main *Main) createDisconnectedNode(idx int) (*Node, chan interface{}) {
	if idx < 0 || idx > main.NumServers {
		log.Fatalf("addNode: index out of range (%d not in %d-%d)\n", idx, 0, main.NumServers)
		return nil, nil
	}

	readyChan := make(chan interface{})

	var node = newNode()
	node.index = idx
	isFirst := false

	if idx == main.NumServers {
		main.nodes = append(main.nodes, node)
		main.NumServers++
		isFirst = true
	} else {
		main.nodes[idx] = node
	}

	main.nodes[idx].server = raft.NewServer(main.CurrentId, readyChan,
		node.commitChan, node.receiveChan, getRoseStorage(main.CurrentId, isFirst))
	main.CurrentId++
	main.nodes[idx].server.Serve()

	go main.collectLocks(node.commitChan, main.nodes[idx])
	go main.collectReconfig(node.receiveChan, main.nodes[idx])

	return main.nodes[idx], readyChan
}

func (main *Main) addNode(idx int) *Node {
	if idx < 0 || idx > main.NumServers {
		log.Fatalf("addNode: index out of range (%d not in %d-%d)\n", idx, 0, main.NumServers)
		return nil
	}

	node, readyChan := main.createDisconnectedNode(idx)

	for j := 0; j < main.NumServers; j++ {
		var otherNode = main.nodes[j]

		if idx != j && otherNode.server.State != raft.Dead {
			if err := node.server.ConnectToPeer(otherNode.server.Id, otherNode.server.GetListenAddr()); err != nil {
				log.Fatal(err)
			}
			if err := otherNode.server.ConnectToPeer(node.server.Id, node.server.GetListenAddr()); err != nil {
				log.Fatal(err)
			}
		}
	}

	close(readyChan)

	return node
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

func (main *Main) removeNode(id int) {
	node := main.findNodeById(id)
	if node == nil {
		log.Fatalf("didn't find node with id %s\n", id)
	}

	main.stopNode(node.index)
	main.nodes = append(main.nodes[:node.index], main.nodes[node.index+1:]...) // WTF GO
	main.NumServers--

	for idx, node := range main.nodes {
		node.index = idx
	}
}

func (main *Main) findNodeById(id int) *Node {
	for _, node := range main.nodes {
		if node.server.Id == id {
			return node
		}
	}

	return nil
}
