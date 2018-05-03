package main

import (
	"encoding/json"
	"fmt"
	"github.com/gorilla/mux"
	"log"
	"net/http"
	"os"
)

type GetResponse struct {
	Success bool
	Value   string
}

type StoreResponse struct {
	Success bool
}

type StoreRequest struct {
	Value string
}

// DATA table: key -> value mapping
var m map[string]string

func GetValue(w http.ResponseWriter, r *http.Request) {
	key := mux.Vars(r)["key"]
	value, exist := m[key]
	json.NewEncoder(w).Encode(GetResponse{exist, value})
}

func StoreValue(w http.ResponseWriter, r *http.Request) {
	key := mux.Vars(r)["key"]
	var req StoreRequest
	err := json.NewDecoder(r.Body).Decode(&req)
	defer r.Body.Close()
	if err != nil {
		fmt.Printf("%s\n", err)
		json.NewEncoder(w).Encode(StoreResponse{false})
	} else {
		m[key] = req.Value
		json.NewEncoder(w).Encode(StoreResponse{true})
	}
}

func main() {
	m = make(map[string]string)
	router := mux.NewRouter()
	router.HandleFunc("/get/{key}", GetValue).Methods("GET")
	router.HandleFunc("/store/{key}", StoreValue).Methods("POST")
	log.Fatal(http.ListenAndServe(":"+os.Getenv("PORT"), router))
}
