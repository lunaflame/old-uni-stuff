package raft

import (
	"bytes"
	"encoding/gob"
	"github.com/rosedblabs/rosedb/v2"
	"log"
)

type Storage interface {
	Set(key string, value []byte)
	Get(key string) ([]byte, bool)

	HasData() bool
	Close() error
}

func (sv *Server) restorePersistentState(storage Storage) {
	termData, _ := sv.storage.Get("currentTerm")
	votedData, _ := sv.storage.Get("votedFor")
	logData, _ := sv.storage.Get("log")

	d := gob.NewDecoder(bytes.NewBuffer(termData))
	d.Decode(&sv.Term)

	d = gob.NewDecoder(bytes.NewBuffer(votedData))
	d.Decode(&sv.VotedFor)

	d = gob.NewDecoder(bytes.NewBuffer(logData))
	d.Decode(&sv.Entries)
}

func (sv *Server) savePersistentState() {
	var termData bytes.Buffer
	if err := gob.NewEncoder(&termData).Encode(sv.Term); err != nil {
		log.Fatal(err)
	}
	sv.storage.Set("currentTerm", termData.Bytes())

	var votedData bytes.Buffer
	if err := gob.NewEncoder(&votedData).Encode(sv.VotedFor); err != nil {
		log.Fatal(err)
	}
	sv.storage.Set("votedFor", votedData.Bytes())

	var logData bytes.Buffer
	if err := gob.NewEncoder(&logData).Encode(sv.Entries); err != nil {
		log.Fatal(err)
	}
	sv.storage.Set("log", logData.Bytes())
}

type RoseStorage struct {
	DB *rosedb.DB
}

func NewRoseStorage(rose *rosedb.DB) *RoseStorage {
	return &RoseStorage{
		DB: rose,
	}
}

func (r RoseStorage) Set(key string, value []byte) {
	err := r.DB.Put([]byte(key), []byte(value))

	if err != nil {
		panic(err)
	}
}

func (r RoseStorage) Get(key string) ([]byte, bool) {
	ret, err := r.DB.Get([]byte(key))

	if err == rosedb.ErrKeyNotFound {
		return nil, false
	} else if err != nil {
		panic(err)
	}

	return ret, true
}

func (r RoseStorage) HasData() bool {
	return r.DB.Stat().KeysNum > 0
}

func (r RoseStorage) Close() error {
	return r.DB.Close()
}
