#pragma once  // Ensures this header file is included only once during compilation.

// Includes the CoreMinimal.h header file, which is a central part of the Unreal Engine framework.
// This header file includes essential core definitions, macros, and types used throughout Unreal Engine.
// It simplifies includes by aggregating commonly used minimal core components, such as fundamental types,
// utilities, and standard libraries, reducing the need for multiple individual header file includes.
#include "CoreMinimal.h"

#include <iostream>    // Provides input and output functionalities (e.g., std::cout for logging).
#include <sstream>     // Provides std::stringstream for parsing and processing strings.
#include <string>      // Provides std::string for handling text.
#include <vector>      // Provides std::vector for handling dynamic arrays.
#include <mutex>       // Provides std::mutex for thread safety.

// Platform-specific includes
#ifdef _WIN32
#include <windows.h>    // Windows-specific headers for process and pipe management.
#else
#include <unistd.h>     // POSIX headers for process control (fork, pipe, exec).
#include <sys/wait.h>   // Provides waitpid to handle child process termination.
#include <fcntl.h>      // Provides file control options (e.g., non-blocking mode for pipes).
#endif

class LIVINGROOM_API Stockfish {
public:
    // Retrieve the single instance of the Stockfish class (singleton pattern)
    static Stockfish& getInstance();

    // Requests Stockfish to analyze a position based on skill level and FEN string.
    // Returns the results from Stockfish as a vector of strings.
    std::vector<std::string> requestStockfish(const int& skillLevel, const std::string& fen);

    // Closes the handles to the pipes and the Stockfish process.
    // This is called when Stockfish is no longer needed.
    void closeStockfish();

    private:
    #ifdef _WIN32
        SECURITY_ATTRIBUTES saAttr;    // Security attributes for process and pipe creation.
        STARTUPINFO si;                // STARTUPINFO structure for process creation.
        PROCESS_INFORMATION pi;        // PROCESS_INFORMATION structure for process creation.
        HANDLE hStdinRead = NULL;      // Handle for reading from the standard input pipe.
        HANDLE hStdinWrite = NULL;     // Handle for writing to the standard input pipe.
        HANDLE hStdoutRead = NULL;     // Handle for reading from the standard output pipe.
        HANDLE hStdoutWrite = NULL;    // Handle for writing to the standard output pipe.
        HANDLE hStderrRead = NULL;     // Handle for reading from the standard error pipe.
        HANDLE hStderrWrite = NULL;    // Handle for writing to the standard error pipe.
        HANDLE stockfishMutex = NULL;  // Mutex handle to prevent multiple instances.

        // Converts a standard string (narrow string) to a wide string (std::wstring).
        std::wstring convertToWideString(const std::string& str);
    #else
        int stdinPipe[2];   // Pipe for writing commands to Stockfish's standard input.
        int stdoutPipe[2];  // Pipe for reading Stockfish's standard output.
        int stderrPipe[2]; // Pipe for Stockfish's standard errors output.
        pid_t stockfishPid;  // Process ID of the Stockfish engine.
        pthread_mutex_t stockfishMutex = PTHREAD_MUTEX_INITIALIZER;  // POSIX mutex
        int lockFileDescriptor = -1;   // File descriptor for lock (macOS/Linux)
        int hStdinRead = 0;      // Handle for reading from the standard input pipe.
        int hStdinWrite = 0;     // Handle for writing to the standard input pipe.
        int hStdoutRead = 0;     // Handle for reading from the standard output pipe.
        int hStdoutWrite = 0;    // Handle for writing to the standard output pipe.
        int hStderrRead = 0;     // Handle for reading from the standard error pipe.
    #endif

    // Starts the Stockfish engine with the specified skill level.
    // Sets up pipes for communication and launches the Stockfish process.
    void startStockfish(const int& skillLevel);

    // Sends a command to Stockfish through the standard input pipe.
    // Ensures the command ends with a newline and handles errors.
    void sendStockfishCommand(const std::string& str);

    // Retrieves the results from Stockfish after sending a command.
    // This function collects output from Stockfish for various commands.
    std::vector<std::string> getStockfishResults(const std::string& command);

    // Reads output from Stockfish through the standard output pipe.
    // Collects and returns the output as a vector of strings.
    std::vector<std::string> readStockfishOutput();

    // Prevent direct instantiation
    Stockfish() {}

    // Prevent copy construction and assignment
    Stockfish(const Stockfish&) = delete;
    Stockfish& operator=(const Stockfish&) = delete;

    // Function to check if Stockfish is already running
    bool isStockfishAlreadyRunning();
};

