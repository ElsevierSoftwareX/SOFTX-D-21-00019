#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include "../gc-7.2/include/gc.h"
#include "../simulator/initialize.h"
#include "../protocol/protocol.h"
#include "../utils/heap.h"
#include "../utils/array.h"
#include "findRoute.h"
#include "../global.h"
#define INF UINT16_MAX
#define HOPSLIMIT 20

//FIXME: non globale ma passato per riferimento a dijkstra
char error[100];
uint32_t** dist;
PathHop** next;


/* void initializeFindRoute() { */
/*   distanceHeap=NULL; */
/*   previousPeer = NULL; */
/*   distance = NULL; */
/* } */

int present(long i) {
  long p;
  Payment *payment;

  for(p=0; p<paymentIndex; p++) {
    payment = hashTableGet(payments, p);
    if(i==payment->sender || i == payment->receiver) return 1;
  }

  return 0;
}

int compareDistance(Distance* a, Distance* b) {
  uint16_t d1, d2;
  d1=a->distance;
  d2=b->distance;
  if(d1==d2)
    return 0;
  else if(d1<d2)
    return -1;
  else
    return 1;
}

/* version for not all peers (doesn't work)
void floydWarshall() {
  long i, j, k, p;
  ChannelInfo* channelInfo;
  Channel* direction1, *direction2;
  PathHop *hop1, *hop2;
  uint16_t* d, *newd, dij, dik, dkj;
  Payment* payment;


  distht = hashTableInitialize(peerIndex);
  nextht = hashTableInitialize(peerIndex);

  for(i=0; i<channelInfoIndex; i++) {
    channelInfo = hashTableGet(channelInfos, i);
    direction1 = hashTableGet(channels, channelInfo->channelDirection1);
    direction2 = hashTableGet(channels, channelInfo->channelDirection2);

    hashTableMatrixPut(distht, channelInfo->peer1, channelInfo->peer2, &(direction1->policy.timelock));
    hashTableMatrixPut(distht, channelInfo->peer2, channelInfo->peer1, &(direction2->policy.timelock));

    hop1 = malloc(sizeof(PathHop));
    hop1->sender = channelInfo->peer1;
    hop1->receiver = channelInfo->peer2;
    hop1->channel = channelInfo->channelDirection1;
    hashTableMatrixPut(nextht, channelInfo->peer1, channelInfo->peer2, hop1);

    hop2 = malloc(sizeof(PathHop));
    hop2->sender = channelInfo->peer2;
    hop2->receiver = channelInfo->peer1;
    hop2->channel = channelInfo->channelDirection2;
    hashTableMatrixPut(nextht, channelInfo->peer2, channelInfo->peer1, hop2);
  }

  long* peersPay;
  peersPay = malloc(sizeof(long)*2*paymentIndex);
  for(i=0; i<paymentIndex; i++) {
    payment = hashTableGet(payments, i);
    j = i*2;
    peersPay[j] = payment->sender;
    peersPay[j+1] = payment->receiver;
  }


  for(k=0; k<peerIndex; k++) {
    for(p=0; p<paymentIndex*2; p++){
      i = peersPay[p];
      for(j=0; j<peerIndex; j++) {

        d = hashTableMatrixGet(distht, i, j);
        if(d==NULL && i!=j)
          dij = INF;
        else if (d==NULL && i==j)
          dij = 0;
        else 
          dij = *d;

        d = hashTableMatrixGet(distht, i, k);
        if(d==NULL && i!=k)
          dik = INF;
        else if (d==NULL && i==k)
          dik = 0;
        else{
          dik = *d;
          //  printf("%u\n", dik);
        }

        d = hashTableMatrixGet(distht, k, j);
        if(d==NULL && k!=j)
          dkj = INF;
        else if (d==NULL && k==j)
          dkj = 0;
        else{
          dkj = *d;
          //        printf("%u\n", dkj);
    }



        if(dij > dik + dkj) {
          newd = malloc(sizeof(uint16_t));
          *newd = dik+dkj;
          hashTableMatrixPut(distht, i, j, newd);
          hashTableMatrixPut(nextht, i, j, hashTableMatrixGet(nextht, i, k));
        }

      }
    }
    printf("%ld\n", k);
    }

}


Array* getPath(long source, long destination) {
  Array* path;
  long nextPeer;
  PathHop* hop;

  hop = hashTableMatrixGet(nextht, source, destination);

  if(hop == NULL) {
    return NULL;
  }

  path = arrayInitialize(10);

  path = arrayInsert(path, hop);
  nextPeer = hop->receiver;
  while(nextPeer != destination ) {
    hop = hashTableMatrixGet(nextht, nextPeer, destination);
    path = arrayInsert(path, hop);
    nextPeer = hop->receiver;
  }

  if(arrayLen(path)>HOPSLIMIT)
    return NULL;

  return path;
}
*/


void floydWarshall() {
  long i, j, k;
  ChannelInfo* channelInfo;
  Channel* direction1, *direction2;

  dist = malloc(sizeof(uint32_t*)*peerIndex);
  next = malloc(sizeof(PathHop*)*peerIndex);
  //  paths = malloc(sizeof(Array**)*peerIndex);
  for(i=0; i<peerIndex; i++) {
    dist[i] = malloc(sizeof(uint32_t)*peerIndex);
    next[i] = malloc(sizeof(PathHop)*peerIndex);
    //paths[i] = malloc(sizeof(Array*)*peerIndex);
  }


  for(i=0; i<peerIndex; i++){
    for(j=0; j<peerIndex; j++) {
      if(i==j)
        dist[i][j] = 0;
      else
        dist[i][j] = INF;

      next[i][j].channel = -1;
      //      paths[i][j] = arrayInitialize(10);
    }
  }

  for(i=0; i<channelInfoIndex; i++) {
    channelInfo = hashTableGet(channelInfos, i);
    direction1 = hashTableGet(channels, channelInfo->channelDirection1);
    direction2 = hashTableGet(channels, channelInfo->channelDirection2);
    dist[channelInfo->peer1][channelInfo->peer2] = direction1->policy.timelock;
    dist[channelInfo->peer2][channelInfo->peer1] = direction2->policy.timelock;
    next[channelInfo->peer1][channelInfo->peer2].sender = channelInfo->peer1;
    next[channelInfo->peer1][channelInfo->peer2].receiver = channelInfo->peer2;
    next[channelInfo->peer1][channelInfo->peer2].channel = channelInfo->channelDirection1;
    next[channelInfo->peer2][channelInfo->peer1].sender = channelInfo->peer2;
    next[channelInfo->peer2][channelInfo->peer1].receiver = channelInfo->peer1;
    next[channelInfo->peer2][channelInfo->peer1].channel = channelInfo->channelDirection2;
  }

  for(k=0; k<peerIndex; k++) {
    for(i=0; i<peerIndex; i++){
      for(j=0; j<peerIndex; j++){
        if(dist[i][j] > dist[i][k] + dist[k][j]) {
          dist[i][j] = dist[i][k] + dist[k][j];
          next[i][j] = next[i][k];
        }
      }
    }
  }

  /*
  for(i=0; i<paymentIndex; i++) {
    payment = hashTableGet(payments, i);
    source = payment->sender;
    destination = payment->receiver;

    if(next[source][destination].channel==-1)
      continue;

    paths[source][destination] = arrayInsert(paths[source][destination], &(next[source][destination]));
    nextPeer = next[source][destination].receiver;
    while(nextPeer != destination ) {
      paths[source][destination] = arrayInsert(paths[source][destination], &(next[nextPeer][destination]));
      nextPeer = next[nextPeer][destination].receiver;
    }

    if(arrayLen(paths[source][destination])>HOPSLIMIT)
      arrayDeleteAll(paths[source][destination]);
  }
*/

}

Array* getPath(long source, long destination) {
  Array* path;
  long nextPeer;

  if(next[source][destination].channel==-1) {
    return NULL;
  }
  path = arrayInitialize(10);

  path = arrayInsert(path, &(next[source][destination]));
  nextPeer = next[source][destination].receiver;
  while(nextPeer != destination ) {
    path = arrayInsert(path, &(next[nextPeer][destination]));
    nextPeer = next[nextPeer][destination].receiver;
  }

  if(arrayLen(path)>HOPSLIMIT)
    return NULL;

  return path;
}

Distance **distance;
DijkstraHop ** previousPeer;
Heap** distanceHeap;
//Array** hops;

void initializeDijkstra() {
  int i;

  distance = malloc(sizeof(Distance*)*PARALLEL);
  previousPeer = malloc(sizeof(DijkstraHop*)*PARALLEL);
  distanceHeap = malloc(sizeof(Heap*)*PARALLEL);
  //  hops = malloc(sizeof(Array*)*4);

  for(i=0; i<PARALLEL; i++) {
    distance[i] = malloc(sizeof(Distance)*peerIndex);
    previousPeer[i] = malloc(sizeof(DijkstraHop)*peerIndex);
    distanceHeap[i] = heapInitialize(channelIndex);
    //    hops[i] = arrayInitialize(HOPSLIMIT);
    /* for(j=0; j<HOPSLIMIT; j++) { */
    /*   hop = malloc(sizeof(PathHop)); */
    /*   arrayInsert(hops[i], hop); */
    /* } */
  }

}

Array* dijkstraP(long source, long target, uint64_t amount, Array* ignoredPeers, Array* ignoredChannels, long p) {
  Distance *d=NULL;
  long i, bestPeerID, j,*channelID=NULL, nextPeerID, prev;
  Peer* bestPeer=NULL;
  Channel* channel=NULL;
  ChannelInfo* channelInfo=NULL;
  uint32_t tmpDist;
  uint64_t capacity;
  PathHop* hop=NULL;
  Array* hops=NULL;

  //  printf("DIJKSTRA\n");

  /* pthread_mutex_lock(&peersMutex); */
  /* distance = malloc(sizeof(Distance)*peerIndex); */

  /* previousPeer[p] = malloc(sizeof(DijkstraHop)*peerIndex); */

  /* distanceHeap[p] = heapInitialize(peerIndex/10); */
  /* pthread_mutex_unlock(&peersMutex); */

  while(heapLen(distanceHeap[p])!=0) {
    heapPop(distanceHeap[p], compareDistance);
    //    printf("popping\n");
  }

  //  hops[p]->index = HOPSLIMIT;

  for(i=0; i<peerIndex; i++){
    distance[p][i].peer = i;
    distance[p][i].distance = INF;
    previousPeer[p][i].channel = -1;
    previousPeer[p][i].peer = -1;
  }

  distance[p][source].peer = source;
  distance[p][source].distance = 0;

  //TODO: e' safe passare l'inidrizzo dell'i-esimo elemento dell'array?
  //  pthread_mutex_lock(&peersMutex);
  distanceHeap[p] =  heapInsert(distanceHeap[p], &distance[p][source], compareDistance);
  //pthread_mutex_unlock(&peersMutex);

  while(heapLen(distanceHeap[p])!=0) {
    d = heapPop(distanceHeap[p], compareDistance);
    bestPeerID = d->peer;
    if(bestPeerID==target) break;

    pthread_mutex_lock(&peersMutex);
    bestPeer = hashTableGet(peers, bestPeerID);
    pthread_mutex_unlock(&peersMutex);

    for(j=0; j<arrayLen(bestPeer->channel); j++) {
      channelID = arrayGet(bestPeer->channel, j);
      if(channelID==NULL) continue;

      pthread_mutex_lock(&peersMutex);
      channel = hashTableGet(channels, *channelID);
      pthread_mutex_unlock(&peersMutex);

      nextPeerID = channel->counterparty;

      if(isPresent(nextPeerID, ignoredPeers)) continue;
      if(isPresent(*channelID, ignoredChannels)) continue;

      tmpDist = distance[p][bestPeerID].distance + channel->policy.timelock;

      pthread_mutex_lock(&peersMutex);
      channelInfo = hashTableGet(channelInfos, channel->channelInfoID);
      pthread_mutex_unlock(&peersMutex);

      capacity = channelInfo->capacity;

      if(tmpDist < distance[p][nextPeerID].distance && amount<=capacity) {
        distance[p][nextPeerID].peer = nextPeerID;
        distance[p][nextPeerID].distance = tmpDist;

        previousPeer[p][nextPeerID].channel = *channelID;
        previousPeer[p][nextPeerID].peer = bestPeerID;

        //    pthread_mutex_lock(&peersMutex);
        distanceHeap[p] = heapInsert(distanceHeap[p], &distance[p][nextPeerID], compareDistance);
        //pthread_mutex_unlock(&peersMutex);
      }
      }

    }

  if(previousPeer[p][target].peer == -1) {
    //    printf ("no path available!\n");
    //    strcpy(error, "noPath");
    return NULL;
  }


  i=0;
  //pthread_mutex_lock(&peersMutex);
  hops=arrayInitialize(HOPSLIMIT);
  //pthread_mutex_unlock(&peersMutex);
  prev=target;
  while(prev!=source) {
   /*  pthread_mutex_lock(&peersMutex); */
   /*  hop = arrayGet(hops[p], i); */
   /*  if(hop==NULL) printf("index here: %ld\n", hops[p]->index); */
   /*  hop->channel = previousPeer[p][prev].channel; */
   /*  hop->sender = previousPeer[p][prev].peer; */

   /* channel = hashTableGet(channels, hop->channel); */

   /*  hop->receiver = channel->counterparty; */
   /*  pthread_mutex_unlock(&peersMutex); */
   /*  prev = previousPeer[p][prev].peer; */
   /*  i++; */
   /*  if(i>=HOPSLIMIT) return NULL; */
    pthread_mutex_lock(&peersMutex);
    channel = hashTableGet(channels, previousPeer[p][prev].channel);
    pthread_mutex_unlock(&peersMutex);

    //pthread_mutex_lock(&peersMutex);
    hop = malloc(sizeof(PathHop));
    //pthread_mutex_unlock(&peersMutex);

    hop->channel = previousPeer[p][prev].channel;
    hop->sender = previousPeer[p][prev].peer;
    hop->receiver = channel->counterparty;

    //pthread_mutex_lock(&peersMutex);
    hops=arrayInsert(hops, hop );
    //pthread_mutex_unlock(&peersMutex);

    prev = previousPeer[p][prev].peer;

  }

  //  hops[p]->index = i;

  if(arrayLen(hops)>HOPSLIMIT) {
    return NULL;
  }

  arrayReverse(hops);

  /* pthread_mutex_lock(&peersMutex); */
  /* free(previousPeer[p]); */
  /* free(distance); */
  /* heapFree(distanceHeap[p]); */
  /* pthread_mutex_unlock(&peersMutex); */

  return hops;
}


Array* dijkstra(long source, long target, uint64_t amount, Array* ignoredPeers, Array* ignoredChannels) {
  Distance *d=NULL;
  long i, bestPeerID, j,*channelID=NULL, nextPeerID, prev;
  Peer* bestPeer=NULL;
  Channel* channel=NULL;
  ChannelInfo* channelInfo=NULL;
  uint32_t tmpDist;
  uint64_t capacity;
  Array* hops=NULL;
  PathHop* hop=NULL;

  printf("DIJKSTRA\n");

  while(heapLen(distanceHeap[0])!=0) 
    heapPop(distanceHeap[0], compareDistance);

  for(i=0; i<peerIndex; i++){
    distance[0][i].peer = i;
    distance[0][i].distance = INF;
    previousPeer[0][i].channel = -1;
    previousPeer[0][i].peer = -1;
  }

  distance[0][source].peer = source;
  distance[0][source].distance = 0;

  //TODO: e' safe passare l'inidrizzo dell'i-esimo elemento dell'array?
  distanceHeap[0] =  heapInsert(distanceHeap[0], &distance[0][source], compareDistance);

  while(heapLen(distanceHeap[0])!=0) {
    d = heapPop(distanceHeap[0], compareDistance);
    bestPeerID = d->peer;
    if(bestPeerID==target) break;

    bestPeer = hashTableGet(peers, bestPeerID);

    for(j=0; j<arrayLen(bestPeer->channel); j++) {
      channelID = arrayGet(bestPeer->channel, j);
      if(channelID==NULL) continue;

      channel = hashTableGet(channels, *channelID);

      nextPeerID = channel->counterparty;

      if(isPresent(nextPeerID, ignoredPeers)) continue;
      if(isPresent(*channelID, ignoredChannels)) continue;

      tmpDist = distance[0][bestPeerID].distance + channel->policy.timelock;

      channelInfo = hashTableGet(channelInfos, channel->channelInfoID);

      capacity = channelInfo->capacity;

      if(tmpDist < distance[0][nextPeerID].distance && amount<=capacity) {
        distance[0][nextPeerID].peer = nextPeerID;
        distance[0][nextPeerID].distance = tmpDist;

        previousPeer[0][nextPeerID].channel = *channelID;
        previousPeer[0][nextPeerID].peer = bestPeerID;

        distanceHeap[0] = heapInsert(distanceHeap[0], &distance[0][nextPeerID], compareDistance);
      }
      }

    }

  if(previousPeer[0][target].peer == -1) {
    //    printf ("no path available!\n");
    //    strcpy(error, "noPath");
    return NULL;
  }


  hops=arrayInitialize(HOPSLIMIT);
  prev=target;
  while(prev!=source) {
    //    printf("%ld ", previousPeer[0][prev].peer);
    hop = malloc(sizeof(PathHop));
    hop->channel = previousPeer[0][prev].channel;
    hop->sender = previousPeer[0][prev].peer;

   channel = hashTableGet(channels, hop->channel);

    hop->receiver = channel->counterparty;
    hops=arrayInsert(hops, hop );
    prev = previousPeer[0][prev].peer;
  }


  if(arrayLen(hops)>HOPSLIMIT) {
    //    strcpy(error, "limitExceeded");
    return NULL;
  }

  arrayReverse(hops);

  return hops;
}

int isSamePath(Array*rootPath, Array*path) {
  long i;
  PathHop* hop1, *hop2;

  for(i=0; i<arrayLen(rootPath); i++) {
    hop1=arrayGet(rootPath, i);
    hop2=arrayGet(path, i);
    if(hop1->channel != hop2->channel)
      return 0;
  }

  return 1;
}

int comparePath(Array* path1, Array* path2){
  long len1, len2;
  len1 = arrayLen(path1);
  len2=arrayLen(path2);

  if(len1==len2) return 0;
  else if (len1<len2) return -1;
  else return 1;
}

Array* findPaths(long source, long target, double amount){
  Array* startingPath, *firstPath, *prevShortest, *rootPath, *path, *spurPath, *newPath, *nextShortestPath;
  Array* ignoredChannels, *ignoredPeers;
 PathHop *hop;
  long i, k, j, spurNode, newPathLen;
  Array* shortestPaths;
  Heap* candidatePaths;

  candidatePaths = heapInitialize(100);

  ignoredPeers=arrayInitialize(2);
  ignoredChannels=arrayInitialize(2);

  shortestPaths = arrayInitialize(100);

  startingPath=dijkstra(source, target, amount, ignoredPeers, ignoredChannels);
  if(startingPath==NULL) return NULL;

  firstPath = arrayInitialize(arrayLen(startingPath)+1);
  hop = malloc(sizeof(PathHop));
  hop->receiver = source;
  firstPath=arrayInsert(firstPath, hop);
  for(i=0; i<arrayLen(startingPath); i++) {
    hop = arrayGet(startingPath, i);
    firstPath=arrayInsert(firstPath, hop);
  }

  shortestPaths = arrayInsert(shortestPaths, firstPath);

  for(k=1; k<100; k++) {
    prevShortest = arrayGet(shortestPaths, k-1);

    for(i=0; i<arrayLen(prevShortest)-1; i++) {
      hop = arrayGet(prevShortest, i);
      spurNode = hop->receiver;

      //roothPath = prevshortest[:i+1]
      rootPath = arrayInitialize(i+1);
      for(j=0; j<i+1; j++) {
        hop = arrayGet(prevShortest, j);
        rootPath=arrayInsert(rootPath, hop);
      }

      for(j=0; j<arrayLen(shortestPaths); j++) {
        path=arrayGet(shortestPaths, j);
        if(arrayLen(path)>i+1) {
          if(isSamePath(rootPath, path)) {
            hop = arrayGet(path, i+1);
            ignoredChannels = arrayInsert(ignoredChannels, &(hop->channel));
          }
        }
      }

      for(j=0; j<arrayLen(rootPath); j++) {
        hop = arrayGet(rootPath, j);
        if(hop->receiver == spurNode) continue;
        ignoredPeers = arrayInsert(ignoredPeers, &(hop->receiver));
      }

      spurPath = dijkstra(spurNode, target, amount, ignoredPeers, ignoredChannels);

      if(spurPath==NULL) {
        if(strcmp(error, "noPath")==0) continue;
        else return NULL;
      }

      newPathLen = arrayLen(rootPath) + arrayLen(spurPath);
      newPath = arrayInitialize(newPathLen);
      for(j=0; j<arrayLen(rootPath); j++) {
        hop = arrayGet(rootPath, j);
        newPath = arrayInsert(newPath, hop);
      }
      for(j=0; j<arrayLen(spurPath); j++) {
        hop = arrayGet(spurPath, j);
        newPath = arrayInsert(newPath, hop);
      }

      heapInsert(candidatePaths, newPath, comparePath);
    }

    if(heapLen(candidatePaths)==0) break;

    nextShortestPath = heapPop(candidatePaths, comparePath);
    shortestPaths = arrayInsert(shortestPaths, nextShortestPath);
  }

  return shortestPaths;

}


Route* routeInitialize(long nHops) {
  Route* r;

  r = malloc(sizeof(Route));
  r->routeHops = arrayInitialize(nHops);
  r->totalAmount = 0.0;
  r->totalFee = 0.0;
  r->totalTimelock = 0;

  return r;
}

//TODO: sposta computeFee nei file protocol

Route* transformPathIntoRoute(Array* pathHops, uint64_t amountToSend, int finalTimelock) {
  PathHop *pathHop;
  RouteHop *routeHop, *nextRouteHop;
  Route *route;
  long nHops, i;
  uint64_t fee, currentChannelCapacity;
  Channel* channel;
  Policy currentChannelPolicy, nextChannelPolicy;
  ChannelInfo* channelInfo;

  nHops = arrayLen(pathHops);
  route = routeInitialize(nHops);

  for(i=nHops-1; i>=0; i--) {
    pathHop = arrayGet(pathHops, i);

    channel = hashTableGet(channels, pathHop->channel);
    currentChannelPolicy = channel->policy;
    channelInfo = hashTableGet(channelInfos,channel->channelInfoID);
    currentChannelCapacity = channelInfo->capacity;

    routeHop = malloc(sizeof(RouteHop));
    routeHop->pathHop = pathHop;

    if(i == arrayLen(pathHops)-1) {
      routeHop->amountToForward = amountToSend;
      route->totalAmount += amountToSend;

      if(nHops == 1) {
        routeHop->timelock = 0;
        route->totalTimelock = 0;
      }
      else {
        routeHop->timelock = currentChannelPolicy.timelock;
        route->totalTimelock += currentChannelPolicy.timelock;
      }
    }
    else {
      fee = computeFee(nextRouteHop->amountToForward, nextChannelPolicy);
      routeHop->amountToForward = nextRouteHop->amountToForward + fee;
      route->totalFee += fee;
      route->totalAmount += fee;

      routeHop->timelock = nextRouteHop->timelock + currentChannelPolicy.timelock;
      route->totalTimelock += currentChannelPolicy.timelock;
    }

    //TODO: mettere stringa con messaggio di errore tra i parametri della funzione
    if(routeHop->amountToForward > currentChannelCapacity)
      return NULL;

    route->routeHops = arrayInsert(route->routeHops, routeHop);
    nextChannelPolicy = currentChannelPolicy;
    nextRouteHop = routeHop;

    }

  arrayReverse(route->routeHops);

  return route;
  }


void printHop(RouteHop* hop){
  printf("Sender %ld, Receiver %ld, Channel %ld\n", hop->pathHop->sender, hop->pathHop->receiver, hop->pathHop->channel);
}

