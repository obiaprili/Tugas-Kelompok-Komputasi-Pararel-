import hashlib
import json
import random
import string
import os

def generate_wordlist(size=1000):
    common = [
        "password", "123456", "123456789", "qwerty", "admin",
        "letmein", "welcome", "monkey", "dragon", "master",
        "football", "baseball", "whatever", "shadow", "superman",
        "iloveyou", "princess", "rockyou", "abc123", "nicole"
    ]
    
    random_words = []
    for i in range(size - len(common)):
        word = ''.join(random.choices(string.ascii_lowercase, k=random.randint(4, 10)))
        random_words.append(word)
    
    return common + random_words

def generate_hashes(wordlist):
    hashes = []
    for word in wordlist:
        sha256_hash = hashlib.sha256(word.encode()).hexdigest()
        md5_hash = hashlib.md5(word.encode()).hexdigest()
        hashes.append({
            "word": word,
            "sha256": sha256_hash,
            "md5": md5_hash
        })
    return hashes

def save_datasets(wordlist, hashes):
    if not os.path.exists("wordlist"):
        os.makedirs("wordlist")
    
    with open("wordlist/wordlist_1000.txt", "w") as f:
        f.write("\n".join(wordlist))
    
    targets = [{"sha256": h["sha256"], "md5": h["md5"]} for h in hashes]
    with open("target_hashes.json", "w") as f:
        json.dump(targets, f, indent=2)
    
    with open("hash_mapping.json", "w") as f:
        json.dump(hashes, f, indent=2)
    
    print(f"Generated {len(wordlist)} passwords")
    print(f"Saved to wordlist/wordlist_1000.txt")
    print(f"Saved to target_hashes.json")
    print(f"Saved to hash_mapping.json (for validation only)")

if __name__ == "__main__":
    print("Generating hash dataset...")
    wordlist = generate_wordlist(1000)
    hashes = generate_hashes(wordlist)
    save_datasets(wordlist, hashes)
    print("\nDataset siap digunakan!")