// на часах 4:45 утра
// отправляйте подмогу

package main

import (
	"fmt"
	fifo "github.com/foize/go.fifo"
	"log"
	"os"
	"raft"
	"strconv"
	"strings"
	"sync"
	"time"
)

import "github.com/rosedblabs/rosedb/v2"

type Main struct {
	nodes      []*Node
	NumServers int
	mtx        sync.Mutex
}

type Node struct {
	server     *raft.Server
	commitChan chan raft.CommitEntry
	locks      map[string]LockData
	values     map[string]string

	lockWaiting bool
	lockChan    chan bool
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

func (main *Main) addCommandHandlerChan(node *Node) chan interface{} {
	chn := make(chan interface{})

	go func() {
		// Бесконечно вытаскиваем команды из канала для обработки в отдельной горутине
		// Сделано для того, чтобы поданная команда не блокировала (в случае блокировки, например)
		for cmd := range chn {
			main.processCommand(node, cmd)
		}
	}()

	return chn
}

func main() {
	registerLockCommand()

	main := new(Main)
	main.NumServers = 3

	main.nodes = make([]*Node, main.NumServers)
	ready := make(chan interface{})

	for i := 0; i < main.NumServers; i++ {
		var storage = getRoseStorage(i, true)
		var node = newNode()

		main.nodes[i] = node
		node.server = raft.NewServer(i, ready, node.commitChan, storage)
		node.server.Serve()

		go main.collectLocks(node.commitChan, main.nodes[i])
	}

	// Connect all peers to each other.
	for i := 0; i < main.NumServers; i++ {
		for j := 0; j < main.NumServers; j++ {
			if i != j {
				main.nodes[i].server.ConnectToPeer(j, main.nodes[j].server.GetListenAddr())
			}
		}
	}

	inputQueue := fifo.NewQueue()
	commandChans := make([]chan interface{}, 0)

	for i := 0; i < main.NumServers; i++ {
		commandChans = append(commandChans, main.addCommandHandlerChan(main.nodes[i]))
	}

	close(ready)

	fmt.Println("Servers spun up, accepting input now")
	fmt.Println("set -> Set Key/Value")
	fmt.Println("lock -> Lock")
	fmt.Println("unlock -> Unlock")
	fmt.Println("k, kill -> Shutdown a node")
	fmt.Println("s, status -> Every node's status")
	fmt.Println("q, quit -> Quit")
	fmt.Println("auto -> auto case")

	var valIdx = 0

	for {
		var in = readQueueOrStdin(inputQueue)

		if in == "wait" {
			var durStr = readQueueOrStdin(inputQueue)
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
			inputQueue.Add("set")
			inputQueue.Add("key")
			inputQueue.Add(fmt.Sprintf("value%d", valIdx))
			valIdx++

			inputQueue.Add("set")
			inputQueue.Add("key")
			inputQueue.Add(fmt.Sprintf("value%d", valIdx))
			valIdx++

			// let RPC work
			inputQueue.Add("wait")
			inputQueue.Add("200")

			inputQueue.Add("get")
			inputQueue.Add(fmt.Sprintf("%d", nonLeaderIdx))
			inputQueue.Add("key")

			inputQueue.Add("kill")
			inputQueue.Add(fmt.Sprintf("%d", nonLeaderIdx))

			inputQueue.Add("restart")
			inputQueue.Add(fmt.Sprintf("%d", nonLeaderIdx))

			inputQueue.Add("wait")
			inputQueue.Add("200")

			inputQueue.Add("get")
			inputQueue.Add(fmt.Sprintf("%d", nonLeaderIdx))
			inputQueue.Add("key")
			continue
		}

		if in == "q" || in == "quit" {
			fmt.Println("Exiting.")
			break
		}

		if in == "lock" || in == "unlock" {
			var isLock = in == "lock"
			fmt.Printf("Node #")
			idx, _ := strconv.Atoi(readQueueOrStdin(inputQueue))

			if idx < 0 || idx > main.NumServers {
				fmt.Printf("out of range (%d > %d)\n", idx, main.NumServers)
				continue
			}

			if isLock {
				fmt.Printf("Locking: ")
			} else {
				fmt.Printf("Unlocking: ")
			}

			var key string
			key = readQueueOrStdin(inputQueue)

			commandChans[idx] <- LockCommand{
				LockName: key,
				LockedBy: idx,
				NewState: isLock,
				LockTime: time.Now().UnixMilli(),
			}
			continue
		}

		if in == "set" {
			fmt.Printf("Node #")
			idx, _ := strconv.Atoi(readQueueOrStdin(inputQueue))

			if idx < 0 || idx >= main.NumServers {
				fmt.Printf("out of range (%d > %d)\n", idx, main.NumServers)
				continue
			}

			fmt.Printf("Key/Value: ")
			key := readQueueOrStdin(inputQueue)

			fmt.Print(" = ")
			value := readQueueOrStdin(inputQueue)

			commandChans[idx] <- DictCommand{
				Key:   key,
				Value: value,
			}

			continue
		}

		if in == "lockset" || in == "ls" {
			fmt.Printf("Node #")
			idx, _ := strconv.Atoi(readQueueOrStdin(inputQueue))

			if idx < 0 || idx >= main.NumServers {
				fmt.Printf("out of range (%d > %d)\n", idx, main.NumServers)
				continue
			}

			fmt.Printf("Lock name: ")
			ln := readQueueOrStdin(inputQueue)

			lc := LockCommand{
				LockName: ln,
				LockedBy: idx,
				NewState: true,
				LockTime: time.Now().UnixMilli(),
			}

			fmt.Printf("Trying to lock %s...\n", ln)
			if !main.tryLock(main.nodes[idx], lc) {
				fmt.Printf("Node couldn't lock, bailing!\n")
				continue
			}

			fmt.Printf("Key/Value: ")
			key := readQueueOrStdin(inputQueue)

			fmt.Print(" = ")
			value := readQueueOrStdin(inputQueue)

			commandChans[idx] <- DictCommand{
				Key:   key,
				Value: value,
			}

			lc.NewState = false
			main.tryLock(main.nodes[idx], lc) // unlock

			continue
		}

		if in == "g" || in == "get" {
			fmt.Printf("Getting value from Node #")
			idx, _ := strconv.Atoi(readQueueOrStdin(inputQueue))

			if idx < 0 || idx >= main.NumServers {
				fmt.Printf("NOPE")
				continue
			}

			fmt.Printf("Key: ")
			key := readQueueOrStdin(inputQueue)

			fmt.Printf("map[%s] = \"%s\"\n", key, main.nodes[idx].values[key])
			continue
		}

		if in == "r" || in == "restart" {
			fmt.Printf("Restart node: #")
			idx, _ := strconv.Atoi(readQueueOrStdin(inputQueue))

			if idx < 0 || idx >= main.NumServers {
				fmt.Printf("Index out of range!")
				return
			}

			node := main.addNode(idx)
			commandChans[idx] = main.addCommandHandlerChan(node)
			continue
		}

		if in == "k" || in == "kill" {
			fmt.Printf("Killing node: #")
			idx, _ := strconv.Atoi(readQueueOrStdin(inputQueue))

			main.stopNode(idx)
			continue
		}

		if in == "add" {
			fmt.Printf("Adding node...")
			node := main.addNode(main.NumServers)
			if len(commandChans) > node.server.Id {
				// i hate go i hate go i hate go i hate go
				commandChans[node.server.Id] = main.addCommandHandlerChan(node)
			} else {
				commandChans = append(commandChans, main.addCommandHandlerChan(node))
			}

			continue
		}

		if in == "remove" {
			fmt.Printf("Removing node #")
			var n int
			n, _ = strconv.Atoi(readQueueOrStdin(inputQueue))

			if n < 0 || n >= main.NumServers {
				fmt.Printf("Index out of range!")
				return
			}

			main.removeNode(n)
			continue
		}

		if in == "s" || in == "status" {
			for i := 0; i < main.NumServers; i++ {
				node := main.nodes[i]
				var sb strings.Builder
				var now = time.Now().UnixMilli()

				for lockName, lock := range node.locks {
					if lock.LockedBy == i {
						sb.WriteString(lockName)
						sb.WriteString(fmt.Sprintf(" (%.1fs./%ds. ago)", float64((now-lock.LockTime))/1000, LockTimeout/1000))
					}
				}

				fmt.Printf("[%d] state = %v (blocked = %v, locks = %s)\n", i, node.server.State,
					node.lockWaiting, sb.String())
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
			_, term, isLeader := m.nodes[i].server.Status()
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
