# DVT Tess
## to parse TS sample mux1-cp.ts
`node tools parse`

## Docker
pour construire l'image : 
```
docker build -dtv .
```

pour pousser l'image sur le cloud
```
docker tag dtv thecodeisgreen/dtv:latest
docker push thecodeisgreen/dtv:latest
```

### Utilisation dans https://labs.play-with-docker.com/
```
docker pull thecodeisgreen/dtv:latest
docker run -ti thecodeisgreen/dtv node tools parse mux1-cp.ts
docker run -ti thecodeisgreen/dtv node tools parse mux1-cp.ts --table PMT
```
# TSDuck

## Utilisation dans https://labs.play-with-docker.com/
```
docker pull thecodeisgreen/tsduck:latest
docker run -ti thecodeisgreen/tsduck analyze mux1-cp.ts
```
