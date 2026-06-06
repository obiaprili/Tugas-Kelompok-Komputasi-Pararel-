#!/usr/bin/env python3
import grpc
from concurrent import futures
import hashlib
import time
import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)))

import hash_cracker_pb2 as hash_cracker_pb2
import hash_cracker_pb2_grpc as hash_cracker_pb2_grpc

class HashCrackerServicer(hash_cracker_pb2_grpc.HashCrackerServicer):
    def __init__(self, wordlist_path, num_workers):
        with open(wordlist_path, 'r') as f:
            self.wordlist = [line.strip() for line in f.readlines()]
        
        self.num_workers = num_workers
        self.chunk_size = len(self.wordlist) // num_workers
        self.assigned_workers = set()
        self.found_password = None
        self.start_time = None
        
        print(f"Wordlist loaded: {len(self.wordlist)} kata")
        print(f"Workers: {num_workers}")
        print(f"Chunk size per worker: {self.chunk_size}")
    
    def GetWork(self, request, context):
        worker_id = request.worker_id
        
        if worker_id in self.assigned_workers:
            return hash_cracker_pb2.WorkResponse(words=[], start_index=-1)
        
        self.assigned_workers.add(worker_id)
        
        start_idx = worker_id * self.chunk_size
        end_idx = start_idx + self.chunk_size
        
        if worker_id == self.num_workers - 1:
            end_idx = len(self.wordlist)
        
        words = self.wordlist[start_idx:end_idx]
        
        print(f"Worker {worker_id} mendapat {len(words)} kata (index {start_idx}-{end_idx-1})")
        
        if len(self.assigned_workers) == self.num_workers and self.start_time is None:
            self.start_time = time.time()
            print(f"\nSemua worker terassign! Memulai cracking...\n")
        
        return hash_cracker_pb2.WorkResponse(
            words=words,
            start_index=start_idx
        )
    
    def SubmitResult(self, request, context):
        if self.found_password is None and request.found_password:
            self.found_password = request.found_password
            elapsed = time.time() - self.start_time if self.start_time else 0
            
            print(f"\n{'='*50}")
            print(f"PASSWORD DITEMUKAN!")
            print(f"   Password: {request.found_password}")
            print(f"   Hash: {request.target_hash}")
            print(f"   Ditemukan oleh: Worker {request.worker_id}")
            print(f"   Waktu: {elapsed:.6f} detik")
            print(f"{'='*50}\n")
        
        return hash_cracker_pb2.ResultResponse(
            success=True,
            message="Result received"
        )

def serve():
    wordlist_path = os.environ.get('WORDLIST_PATH', '../hash_generator/wordlist/wordlist_1000.txt')
    num_workers = int(os.environ.get('NUM_WORKERS', '3'))
    port = os.environ.get('PORT', '50051')
    target_hash = os.environ.get('TARGET_HASH', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918')
    
    print("\n" + "="*50)
    print("   DISTRIBUTED HASH CRACKER - COORDINATOR")
    print("="*50)
    print(f"Port: {port}")
    print(f"Workers: {num_workers}")
    print(f"Target hash: {target_hash}")
    print("="*50 + "\n")
    
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    servicer = HashCrackerServicer(wordlist_path, num_workers)
    hash_cracker_pb2_grpc.add_HashCrackerServicer_to_server(servicer, server)
    
    server.add_insecure_port(f'[::]:{port}')
    server.start()
    
    print(f"Coordinator running on port {port}")
    print(f"Menunggu {num_workers} worker terhubung...\n")
    
    try:
        server.wait_for_termination()
    except KeyboardInterrupt:
        print("\nShutting down coordinator...")
        server.stop(0)

if __name__ == '__main__':
    serve()