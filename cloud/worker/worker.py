#!/usr/bin/env python3
import grpc
import hashlib
import sys
import os
import time

sys.path.append(os.path.dirname(os.path.abspath(__file__)))

import hash_cracker_pb2 as hash_cracker_pb2
import hash_cracker_pb2_grpc as hash_cracker_pb2_grpc

def sha256_hash(text):
    return hashlib.sha256(text.encode()).hexdigest()

def run_worker(worker_id, coordinator_addr, target_hash):
    print("\n" + "="*50)
    print(f"   WORKER {worker_id} - DISTRIBUTED HASH CRACKER")
    print("="*50)
    
    channel = grpc.insecure_channel(coordinator_addr)
    stub = hash_cracker_pb2_grpc.HashCrackerStub(channel)
    
    print(f"Worker {worker_id} starting...")
    print(f"Connected to coordinator at {coordinator_addr}")
    print(f"Target hash: {target_hash}")
    
    response = stub.GetWork(hash_cracker_pb2.WorkRequest(worker_id=worker_id))
    
    if not response.words:
        print(f"Worker {worker_id} tidak mendapat kerjaan")
        return
    
    print(f"Worker {worker_id} mendapat {len(response.words)} kata")
    
    found = None
    start_time = time.time()
    
    for i, word in enumerate(response.words):
        if i > 0 and i % 100 == 0:
            print(f"   Worker {worker_id}: {i}/{len(response.words)} kata ({i*100/len(response.words):.0f}%)")
        
        computed_hash = sha256_hash(word)
        
        if computed_hash == target_hash:
            found = word
            elapsed = time.time() - start_time
            print(f"\nWorker {worker_id} menemukan password!")
            print(f"   Password: {word}")
            print(f"   Waktu lokal: {elapsed:.6f} detik")
            break
    
    if found:
        stub.SubmitResult(hash_cracker_pb2.ResultRequest(
            worker_id=worker_id,
            found_password=found,
            target_hash=target_hash
        ))
    else:
        print(f"Worker {worker_id} selesai, password tidak ditemukan di chunk-nya")
    
    print(f"Worker {worker_id} selesai\n")

if __name__ == '__main__':
    worker_id = int(os.environ.get('WORKER_ID', '0'))
    coordinator_addr = os.environ.get('COORDINATOR_ADDR', 'localhost:50051')
    target_hash = os.environ.get('TARGET_HASH', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918')
    
    run_worker(worker_id, coordinator_addr, target_hash)