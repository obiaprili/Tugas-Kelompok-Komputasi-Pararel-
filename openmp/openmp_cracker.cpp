#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <omp.h>

class SHA256 {
private:
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    
    uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    
    uint32_t rightRotate(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }
    
public:
    std::string hash(const std::string& input) {
        uint32_t hh[8];
        for(int i = 0; i < 8; i++) hh[i] = h[i];
        
        std::vector<uint8_t> msg(input.begin(), input.end());
        msg.push_back(0x80);
        
        while(msg.size() % 64 != 56) {
            msg.push_back(0x00);
        }
        
        uint64_t bit_len = input.size() * 8;
        for(int i = 7; i >= 0; i--) {
            msg.push_back((bit_len >> (i * 8)) & 0xFF);
        }
        
        for(size_t i = 0; i < msg.size(); i += 64) {
            uint32_t w[64];
            
            for(int j = 0; j < 16; j++) {
                w[j] = (msg[i + j*4] << 24) | (msg[i + j*4 + 1] << 16) |
                       (msg[i + j*4 + 2] << 8) | (msg[i + j*4 + 3]);
            }
            
            for(int j = 16; j < 64; j++) {
                uint32_t s0 = rightRotate(w[j-15], 7) ^ rightRotate(w[j-15], 18) ^ (w[j-15] >> 3);
                uint32_t s1 = rightRotate(w[j-2], 17) ^ rightRotate(w[j-2], 19) ^ (w[j-2] >> 10);
                w[j] = w[j-16] + s0 + w[j-7] + s1;
            }
            
            uint32_t a = hh[0], b = hh[1], c = hh[2], d = hh[3];
            uint32_t e = hh[4], f = hh[5], g = hh[6], h_ = hh[7];
            
            for(int j = 0; j < 64; j++) {
                uint32_t S1 = rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25);
                uint32_t ch = (e & f) ^ ((~e) & g);
                uint32_t temp1 = h_ + S1 + ch + k[j] + w[j];
                uint32_t S0 = rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = S0 + maj;
                
                h_ = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }
            
            hh[0] += a; hh[1] += b; hh[2] += c; hh[3] += d;
            hh[4] += e; hh[5] += f; hh[6] += g; hh[7] += h_;
        }
        
        std::stringstream ss;
        for(int i = 0; i < 8; i++) {
            ss << std::hex << std::setw(8) << std::setfill('0') << hh[i];
        }
        return ss.str();
    }
};

class MD5 {
private:
    uint32_t s[64] = {
        7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
        5,9,14,20, 5,9,14,20, 5,9,14,20, 5,9,14,20,
        4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
        6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21
    };
    
    uint32_t K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };
    
    uint32_t leftRotate(uint32_t x, uint32_t n) {
        return (x << n) | (x >> (32 - n));
    }
    
public:
    std::string hash(const std::string& input) {
        uint32_t a0 = 0x67452301;
        uint32_t b0 = 0xefcdab89;
        uint32_t c0 = 0x98badcfe;
        uint32_t d0 = 0x10325476;
        
        std::vector<uint8_t> msg(input.begin(), input.end());
        msg.push_back(0x80);
        
        while(msg.size() % 64 != 56) {
            msg.push_back(0x00);
        }
        
        uint64_t bit_len = input.size() * 8;
        for(int i = 0; i < 8; i++) {
            msg.push_back((bit_len >> (i * 8)) & 0xFF);
        }
        
        for(size_t i = 0; i < msg.size(); i += 64) {
            uint32_t M[16];
            for(int j = 0; j < 16; j++) {
                M[j] = (msg[i + j*4]) | (msg[i + j*4 + 1] << 8) |
                       (msg[i + j*4 + 2] << 16) | (msg[i + j*4 + 3] << 24);
            }
            
            uint32_t A = a0, B = b0, C = c0, D = d0;
            
            for(int j = 0; j < 64; j++) {
                uint32_t F, g;
                if(j < 16) {
                    F = (B & C) | ((~B) & D);
                    g = j;
                } else if(j < 32) {
                    F = (D & B) | ((~D) & C);
                    g = (5*j + 1) % 16;
                } else if(j < 48) {
                    F = B ^ C ^ D;
                    g = (3*j + 5) % 16;
                } else {
                    F = C ^ (B | (~D));
                    g = (7*j) % 16;
                }
                
                uint32_t temp = D;
                D = C;
                C = B;
                B = B + leftRotate(A + F + K[j] + M[g], s[j]);
                A = temp;
            }
            
            a0 += A;
            b0 += B;
            c0 += C;
            d0 += D;
        }
        
        std::stringstream ss;
        uint32_t parts[] = {a0, b0, c0, d0};
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                ss << std::hex << std::setw(2) << std::setfill('0') 
                   << ((parts[i] >> (j*8)) & 0xFF);
            }
        }
        return ss.str();
    }
};

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   OPENMP HASH CRACKER (PARALLEL CPU)" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    int num_threads;
    #pragma omp parallel
    {
        #pragma omp single
        {
            num_threads = omp_get_num_threads();
            std::cout << "OpenMP Threads: " << num_threads << std::endl;
        }
    }
    std::cout << "CPU Cores available: " << omp_get_num_procs() << std::endl << std::endl;
    
    int hash_type;
    std::cout << "Pilih jenis hash:" << std::endl;
    std::cout << "1. SHA256" << std::endl;
    std::cout << "2. MD5" << std::endl;
    std::cout << "Pilihan (1/2): ";
    std::cin >> hash_type;
    
    std::string target_hash;
    std::cout << "Masukkan target hash: ";
    std::cin >> target_hash;
    
    std::ifstream wordlist_file("../hash_generator/wordlist/wordlist_1000.txt");
    if(!wordlist_file.is_open()) {
        std::cerr << "Error: Tidak bisa membuka wordlist!" << std::endl;
        return 1;
    }
    
    std::vector<std::string> dictionary;
    std::string word;
    while(std::getline(wordlist_file, word)) {
        dictionary.push_back(word);
    }
    wordlist_file.close();
    
    std::cout << std::endl << "Wordlist: " << dictionary.size() << " kata" << std::endl;
    std::cout << "Target hash: " << target_hash << std::endl;
    std::cout << std::endl << "Mencari password dengan " << num_threads << " thread..." << std::endl << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::string found_password = "";
    int attempts = 0;
    
    if(hash_type == 1) {
        SHA256 sha256;
        
        #pragma omp parallel for shared(found_password, dictionary) schedule(dynamic, 10)
        for(int i = 0; i < (int)dictionary.size(); i++) {
            if(!found_password.empty()) continue;
            
            std::string computed_hash = sha256.hash(dictionary[i]);
            
            if(computed_hash == target_hash) {
                #pragma omp critical
                {
                    if(found_password.empty()) {
                        found_password = dictionary[i];
                    }
                }
            }
            
            #pragma omp atomic
            attempts++;
        }
    } else {
        MD5 md5;
        
        #pragma omp parallel for shared(found_password, dictionary) schedule(dynamic, 10)
        for(int i = 0; i < (int)dictionary.size(); i++) {
            if(!found_password.empty()) continue;
            
            std::string computed_hash = md5.hash(dictionary[i]);
            
            if(computed_hash == target_hash) {
                #pragma omp critical
                {
                    if(found_password.empty()) {
                        found_password = dictionary[i];
                    }
                }
            }
            
            #pragma omp atomic
            attempts++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(end - start);
    
    std::cout << std::endl << "========================================" << std::endl;
    if(found_password != "") {
        std::cout << "PASSWORD DITEMUKAN!" << std::endl;
        std::cout << "   Password: " << found_password << std::endl;
    } else {
        std::cout << "PASSWORD TIDAK DITEMUKAN dalam wordlist" << std::endl;
    }
    std::cout << "   Attempts: " << attempts << " kata" << std::endl;
    std::cout << "   Waktu: " << duration.count() << " detik" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}