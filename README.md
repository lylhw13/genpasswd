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


procedure
1. parse argument
2. configure file hash times aes key start iv start password length   // to do later
3. generate a random password
4. password hash + tag hash
5. hash times greater than 1e6
6. basic procedure

Two type of passwd:
- Passwd for database. 

    The passwd need user to input. It will be used to access database and generate passwd for specific tag.

- Passwd for tag

    The passwd is generate for specific tag.

```sh
$ ./genpasswd.out 
Random passwd is fdf4ab4f24eb0e75
$ ./genpasswd.out -i
init the database
New password: 
Retype new password: 
Init records successed
$ ./genpasswd.out -t google
genpasswd for google, and add it to database
Password: 
The password for tag google is cf048d00fab0f14f
```