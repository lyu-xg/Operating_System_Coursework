package main

import (
	"bytes"
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

type StatusResponse struct {
	Success bool
}

type StoreRequest struct {
	Value string
}

type Worker struct {
	Loc string
	Rem int // remaining ram
}

// DATA table: key -> worker mapping
var m map[string]Worker

var Workers []Worker

func AddWorker(w http.ResponseWriter, r *http.Request) {
	var worker Worker
	err := json.NewDecoder(r.Body).Decode(&worker)
	// fmt.Println(r.Body)

	// b, err := ioutil.ReadAll(r.Body)

	if err != nil {

		fmt.Printf("add worker request err: %s\n", err)
		json.NewEncoder(w).Encode(StatusResponse{false})
	}

	defer r.Body.Close()
	Workers = append(Workers, worker) // does not check for dup
	json.NewEncoder(w).Encode(StatusResponse{true})
}

func MostLeisureWorker() int {
	rem := -1
	res := -1
	for i, w := range Workers {
		if w.Rem >= rem {
			res = i
			rem = w.Rem
		}
	}
	return res
}

func WorkerGetValue(worker Worker, key string) (string, bool) {
	r, e := http.Get(worker.Loc + "/get/" + key)
	if e != nil {
		fmt.Printf("Failed to fetch %s from worker at %s\n", key, worker.Loc)
		fmt.Printf("%s\n", e)
		return "", false
	} else {
		defer r.Body.Close()
		var res GetResponse
		json.NewDecoder(r.Body).Decode(&res)
		return res.Value, res.Success
	}

}

func GetValue(w http.ResponseWriter, r *http.Request) {
	key := mux.Vars(r)["key"]
	worker, exist := m[key]
	if !exist {
		json.NewEncoder(w).Encode(GetResponse{Success: false})
	} else {
		value, ok := WorkerGetValue(worker, key)
		json.NewEncoder(w).Encode(GetResponse{ok, value})
	}
}

func StoreValue(w http.ResponseWriter, r *http.Request) {
	key := mux.Vars(r)["key"]
	var req StoreRequest
	err := json.NewDecoder(r.Body).Decode(&req)
	defer r.Body.Close()
	if err != nil {
		fmt.Printf("%s\n", err)
		json.NewEncoder(w).Encode(StatusResponse{false})
	} else {
		// fmt.Printf("Storing %s: %s\n", key, req.Value)
		selected_worker := &Workers[MostLeisureWorker()]
		m[key] = *selected_worker
		// Do a post for this selected_worker.
		payload, _ := json.Marshal(req)
		res, e := http.Post(selected_worker.Loc+"/store/"+key, "application/json", bytes.NewBuffer(payload))
		if e != nil {
			fmt.Printf("Failed to store %s from worker at %s\n", key, selected_worker.Loc)
			fmt.Printf("%s\n", e)
			json.NewEncoder(w).Encode(StatusResponse{false})
		} else {
			var status StatusResponse
			json.NewDecoder(res.Body).Decode(&status)
			json.NewEncoder(w).Encode(status)
			if status.Success {
				selected_worker.Rem -= len([]byte(req.Value))
			}
		}
	}
}

func main() {
	m = make(map[string]Worker)
	router := mux.NewRouter()
	router.HandleFunc("/get/{key}", GetValue).Methods("GET")
	router.HandleFunc("/store/{key}", StoreValue).Methods("POST")
	router.HandleFunc("/add/worker", AddWorker).Methods("POST")
	log.Fatal(http.ListenAndServe(":"+os.Getenv("PORT"), router))
}
