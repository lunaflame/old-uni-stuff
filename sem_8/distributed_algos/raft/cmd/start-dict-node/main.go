package main

import (
	"fmt"
	fifo "github.com/foize/go.fifo"
	"log"
	"os"
	"raft"
	"strconv"
	"sync"
	"time"
)

import "github.com/rosedblabs/rosedb/v2"

type Main struct {
	servers    []*raft.Server
	NumServers int
	mtx        sync.Mutex
}

func getRoseStorage(id int, wipe bool) *raft.RoseStorage {
	options := rosedb.DefaultOptions
	options.DirPath = fmt.Sprintf("/tmp/raft_db_%d", id)

	if wipe {
		os.RemoveAll(options.DirPath)
	}

	db, err := rosedb.Open(options)
	if err != nil {
		panic(err)
	}

	return raft.NewRoseStorage(db)
}

func readQueueOrStdin(queue *fifo.Queue) string {
	var in string
	if queue.Len() == 0 {
		fmt.Scan(&in)
	} else {
		in = queue.Next().(string)
		fmt.Printf("[AUTO ->] %s\n", in)
	}

	return in
}

func main() {
	registerDictCommand()

	main := new(Main)
	main.NumServers = 3

	main.servers = make([]*raft.Server, main.NumServers)
	ready := make(chan interface{})
	commitChans := make([]chan raft.CommitEntry, main.NumServers)
	values := make([]map[string]string, main.NumServers)

	// Create all Servers in this cluster, assign ids and peer ids.
	for i := 0; i < main.NumServers; i++ {
		peerIds := make([]int, 0)
		for p := 0; p < main.NumServers; p++ {
			if p != i {
				peerIds = append(peerIds, p)
			}
		}

		commitChans[i] = make(chan raft.CommitEntry)

		var storage = getRoseStorage(i, true)
		main.servers[i] = raft.NewServer(i, peerIds, ready, commitChans[i], storage)
		main.servers[i].Serve()

		values[i] = make(map[string]string)
		go main.collectIntoMap(&commitChans[i], values[i])
	}

	// Connect all peers to each other.
	for i := 0; i < main.NumServers; i++ {
		for j := 0; j < main.NumServers; j++ {
			if i != j {
				main.servers[i].ConnectToPeer(j, main.servers[j].GetListenAddr())
			}
		}
	}

	close(ready)
	fmt.Println("Servers spun up, accepting input now")
	fmt.Println("set -> Set Key/Value")
	fmt.Println("k, kill -> Shutdown a node")
	fmt.Println("s, status -> Every node's status")
	fmt.Println("q, quit -> Quit")
	fmt.Println("auto -> auto case")

	// yeah im using a library for a fifo queue. gonna cry abt it?
	commandQueue := fifo.NewQueue()
	var valIdx = 0

	for {
		var in = readQueueOrStdin(commandQueue)

		if in == "wait" {
			var durStr = readQueueOrStdin(commandQueue)
			var dur, _ = strconv.Atoi(durStr)

			fmt.Println("/// Waiting... ///")
			time.Sleep(time.Duration(dur) * time.Millisecond)
			continue
		}

		if in == "auto" {
			leaderIdx, _ := main.findLeader()
			if leaderIdx == -1 {
				fmt.Printf("No leader. Try again?")
				continue
			}

			nonLeaderIdx := 0
			if leaderIdx == nonLeaderIdx {
				nonLeaderIdx++
			}

			// Leader broadcasts entries every 100ms (+ "ping"), so make sure you wait for slightly more than that
			commandQueue.Add("set")
			commandQueue.Add("key")
			commandQueue.Add(fmt.Sprintf("value%d", valIdx))
			valIdx++

			commandQueue.Add("set")
			commandQueue.Add("key")
			commandQueue.Add(fmt.Sprintf("value%d", valIdx))
			valIdx++

			// let RPC work
			commandQueue.Add("wait")
			commandQueue.Add("200")

			commandQueue.Add("get")
			commandQueue.Add(fmt.Sprintf("%d", nonLeaderIdx))
			commandQueue.Add("key")

			commandQueue.Add("kill")
			commandQueue.Add(fmt.Sprintf("%d", nonLeaderIdx))

			commandQueue.Add("restart")
			commandQueue.Add(fmt.Sprintf("%d", nonLeaderIdx))

			commandQueue.Add("wait")
			commandQueue.Add("200")

			commandQueue.Add("get")
			commandQueue.Add(fmt.Sprintf("%d", nonLeaderIdx))
			commandQueue.Add("key")
			continue
		}

		if in == "q" || in == "quit" {
			fmt.Println("Exiting.")
			break
		}

		if in == "set" {
			fmt.Printf("Setting Key/Value: ")
			var key, value string
			key = readQueueOrStdin(commandQueue)
			fmt.Print(" = ")
			value = readQueueOrStdin(commandQueue)

			var idx, _ = main.findLeader()
			if idx == -1 {
				fmt.Printf("No leader; cannot set value.\n")
				continue
			}

			main.servers[idx].SubmitCommand(DictCommand{
				Key:   key,
				Value: value,
			})

			continue
		}

		if in == "g" || in == "get" {
			fmt.Printf("Getting Key from Node #")
			var idx int
			var key string
			idx, _ = strconv.Atoi(readQueueOrStdin(commandQueue))

			if idx > len(values) {
				fmt.Printf("NOPE")
				continue
			}

			fmt.Printf("Key: ")
			key = readQueueOrStdin(commandQueue)

			fmt.Printf("map[%s] = \"%s\"\n", key, values[idx][key])
			continue
		}

		if in == "r" || in == "restart" {
			fmt.Printf("Restart node: #")
			var n int
			n, _ = strconv.Atoi(readQueueOrStdin(commandQueue))

			if n > main.NumServers {
				fmt.Printf("out of range (%d > %d)\n", n, main.NumServers)
				continue
			}

			peerIds := make([]int, 0)
			for p := 0; p < main.NumServers; p++ {
				if p != n {
					peerIds = append(peerIds, p)
				}
			}

			newReady := make(chan interface{})
			values[n] = make(map[string]string)
			commitChans[n] = make(chan raft.CommitEntry)

			main.servers[n] = raft.NewServer(n, peerIds, newReady, commitChans[n], getRoseStorage(n, false))
			main.servers[n].Serve()

			go main.collectIntoMap(&commitChans[n], values[n])

			for j := 0; j < main.NumServers; j++ {
				if n != j && main.servers[j].State != raft.Dead {
					if err := main.servers[n].ConnectToPeer(j, main.servers[j].GetListenAddr()); err != nil {
						log.Fatal(err)
					}
					if err := main.servers[j].ConnectToPeer(n, main.servers[n].GetListenAddr()); err != nil {
						log.Fatal(err)
					}
				}
			}

			close(newReady)

			continue
		}

		if in == "k" || in == "kill" {
			fmt.Printf("Killing node: #")
			var n int
			n, _ = strconv.Atoi(readQueueOrStdin(commandQueue))

			if n > main.NumServers {
				fmt.Printf("out of range (%d > %d)\n", n, main.NumServers)
				continue
			}

			main.servers[n].Shutdown()
			close(commitChans[n])
			continue
		}

		if in == "s" || in == "status" {
			for i := 0; i < main.NumServers; i++ {
				fmt.Printf("[%d] state = %s\n", i, main.servers[i].State)
			}
			continue
		}

		fmt.Printf("Unrecognized Key: %s\n", in)
	}
}

func (m *Main) findLeader() (int, int) {
	// Пробуем по нескольку раз, если вдруг лидер ещё не избран
	for r := 0; r < 8; r++ {
		leaderId := -1
		leaderTerm := -1
		for i := 0; i < m.NumServers; i++ {
			_, term, isLeader := m.servers[i].Status()
			if isLeader {
				if leaderId < 0 {
					leaderId = i
					leaderTerm = term
				} else {
					log.Fatalf("servers %d and %d are leaders", leaderId, i)
				}
			}
		}
		if leaderId >= 0 {
			return leaderId, leaderTerm
		}
		time.Sleep(50 * time.Millisecond)
	}

	return -1, -1
}
