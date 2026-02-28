#include <iostream>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

struct Point { int x, y; };

enum class Direction { UP, DOWN, LEFT, RIGHT };

class SnakeGame {
public:
    SnakeGame() : width_(40), height_(20), score_(0), game_over_(false), dir_(Direction::RIGHT) {
        snake_.push_back({width_ / 2, height_ / 2});
        snake_.push_back({width_ / 2 - 1, height_ / 2});
        snake_.push_back({width_ / 2 - 2, height_ / 2});
        spawn_food();
    }

    void run() {
        enable_raw_mode();
        hide_cursor();
        clear_screen();

        while (!game_over_) {
            process_input();
            update();
            render();

            int delay = std::max(50, 150 - score_ * 5);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }

        show_cursor();
        disable_raw_mode();

        set_cursor(1, height_ + 4);
        std::cout << "Game Over! Final Score: " << score_ << "\n";
    }

private:
    int width_, height_, score_;
    bool game_over_;
    Direction dir_;
    std::deque<Point> snake_;
    Point food_{};

    struct termios orig_termios_{};

    void enable_raw_mode() {
        tcgetattr(STDIN_FILENO, &orig_termios_);
        struct termios raw = orig_termios_;
        raw.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    void disable_raw_mode() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios_);
    }

    void clear_screen() { std::cout << "\033[2J"; }
    void hide_cursor() { std::cout << "\033[?25l"; }
    void show_cursor() { std::cout << "\033[?25h"; }
    void set_cursor(int x, int y) { std::cout << "\033[" << y << ";" << x << "H"; }

    void process_input() {
        char c;
        Direction new_dir = dir_;
        while (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == '\033') {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) != 1) continue;
                if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;
                if (seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A': new_dir = Direction::UP; break;
                        case 'B': new_dir = Direction::DOWN; break;
                        case 'C': new_dir = Direction::RIGHT; break;
                        case 'D': new_dir = Direction::LEFT; break;
                    }
                }
            } else {
                switch (c) {
                    case 'w': case 'W': new_dir = Direction::UP; break;
                    case 's': case 'S': new_dir = Direction::DOWN; break;
                    case 'a': case 'A': new_dir = Direction::LEFT; break;
                    case 'd': case 'D': new_dir = Direction::RIGHT; break;
                    case 'q': case 'Q': game_over_ = true; return;
                }
            }
        }

        bool opposite =
            (new_dir == Direction::UP && dir_ == Direction::DOWN) ||
            (new_dir == Direction::DOWN && dir_ == Direction::UP) ||
            (new_dir == Direction::LEFT && dir_ == Direction::RIGHT) ||
            (new_dir == Direction::RIGHT && dir_ == Direction::LEFT);

        if (!opposite) dir_ = new_dir;
    }

    void update() {
        Point head = snake_.front();
        switch (dir_) {
            case Direction::UP:    head.y--; break;
            case Direction::DOWN:  head.y++; break;
            case Direction::LEFT:  head.x--; break;
            case Direction::RIGHT: head.x++; break;
        }

        if (head.x < 0 || head.x >= width_ || head.y < 0 || head.y >= height_) {
            game_over_ = true;
            return;
        }

        for (const auto& seg : snake_) {
            if (seg.x == head.x && seg.y == head.y) {
                game_over_ = true;
                return;
            }
        }

        snake_.push_front(head);

        if (head.x == food_.x && head.y == food_.y) {
            score_++;
            spawn_food();
        } else {
            snake_.pop_back();
        }
    }

    void spawn_food() {
        while (true) {
            food_.x = std::rand() % width_;
            food_.y = std::rand() % height_;
            bool on_snake = false;
            for (const auto& seg : snake_) {
                if (seg.x == food_.x && seg.y == food_.y) {
                    on_snake = true;
                    break;
                }
            }
            if (!on_snake) break;
        }
    }

    void render() {
        set_cursor(1, 1);

        std::string buf;
        buf.reserve(static_cast<size_t>((width_ + 3) * (height_ + 3)));

        buf += "\033[1;36m";
        buf += "+" + std::string(static_cast<size_t>(width_), '-') + "+\n";
        buf += "\033[0m";

        for (int y = 0; y < height_; ++y) {
            buf += "\033[1;36m|\033[0m";
            for (int x = 0; x < width_; ++x) {
                if (is_head(x, y)) {
                    buf += "\033[1;32m@\033[0m";
                } else if (is_snake(x, y)) {
                    buf += "\033[32mo\033[0m";
                } else if (x == food_.x && y == food_.y) {
                    buf += "\033[1;31m*\033[0m";
                } else {
                    buf += ' ';
                }
            }
            buf += "\033[1;36m|\033[0m\n";
        }

        buf += "\033[1;36m";
        buf += "+" + std::string(static_cast<size_t>(width_), '-') + "+\n";
        buf += "\033[0m";

        buf += " Score: \033[1;33m" + std::to_string(score_) + "\033[0m";
        buf += "  |  Speed: \033[1;33m" + std::to_string(std::max(50, 150 - score_ * 5)) + "ms\033[0m";
        buf += "  |  \033[90mWASD/Arrows to move, Q to quit\033[0m";

        std::cout << buf << std::flush;
    }

    bool is_head(int x, int y) const {
        return snake_.front().x == x && snake_.front().y == y;
    }

    bool is_snake(int x, int y) const {
        for (size_t i = 1; i < snake_.size(); ++i) {
            if (snake_[i].x == x && snake_[i].y == y) return true;
        }
        return false;
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    SnakeGame game;
    game.run();
    return 0;
}
