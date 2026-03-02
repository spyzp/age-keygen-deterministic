# THIS PROJECT IS UNMAINTAINED

For detailed information, disclaimers, etc, please see original tool README at https://github.com/keisentraut/age-keygen-deterministic/blob/main/README.md

## Compile

The original utility https://github.com/keisentraut/age-keygen-deterministic was rewritten in C.
Just clone this repository, install dependencies (libargon2 and libcrypto) and run ```make```.

## Usage

```
USAGE:
    ./age-keygen-deterministic [OPTIONS]

OPTIONS:
    -c, --count <COUNT>            number of keys to generate [default: 1]
    -o, --offset <OFFSET>          starting index of the keys [default: 0]
    -s, --stdin                    read passphrase from stdin
    -d, --doublecheck-passphrase   confirm passphrase if entered interactively
    -h, --help                     print this help information
```

If you only need a single age key, just use it without any command line arguments.
If you want to generate more than one secret key, then you can simply use the ```--count``` argument.

## Example run

If you use the passphrase ```example-passphrase-do-not-use!``` and want to generate the deterministic secret keys #2, #3 and #4, then just run the commands below:

```
$ make
$ ./age-keygen-deterministic -c 3 -o 2
Enter passphrase: 
# secret key 2 below
AGE-SECRET-KEY-1RSWAHJR48AWPN6HHTVVGXN7X3X0YWWA7TM7H22T7TF35EZPPVHHQ7WYGRZ
# secret key 3 below
AGE-SECRET-KEY-144T9ZKX0HK6CMMGYEN6WPN82Q4K9LVR376NUJF33HKVAQ70TXMHSPV96MY
# secret key 4 below
AGE-SECRET-KEY-1FMPVFDE9WD8CSTNS4J3QRNQ5VRTFE8973FVJ2JANT56HEPZTKA4SQZZ84R
```

The public key for any of those private keys can be obtained by piping the secret key to ```age-keygen -y```.

```
$ echo AGE-SECRET-KEY-1VZ3CREDN87LLHYDVS6FK36EZEVWNZGGFFSWZDN7DL0J04WG723MQCZUS9Q | age-keygen -y
age1z568mysf7kulsml0rxt6vxp3h26hjmgcmdpz8x6dfh0zlazspquqqawzn4
``` 

## Encryption/decryption sample

To prepare public key for encryption: ```age-keygen-deterministic -d | tail -n 1 | age-keygen -y > public.key```

Then you can encrypt files with ```age -R public.key -o README.md.age README.md```

To decrypt file with password without storing the private key: ```age-keygen-deterministic | tail -n 1 | age -d -i - README.md.age > README.md.decrypted```
