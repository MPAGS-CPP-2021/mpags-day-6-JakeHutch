#include "CipherFactory.hpp"
#include "CipherMode.hpp"
#include "CipherType.hpp"
#include "ProcessCommandLine.hpp"
#include "TransformChar.hpp"
#include "CustomExceptions.hpp"

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <thread>

int main(int argc, char* argv[])
{
    // Convert the command-line arguments into a more easily usable form
    const std::vector<std::string> cmdLineArgs{argv, argv + argc};

    // Options that might be set by the command-line arguments
    ProgramSettings settings{
        false, false, "", "", "", CipherMode::Encrypt, CipherType::Caesar};

    // Process command line arguments
    try {
        processCommandLine(cmdLineArgs, settings);
    }catch (const MissingArgument& e){
        std::cerr << "[error] Missing argument: " << e.what() << std::endl;
        return 1;
    }catch (const UnknownArgument& e){
        std::cerr << "[error] Unknown argument: " << e.what() << std::endl;
        return 1;
    }catch (const InvalidKey& e){
        std::cerr << "[error] Invalid Key: " << e.what() << std::endl;
        return 1;
    }
    

    // // Any failure in the argument processing means we can't continue
    // // Use a non-zero return value to indicate failure
    // if (!cmdLineStatus) {
    //     return 1;
    // }

    // Handle help, if requested
    if (settings.helpRequested) {
        // Line splitting for readability
        std::cout
            << "Usage: mpags-cipher [-h/--help] [--version] [-i <file>] [-o <file>] [-c <cipher>] [-k <key>] [--encrypt/--decrypt]\n\n"
            << "Encrypts/Decrypts input alphanumeric text using classical ciphers\n\n"
            << "Available options:\n\n"
            << "  -h|--help        Print this help message and exit\n\n"
            << "  --version        Print version information\n\n"
            << "  -i FILE          Read text to be processed from FILE\n"
            << "                   Stdin will be used if not supplied\n\n"
            << "  -o FILE          Write processed text to FILE\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "  -c CIPHER        Specify the cipher to be used to perform the encryption/decryption\n"
            << "                   CIPHER can be caesar, playfair, or vigenere - caesar is the default\n\n"
            << "  -k KEY           Specify the cipher KEY\n"
            << "                   A null key, i.e. no encryption, is used if not supplied\n\n"
            << "  --encrypt        Will use the cipher to encrypt the input text (default behaviour)\n\n"
            << "  --decrypt        Will use the cipher to decrypt the input text\n\n"
            << std::endl;
        // Help requires no further action, so return from main
        // with 0 used to indicate success
        return 0;
    }

    // Handle version, if requested
    // Like help, requires no further action,
    // so return from main with zero to indicate success
    if (settings.versionRequested) {
        std::cout << "0.5.0" << std::endl;
        return 0;
    }

    // Initialise variables
    char inputChar{'x'};
    std::string inputText;

    // Read in user input from stdin/file
    if (!settings.inputFile.empty()) {
        // Open the file and check that we can read from it
        std::ifstream inputStream{settings.inputFile};
        if (!inputStream.good()) {
            std::cerr << "[error] failed to create istream on file '"
                      << settings.inputFile << "'" << std::endl;
            return 1;
        }

        // Loop over each character from the file
        while (inputStream >> inputChar) {
            inputText += transformChar(inputChar);
        }


    } else {
        // Loop over each character from user input
        // (until Return then CTRL-D (EOF) pressed)
        while (std::cin >> inputChar) {
            inputText += transformChar(inputChar);
        }
    }

    std::unique_ptr<Cipher> cipher ;
    // Request construction of the appropriate cipher
    try {
        cipher = cipherFactory(settings.cipherType, settings.cipherKey);
    }catch (const InvalidKey& e){
        std::cerr << "[error] Invalid Key: " << e.what() << std::endl;
        return 1;
    }
    

    // Check that the cipher was constructed successfully
    if (!cipher) {
        std::cerr << "[error] problem constructing requested cipher"
                  << std::endl;
        return 1;
    }

    // create variable to hold the number of threads we want to use and teh length of the input string
    std::size_t inputLength {inputText.size()};
    std::size_t numThreads {4};
    std::string outputText {""};

    //split inputText into chunks 
    std::vector<std::string> inputChunks {};
    for (std::size_t i{0}; i < numThreads; ++i) {
        if (i+1 == numThreads){
            inputChunks.push_back(inputText.substr( i * (inputLength/numThreads) , inputLength/numThreads + inputLength%numThreads));
        }else {inputChunks.push_back(inputText.substr( i * (inputLength/numThreads) , inputLength/numThreads));
        }
    }

    // // create lambda funtion for applying cipher
    // std::string applyCipherToChunk = [&] (const std::unique_ptr<Cipher>& cipherChunk, const std::string& inputChunk) {
    //    return cipherChunk->applyCipher(inputChunk, settings.cipherMode); 
    // };

    //loop over all threads
    std::vector< std::future< std::string > > futures;
    for (std::size_t i{0}; i < numThreads; ++i) {
        futures.push_back( std::async(std::launch::async,[&inputChunks, i, &cipher, &settings] (){
            return cipher->applyCipher(inputChunks[i], settings.cipherMode);
        } ));
    }

    // for (std::size_t i{0}; i < numThreads; ++i) {
    //     futures[i].wait_for(std::chrono::seconds(10));
    //     outputText+= futures[i].get();
    // }
    
    bool complete {false};
    do {
        complete = true ; 
        for (auto& future : futures) {
            auto status = future.wait_for(std::chrono::seconds(10));
            if (status != std::future_status::ready){
                complete = false ; 
            }
        }
    } while (!complete);

    for (auto& future : futures ) {
        outputText += future.get();
    }


    // // Run the cipher on the input text, specifying whether to encrypt/decrypt
    // const std::string outputText{cipher->applyCipher(inputText, settings.cipherMode)};

    // Output the encrypted/decrypted text to stdout/file
    if (!settings.outputFile.empty()) {
        // Open the file and check that we can write to it
        std::ofstream outputStream{settings.outputFile};
        if (!outputStream.good()) {
            std::cerr << "[error] failed to create ostream on file '"
                      << settings.outputFile << "'" << std::endl;
            return 1;
        }

        // Print the encrypted/decrypted text to the file
        outputStream << outputText << std::endl;

    } else {
        // Print the encrypted/decrypted text to the screen
        std::cout << outputText << std::endl;
    }

    // No requirement to return from main, but we do so for clarity
    // and for consistency with other functions
    return 0;
}
