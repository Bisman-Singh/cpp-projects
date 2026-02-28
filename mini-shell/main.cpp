#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#ifdef __APPLE__
#include <limits.h>
#else
#include <linux/limits.h>
#endif

static const size_t MAX_HISTORY = 20;
static std::deque<std::string> command_history;

static std::string get_prompt() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        return std::string(cwd) + " $ ";
    }
    return "minishell $ ";
}

static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

struct Command {
    std::vector<std::string> args;
    std::string input_file;
    std::string output_file;
    bool append_output = false;
};

static std::vector<Command> parse_pipeline(const std::vector<std::string>& tokens) {
    std::vector<Command> commands;
    Command current;

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == "|") {
            if (!current.args.empty()) {
                commands.push_back(std::move(current));
                current = Command{};
            }
        } else if (tokens[i] == ">" && i + 1 < tokens.size()) {
            current.output_file = tokens[++i];
            current.append_output = false;
        } else if (tokens[i] == ">>" && i + 1 < tokens.size()) {
            current.output_file = tokens[++i];
            current.append_output = true;
        } else if (tokens[i] == "<" && i + 1 < tokens.size()) {
            current.input_file = tokens[++i];
        } else {
            current.args.push_back(tokens[i]);
        }
    }
    if (!current.args.empty()) {
        commands.push_back(std::move(current));
    }
    return commands;
}

static bool handle_builtin(const Command& cmd) {
    if (cmd.args.empty()) return false;
    const std::string& name = cmd.args[0];

    if (name == "exit") {
        std::cout << "Goodbye!\n";
        std::exit(0);
    }

    if (name == "cd") {
        const char* target = nullptr;
        if (cmd.args.size() > 1) {
            target = cmd.args[1].c_str();
        } else {
            target = std::getenv("HOME");
        }
        if (target && chdir(target) != 0) {
            std::perror("cd");
        }
        return true;
    }

    if (name == "pwd") {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd))) {
            std::cout << cwd << '\n';
        } else {
            std::perror("pwd");
        }
        return true;
    }

    if (name == "history") {
        for (size_t i = 0; i < command_history.size(); ++i) {
            std::cout << "  " << (i + 1) << "  " << command_history[i] << '\n';
        }
        return true;
    }

    if (name == "help") {
        std::cout << "Mini Shell - Built-in commands:\n"
                  << "  cd [dir]    Change directory (default: $HOME)\n"
                  << "  pwd         Print working directory\n"
                  << "  history     Show last " << MAX_HISTORY << " commands\n"
                  << "  help        Show this help message\n"
                  << "  exit        Exit the shell\n\n"
                  << "Features:\n"
                  << "  cmd1 | cmd2    Pipe output of cmd1 to cmd2\n"
                  << "  cmd > file     Redirect stdout to file\n"
                  << "  cmd >> file    Append stdout to file\n"
                  << "  cmd < file     Redirect stdin from file\n";
        return true;
    }

    return false;
}

static void execute_command(const Command& cmd) {
    std::vector<char*> argv;
    for (const auto& arg : cmd.args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    if (!cmd.input_file.empty()) {
        int fd = open(cmd.input_file.c_str(), O_RDONLY);
        if (fd < 0) {
            std::perror(cmd.input_file.c_str());
            std::exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (!cmd.output_file.empty()) {
        int flags = O_WRONLY | O_CREAT | (cmd.append_output ? O_APPEND : O_TRUNC);
        int fd = open(cmd.output_file.c_str(), flags, 0644);
        if (fd < 0) {
            std::perror(cmd.output_file.c_str());
            std::exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    execvp(argv[0], argv.data());
    std::perror(argv[0]);
    std::exit(127);
}

static void run_pipeline(const std::vector<Command>& commands) {
    if (commands.empty()) return;

    if (commands.size() == 1 && handle_builtin(commands[0])) {
        return;
    }

    size_t n = commands.size();
    std::vector<int> pids;

    // prev_fd tracks the read-end of the previous pipe
    int prev_fd = -1;

    for (size_t i = 0; i < n; ++i) {
        int pipefd[2] = {-1, -1};
        if (i + 1 < n) {
            if (pipe(pipefd) < 0) {
                std::perror("pipe");
                return;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            std::perror("fork");
            return;
        }

        if (pid == 0) {
            if (prev_fd != -1) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }
            if (pipefd[1] != -1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }
            execute_command(commands[i]);
        }

        pids.push_back(pid);

        if (prev_fd != -1) close(prev_fd);
        if (pipefd[1] != -1) close(pipefd[1]);
        prev_fd = pipefd[0];
    }

    if (prev_fd != -1) close(prev_fd);

    for (int p : pids) {
        int status;
        waitpid(p, &status, 0);
    }
}

int main() {
    std::string line;

    while (true) {
        std::cout << get_prompt() << std::flush;
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }

        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
        if (trimmed.empty()) continue;

        command_history.push_back(trimmed);
        if (command_history.size() > MAX_HISTORY) {
            command_history.pop_front();
        }

        auto tokens = tokenize(trimmed);
        auto commands = parse_pipeline(tokens);
        run_pipeline(commands);
    }

    return 0;
}
