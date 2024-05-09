package raft

import (
	"log"
	"net"
	"net/rpc"
	"sync"
	"time"
)

type Server struct {
	mtx sync.Mutex

	Id      int
	PeerIds []int
	State   ServerState

	// Persistent, т.е. должно быть сохранено
	Term     int
	VotedFor int
	Entries  []CommitEntry
	storage  Storage

	lastElectionReset time.Time

	// Клиентские индексы
	commitIndex      int
	lastAppliedIndex int

	// Серверные (лидерные) индексы: [PeerId] = index
	nextIndex  map[int]int
	matchIndex map[int]int

	// Канал для клиента, через который имплементация будет гонять свою логику
	commitChan chan<- CommitEntry

	// Внутренний канал для асинхронного эмита commitChan
	newCommitReadyChan chan struct{}

	// Внутренний канал для оповещения о необходимости отправки AppendEntries
	sendAppendEntriesChan chan struct{}

	rpcServer *rpc.Server
	peerRPCs  map[int]*rpc.Client
	rpcProxy  RPCProxy
	listener  net.Listener
	wg        sync.WaitGroup

	quit chan interface{}
}

func NewServer(id int, peerIds []int, ready <-chan interface{},
	commitChan chan<- CommitEntry, storage Storage) *Server {

	ret := &Server{
		Id:       id,
		PeerIds:  peerIds,
		State:    Follower,
		peerRPCs: map[int]*rpc.Client{},
		Term:     0,
		VotedFor: -1,
		storage:  storage,

		lastAppliedIndex: -1,
		commitIndex:      -1,
		nextIndex:        make(map[int]int),
		matchIndex:       make(map[int]int),

		newCommitReadyChan:    make(chan struct{}, 10),
		sendAppendEntriesChan: make(chan struct{}, 10),
		commitChan:            commitChan,
		quit:                  make(chan interface{}),
	}

	ret.rpcProxy = RPCProxy{sv: ret}

	if ret.storage.HasData() {
		ret.restorePersistentState(ret.storage)
	}

	go func() {
		// Блокируем, пока не получим сигнал от ready
		<-ready
		ret.mtx.Lock()
		ret.lastElectionReset = time.Now()
		ret.mtx.Unlock()
		ret.runElectionTimer()
	}()

	go ret.commitChanSender()
	return ret
}

func (sv *Server) Serve() {
	sv.mtx.Lock()

	sv.rpcServer = rpc.NewServer()
	err := sv.rpcServer.RegisterName("Raft", &sv.rpcProxy)
	if err != nil {
		return
	}

	sv.listener, err = net.Listen("tcp", ":0")
	if err != nil {
		log.Fatal(err)
	}
	log.Printf("[%v] listening at %s", sv.Id, sv.listener.Addr())
	sv.mtx.Unlock()

	sv.wg.Add(1)
	go func() {
		defer sv.wg.Done()

		var remConn = make(chan interface{})
		exiting := false

		for {
			conn, err := sv.listener.Accept()
			if err != nil {
				select {
				case <-sv.quit:
					exiting = true
					close(remConn)
					return
				default:
					log.Fatal("accept error:", err)
				}
			}

			sv.wg.Add(1)

			/*  this fucking sucks
			    https://t.ly/fll7f

				net/rpc дедлочится если закрыть соединение пока он ждёт ответа (вроде). Спасибо очень круто!!!
			*/

			go func() {
				sv.rpcServer.ServeConn(conn)
				if !exiting {
					sv.wg.Done()
				}
			}()

			go func() {
				_, _ = <-remConn
				conn.Close()
				sv.wg.Done()
			}()
		}
	}()
}

func (sv *Server) ConnectToPeer(peerId int, addr net.Addr) error {
	sv.mtx.Lock()
	defer sv.mtx.Unlock()

	client, err := rpc.Dial(addr.Network(), addr.String())
	if err != nil {
		log.Printf("error during connect %s\n", err)
		return err
	}
	sv.peerRPCs[peerId] = client
	return nil
}

func (sv *Server) GetListenAddr() net.Addr {
	return sv.listener.Addr()
}

func (sv *Server) Shutdown() {
	sv.mtx.Lock()
	defer sv.mtx.Unlock()
	sv.State = Dead
	close(sv.newCommitReadyChan)
	close(sv.quit)
	sv.listener.Close()
	sv.storage.Close()
	sv.wg.Wait()
}
