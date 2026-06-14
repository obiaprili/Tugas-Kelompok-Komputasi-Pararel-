#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <CL/cl.h>

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

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   OPENCL HASH CRACKER (PARALLEL)" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    cl_uint numPlatforms;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    
    if(numPlatforms == 0) {
        std::cerr << "Error: No OpenCL platform found!" << std::endl;
        std::cerr << "Install: sudo apt install pocl-opencl-icd" << std::endl;
        return 1;
    }
    
    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
    
    for(cl_uint i = 0; i < numPlatforms; i++) {
        char name[256];
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(name), name, nullptr);
        std::cout << "Platform " << i << ": " << name << std::endl;
    }
    
    cl_platform_id platform = platforms[0];
    
    cl_uint numDevices;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
    
    if(numDevices == 0) {
        std::cerr << "Error: No OpenCL device found!" << std::endl;
        return 1;
    }
    
    std::vector<cl_device_id> devices(numDevices);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices.data(), nullptr);
    
    for(cl_uint i = 0; i < numDevices; i++) {
        char name[256];
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(name), name, nullptr);
        std::cout << "Device " << i << ": " << name << std::endl;
    }
    
    cl_device_id device = devices[0];
    
    cl_int err;
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    
    int hash_type;
    std::cout << std::endl << "Pilih jenis hash:" << std::endl;
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
    std::cout << std::endl << "Mencari password dengan OpenCL..." << std::endl << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    SHA256 sha256;
    std::string found_password = "";
    int attempts = 0;
    
    for(const auto& pwd : dictionary) {
        attempts++;
        if(sha256.hash(pwd) == target_hash) {
            found_password = pwd;
            break;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(end - start);
    
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    
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
    
    std::cout << std::endl << "Catatan: OpenCL kernel masih dalam pengembangan." << std::endl;
    std::cout << "   Saat ini menggunakan serial SHA256 sebagai placeholder." << std::endl;
    
    return 0;
}