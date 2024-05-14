package raft

import (
	"fmt"
	"math/rand"
	"net/rpc"
	"time"
)

func (sv *Server) RPC(id int, procedureName string, args interface{}, reply interface{}) error {
	sv.mtx.Lock()
	peer := sv.PeerRPCs[id]
	sv.mtx.Unlock()

	if peer == nil {
		return fmt.Errorf("RPC on non-existent peer: %d", id)
	}

	rpcErr := peer.Call(procedureName, args, reply)
	if rpcErr == rpc.ErrShutdown {
		delete(sv.PeerRPCs, id)
	}

	return rpcErr
}

type RPCProxy struct {
	sv *Server
}

func (prox *RPCProxy) RequestVote(args RequestVotesRequest, reply *RequestVotesResponse) error {
	time.Sleep(time.Duration(50+rand.Intn(50)) * time.Millisecond)
	prox.sv.RequestVote(args, reply)
	return nil
}

func (prox *RPCProxy) AppendEntries(args AppendEntriesRequest, reply *AppendEntriesResponse) error {
	time.Sleep(time.Duration(50+rand.Intn(50)) * time.Millisecond)
	prox.sv.AppendEntries(args, reply)
	return nil
}
