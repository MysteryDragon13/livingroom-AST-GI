#include "Stockfish.h"      // Declares the Stockfish class, which handles communication with the Stockfish engine, processes commands, and manages input/output handling.
#include <chrono>           // Provides tools for handling time-related functionality.
#include <iostream>         // Provides input and output functionalities (e.g., std::cout and std::cerr for logging and debugging).
#include <sstream>          // Provides std::stringstream for parsing and processing strings.
#include <stdio.h>          // Provides standard I/O functions (required for POSIX systems).
#include <string>           // Provides std::string for handling text.
#include <thread>           // Provides multithreading support for adding delays or sleeps.
#include <vector>           // Provides std::vector for storing dynamic arrays.

#ifdef _WIN32
#include <windows.h>        // Windows API functions for process and pipe handling.
#else
#include <unistd.h>         // POSIX functions for pipe and process handling on macOS/Linux.
#include <sys/wait.h>       // POSIX functions for waiting on child processes.
#include <fcntl.h>          // For file control options like O_NONBLOCK.
#endif

// Path to Stockfish executable
const char* path = "E:\\LivingRoom-5.5\\Source\\Stockfish\\Stockfish-Windows\\stockfish.exe";

#ifdef _WIN32
// This method uses the MultiByteToWideChar function from the Windows API to perform the conversion.
// The CP_ACP code page is used, which represents the system's default ANSI code page.
std::wstring Stockfish::convertToWideString(const std::string& str) {
    int len; // Variable to hold the length of the resulting wide string.
    int str_len = static_cast<int>(str.length() + 1); // Calculate the length of the input string including null terminator.

    // Get the required length of the wide string.
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str_len, 0, 0);

    // Create a wide string with the calculated length.
    std::wstring wstr(len, L'\0');

    // Perform the actual conversion from narrow to wide string.
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str_len, &wstr[0], len);

    return wstr; // Return the converted wide string.
}
#endif

// Singleton instance method to get the unique Stockfish object
Stockfish& Stockfish::getInstance() {
    static Stockfish instance;
    return instance;
}

// Check if the Stockfish process is already running using a mutex
bool Stockfish::isStockfishAlreadyRunning() {
#ifdef _WIN32
    stockfishMutex = CreateMutexW(NULL, TRUE, L"StockfishMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return true;  // Mutex already exists, meaning Stockfish is running
    }
    return false;
#else
    lockFileDescriptor = open("/tmp/stockfish.lock", O_CREAT | O_RDWR, 0666);  // Create lock file
    if (lockFileDescriptor == -1) {
        std::cerr << "Failed to create lock file\n";
        return true;  // Unable to open lock file, assuming Stockfish is running
    }
    if (flock(lockFileDescriptor, LOCK_EX | LOCK_NB) == -1) {
        close(lockFileDescriptor);
        return true;  // Could not obtain file lock, Stockfish is likely running
    }
    return false;  // Successfully obtained lock
#endif
}

// Start the Stockfish engine and configure its settings
void Stockfish::startStockfish(const int& skillLevel) {
    if (isStockfishAlreadyRunning()) {
        return;
    }

#ifdef _WIN32
    // Windows-specific code for creating pipes and launching Stockfish
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    ZeroMemory(&si, sizeof(si));  // Initialize process information
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create pipes for communication with Stockfish
    if (!CreatePipe(&hStdinRead, &hStdinWrite, &saAttr, 0) ||
        !CreatePipe(&hStdoutRead, &hStdoutWrite, &saAttr, 0) ||
        !CreatePipe(&hStderrRead, &hStderrWrite, &saAttr, 0)) {
        std::cerr << "CreatePipe failed (" << GetLastError() << ").\n";
        return;
    }

    si.hStdError = hStderrWrite;
    si.hStdOutput = hStdoutWrite;
    si.hStdInput = hStdinRead;
    si.dwFlags |= STARTF_USESTDHANDLES;

    std::wstring widePath = convertToWideString(path);

    // Start the Stockfish process
    if (!CreateProcess(widePath.c_str(), NULL, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
        return;
    }

#else
    // macOS/Linux-specific code for creating pipes and launching Stockfish
    if (pipe(this->stdinPipe) == -1 || pipe(this->stdoutPipe) == -1 || pipe(this->stderrPipe) == -1) {
        std::cerr << "Failed to create pipes.\n";
        return;
    }

    pid_t pid = fork();  // Create a new process
    if (pid == -1) {
        std::cerr << "Fork failed.\n";
        return;
    }

    if (pid == 0) {
        // In the child process: Replace stdin, stdout, stderr with pipes
        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);

        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stderrPipe[0]);

        execl(path, "stockfish", NULL);  // Launch Stockfish
        _exit(EXIT_FAILURE);  // Should never reach here if exec is successful
    }
    else {
        // In the parent process: Close unused pipe ends
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        hStdinWrite = stdinPipe[1];
        hStdoutRead = stdoutPipe[0];
        hStderrRead = stderrPipe[0];
    }
#endif

    // Configure Stockfish options
    sendStockfishCommand("setoption name Threads value 2\n");
    sendStockfishCommand("setoption name Skill Level value " + std::to_string(skillLevel) + "\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Small delay to let Stockfish initialize
}

// Send a command to Stockfish
void Stockfish::sendStockfishCommand(const std::string& str) {
    std::string commandWithNewline = str + "\n";  // Add newline to command
#ifdef _WIN32
    DWORD bytesWritten;
    if (!WriteFile(hStdinWrite, commandWithNewline.c_str(), static_cast<DWORD>(commandWithNewline.length()), &bytesWritten, NULL)) {
        std::cerr << "Failed to send command: " << str << " Error: " << GetLastError() << "\n";
    }
#else
    if (write(hStdinWrite, commandWithNewline.c_str(), commandWithNewline.length()) == -1) {
        std::cerr << "Failed to send command: " << str << "\n";
    }
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Wait for Stockfish to process the command
}

// Read Stockfish output and return it as a vector of strings
std::vector<std::string> Stockfish::readStockfishOutput() {
    std::vector<std::string> lines;
    char buffer[4096];
#ifdef _WIN32
    DWORD bytesRead;
    BOOL success;
    while (true) {
        success = ReadFile(hStdoutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (!success || bytesRead == 0) {
            if (GetLastError() == ERROR_BROKEN_PIPE) break;
            std::cerr << "Error reading from pipe: " << GetLastError() << "\n";
            break;
        }
        buffer[bytesRead] = '\0';  // Null-terminate the buffer
#else
    ssize_t bytesRead;
    while ((bytesRead = read(hStdoutRead, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';  // Null-terminate the buffer
#endif
        std::stringstream bufferStream(buffer);
        std::string line;
        while (std::getline(bufferStream, line)) {
            lines.push_back(line);
        }
#ifdef _WIN32
        if (bytesRead < sizeof(buffer) - 1) break;
    }
#else
        if (bytesRead < sizeof(buffer) - 1) break;
        if (bytesRead == -1 && errno != EAGAIN) {
            std::cerr << "Error reading from Stockfish.\n";
            break;
        }
    }
#endif

    return lines;
}

// Get Stockfish results after sending a command
std::vector<std::string> Stockfish::getStockfishResults(const std::string& command) {
    sendStockfishCommand(command);
    std::vector<std::string> response = readStockfishOutput();

    // Ensure Stockfish is ready to respond
    sendStockfishCommand("uci\n");
    std::vector<std::string> uciResponse = readStockfishOutput();
    response.insert(response.end(), uciResponse.begin(), uciResponse.end());

    sendStockfishCommand("isready\n");
    std::vector<std::string> isReadyResponse = readStockfishOutput();
    response.insert(response.end(), isReadyResponse.begin(), isReadyResponse.end());

    sendStockfishCommand("d\n");  // Display current position information
    std::vector<std::string> dResponse = readStockfishOutput();
    response.insert(response.end(), dResponse.begin(), dResponse.end());

    return response;
}

// Request Stockfish to analyze a position (FEN string) and return results
std::vector<std::string> Stockfish::requestStockfish(const int& skillLevel, const std::string& fen) {
    startStockfish(skillLevel);  // Start Stockfish with given skill level
    int depth = 0.5f * skillLevel;  // Adjust depth based on skill level
    depth = depth < 1 ? 1 : depth;
    std::string command = "position fen " + fen + "\n" + "go perft 1" + "\n" + "go depth " + std::to_string(depth);
    std::vector<std::string> response = getStockfishResults(command);
    return response;
}

// Close Stockfish handles and processes
void Stockfish::closeStockfish() {
#ifdef _WIN32
    if (stockfishMutex) CloseHandle(stockfishMutex);
#else
    if (lockFileDescriptor != -1) {
        flock(lockFileDescriptor, LOCK_UN);  // Unlock the file
        close(lockFileDescriptor);            // Close the file descriptor
    }
    pthread_mutex_destroy(&stockfishMutex); // Clean up mutex
#endif
}
