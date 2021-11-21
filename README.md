# Profile
This is a program to generate a password base on a tag.  这是一个基于标签的密码生成本。

# Principle

Two type of passwd:
- Passwd for database. 

    The passwd need user to input. It will be used to access database and generate passwd for a specific tag.

    ```txt
    passwd for database -> sha512(passwd) -> passwd-hash -> key, iv -> aes encrypt database
    ```

- Passwd for tag

    The passwd is generate for the tag.

    ```txt
    passwd for database -> sha512(passwd + tag) -> new-hash -> first len chars as the passwd for tag
    ```
# Usage
```txt
Usage: genpasswd [OPTION][ARG]
   or: genpasswd [OPTION]

    -i          init the database
    -l          list all tags
    -t [ARG]    generate a passwd for the arg. And add the arg to the database
    -r [ARG]    remove the arg from the database
    -h          show usage

    without argument, the program will generate a random password
```

# Example
## Generate a random password
```sh
$ ./genpasswd
Random passwd is fdf4ab4f24eb0e75
```
## Initdatabase
```bash
./genpasswd -i
New password for database: 
Retype new password for database: 
Init records successed
```
## Generate a passwd for a tag
```bash
$ ./genpasswd -t google
Database Password: 
The password for google is cf048d00fab0f14f

$ ./genpasswd -t baidu
Database Password: 
The password for baidu is 7e106a931a7a6b0f
```
## List all tags
```bash
$ ./genpasswd -l
Database Password: 
1 baidu
2 google
```
## Remove one tag
```bash
$ ./genpasswd -r baidu
Database Password: 
remove baidu from database
```
