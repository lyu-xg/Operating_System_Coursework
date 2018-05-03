PORT=8000 go run proxy.go &
PORT=8001 go run worker.go &
PORT=8002 go run worker.go &

sleep 3

curl localhost:8000/add/worker -d '{"Loc":"http://localhost:8001","Rem":1000}'
curl localhost:8000/add/worker -d '{"Loc":"http://localhost:8002","Rem":1000}'
curl localhost:8000/store/test -d '{"Value":"That"}'
curl localhost:8000/get/test

pkill -n worker; pkill -n worker
pkill -n proxy