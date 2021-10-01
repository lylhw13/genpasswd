import hashlib

def gerneratePass(input, num):
    for _ in range(num):
        seed = hashlib.sha512(input.encode('utf-8'))
        input = seed.hexdigest()
    print(num)
    print(seed.hexdigest())
        
if __name__ == "__main__":
    input = "hello world!"
    num = int(3)
    gerneratePass(input, num)