#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

static void show_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " <mode> <input> <output> <key>\n"
              << "Modes:\n"
              << "  xor-enc     Encrypt file using XOR cipher\n"
              << "  xor-dec     Decrypt file using XOR cipher\n"
              << "  caesar-enc  Encrypt file using Caesar cipher\n"
              << "  caesar-dec  Decrypt file using Caesar cipher\n"
              << "\nKey:\n"
              << "  XOR modes:    any string (used as repeating key)\n"
              << "  Caesar modes: integer shift value (e.g. 13)\n";
}

static void show_progress(std::uintmax_t current, std::uintmax_t total) {
    if (total == 0) return;
    int pct = static_cast<int>((current * 100) / total);
    std::cout << "\rProgress: " << pct << "% (" << current << "/" << total << " bytes)" << std::flush;
}

static bool xor_transform(const std::string& input_path, const std::string& output_path,
                           const std::string& key) {
    std::ifstream in(input_path, std::ios::binary | std::ios::ate);
    if (!in) {
        std::cerr << "Error: cannot open input file: " << input_path << '\n';
        return false;
    }

    auto file_size = static_cast<std::uintmax_t>(in.tellg());
    in.seekg(0);

    std::ofstream out(output_path, std::ios::binary);
    if (!out) {
        std::cerr << "Error: cannot open output file: " << output_path << '\n';
        return false;
    }

    const size_t buf_size = 8192;
    std::vector<char> buffer(buf_size);
    std::uintmax_t processed = 0;
    size_t key_idx = 0;

    while (in) {
        in.read(buffer.data(), static_cast<std::streamsize>(buf_size));
        auto bytes_read = static_cast<size_t>(in.gcount());
        if (bytes_read == 0) break;

        for (size_t i = 0; i < bytes_read; ++i) {
            buffer[i] ^= key[key_idx % key.size()];
            ++key_idx;
        }

        out.write(buffer.data(), static_cast<std::streamsize>(bytes_read));
        processed += bytes_read;

        if (file_size > 1024 * 1024) {
            show_progress(processed, file_size);
        }
    }

    if (file_size > 1024 * 1024) {
        std::cout << '\n';
    }
    return true;
}

static char caesar_shift_byte(char c, int shift) {
    auto uc = static_cast<unsigned char>(c);
    if (uc >= 'A' && uc <= 'Z') {
        return static_cast<char>('A' + (uc - 'A' + shift + 26) % 26);
    }
    if (uc >= 'a' && uc <= 'z') {
        return static_cast<char>('a' + (uc - 'a' + shift + 26) % 26);
    }
    return c;
}

static bool caesar_transform(const std::string& input_path, const std::string& output_path,
                              int shift) {
    std::ifstream in(input_path, std::ios::binary | std::ios::ate);
    if (!in) {
        std::cerr << "Error: cannot open input file: " << input_path << '\n';
        return false;
    }

    auto file_size = static_cast<std::uintmax_t>(in.tellg());
    in.seekg(0);

    std::ofstream out(output_path, std::ios::binary);
    if (!out) {
        std::cerr << "Error: cannot open output file: " << output_path << '\n';
        return false;
    }

    const size_t buf_size = 8192;
    std::vector<char> buffer(buf_size);
    std::uintmax_t processed = 0;

    while (in) {
        in.read(buffer.data(), static_cast<std::streamsize>(buf_size));
        auto bytes_read = static_cast<size_t>(in.gcount());
        if (bytes_read == 0) break;

        for (size_t i = 0; i < bytes_read; ++i) {
            buffer[i] = caesar_shift_byte(buffer[i], shift);
        }

        out.write(buffer.data(), static_cast<std::streamsize>(bytes_read));
        processed += bytes_read;

        if (file_size > 1024 * 1024) {
            show_progress(processed, file_size);
        }
    }

    if (file_size > 1024 * 1024) {
        std::cout << '\n';
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        show_usage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];
    std::string input_path = argv[2];
    std::string output_path = argv[3];
    std::string key_str = argv[4];

    bool success = false;

    if (mode == "xor-enc" || mode == "xor-dec") {
        if (key_str.empty()) {
            std::cerr << "Error: XOR key must not be empty.\n";
            return 1;
        }
        std::cout << (mode == "xor-enc" ? "Encrypting" : "Decrypting")
                  << " with XOR cipher...\n";
        success = xor_transform(input_path, output_path, key_str);
    } else if (mode == "caesar-enc") {
        int shift;
        try {
            shift = std::stoi(key_str);
        } catch (...) {
            std::cerr << "Error: Caesar key must be an integer.\n";
            return 1;
        }
        std::cout << "Encrypting with Caesar cipher (shift=" << shift << ")...\n";
        success = caesar_transform(input_path, output_path, shift);
    } else if (mode == "caesar-dec") {
        int shift;
        try {
            shift = std::stoi(key_str);
        } catch (...) {
            std::cerr << "Error: Caesar key must be an integer.\n";
            return 1;
        }
        std::cout << "Decrypting with Caesar cipher (shift=" << shift << ")...\n";
        success = caesar_transform(input_path, output_path, -shift);
    } else {
        std::cerr << "Error: unknown mode '" << mode << "'\n";
        show_usage(argv[0]);
        return 1;
    }

    if (success) {
        std::cout << "Done. Output written to: " << output_path << '\n';
        return 0;
    }
    return 1;
}
