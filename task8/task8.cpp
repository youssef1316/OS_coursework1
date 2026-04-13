#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <ctime>

using namespace std;

//linking to the c file
extern "C" {
    #include "des.h"
}


//take the result from the encryption to convert it into readable hexadecimal
string to_hex(const unsigned char* data, size_t length) { //accept only a byte of data and repeat multiple times

    stringstream ss;
    //pad missing chars with 0
    ss << hex << setfill('0');
    for (size_t i = 0; i < length; ++i) {
        //forces each char to be treated as an integer not ascii
        ss << setw(2) << static_cast<int>(data[i]);
    }
    //converts the made stream into a string
    return ss.str();
}

//generating 8 random plain text passwords
string generate_random_password() {
    //all possible chars
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    //empty password string
    string pwd = "";
    //loop to fill the password
    for (int i = 0; i < 8; ++i) {
        pwd += charset[rand() % (sizeof(charset) - 1)];
    }
    return pwd;
}

//core implementation
string custom_crypt(const string& password, uint16_t salt) {
    //8 byte array to hold the cryptographic key
    unsigned char key[8] = {0};
    //plain text to be encrypted
    unsigned char block[8] = {0};
    //intermediate encryption results
    unsigned char temp_block[8] = {0};
    //loop 8 times with early stopping if password is short
    for (size_t i = 0; i < 8 && i < password.length(); ++i) {
        //left shift and zeroing the parity bit and stores in the array
        key[i] = password[i] << 1;
    }
    //allocate memory to hold the 16 sub keys needed by the DES algo
    key_set key_sets[17];
    //reset the memory to insure no garbage values
    memset(key_sets, 0, sizeof(key_sets));
    //calls the C file to generate the keys
    generate_sub_keys(key, key_sets);


    // Apply the salt
    des_set_salt(salt);
    // 25 Iterations of DES
    for (int i = 0; i < 25; ++i) {
        process_message(block, temp_block, key_sets, ENCRYPTION_MODE);
        //copy the encrypted result into the block for the next iteration
        memcpy(block, temp_block, 8);
    }
    // Format final string: Salt (4 hex chars) + Hash (16 hex chars)
    stringstream result;
    result << hex << setw(4) << setfill('0') << salt;
    result << to_hex(block, 8);

    return result.str();
}

//simulates actual login
bool verify_password(const string& plaintext, const string& hashed_string) {
    //extracts the first 4 chars (salt)
    if (hashed_string.length() < 4) return false; //verification if the stored is less than 4

    //extracting the salt
    string salt_hex = hashed_string.substr(0, 4);
    //converts the salt into 16 bit unsigned int
    uint16_t extracted_salt = static_cast<uint16_t>(stoul(salt_hex, nullptr, 16));

    //hashes the input password
    string computed_hash = custom_crypt(plaintext, extracted_salt);
    //check if the hashes are similar (simulate grant access)
    return computed_hash == hashed_string;
}

int main() {
    //seeding the random number generator using the current time
    srand(static_cast<unsigned>(time(nullptr)));

    //simulating 10 passwords
    for (int i = 1; i <= 10; ++i) {
        //generating the passwords
        string password = generate_random_password();
        //random number and keeping it to a max of 16 bit
        uint16_t salt = rand() % 65536;

        //hash the password (sign up)
        string hashed = custom_crypt(password, salt);
        //verify the password (sign in)
        bool is_valid = verify_password(password, hashed);

        cout << "Item " << setw(2) << i << ":\n"
             << "  Password : " << password << "\n"
             << "  Salt     : 0x" << hex << setw(4) << setfill('0') << salt << dec << "\n"
             << "  Hash     : " << hashed << "\n"
             << "  Verified : " << (is_valid ? "Pass" : "FAIL") << "\n\n";
    }

    return 0;
}