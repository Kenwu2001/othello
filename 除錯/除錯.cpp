#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Point {
    int x, y;
    Point() : Point(0, 0) {}
    Point(float x, float y) : x(x), y(y) {}
    bool operator==(const Point& rhs) const {
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const Point& rhs) const {
        return !operator==(rhs);
    }
    Point operator+(const Point& rhs) const {
        return Point(x + rhs.x, y + rhs.y);
    }
    Point operator-(const Point& rhs) const {
        return Point(x - rhs.x, y - rhs.y);
    }
};

int now_score;
const int me = 1;

class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{ { Point(-1, -1), Point(-1, 0), Point(-1, 1),
                                             Point(0, -1), /*{0, 0}, */ Point(0, 1),
                                             Point(1, -1), Point(1, 0), Point(1, 1) } };
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;

private:
    int get_next_player(int player) const {    //判斷現在誰是下棋者
        //std::cout << "now is " << 3 - player << " playing\n";
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {    //有無超出邊界
        //if ( 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE )
        //    std::cout << "cross over!, p.x = " << p.x << " p.y = " << p.y << '\n';
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {    //那格是黑或白或沒有棋子
        //std::cout << "check board[p.x][p.y] = board[" << p.x << "][" << p.y << "] = " << board[p.x][p.y] << '\n';
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {    //把黑或白棋放上某一格
        std::cout << disc << " set [" << p.x << "][" << p.y << "] = " << disc << '\n';
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(const Point& p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(const Point& center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (const Point& dir : directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(const Point& center) {
        for (const Point& dir : directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({ p });
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s : discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    if (cur_player == me) now_score = disc_count[cur_player] - disc_count[get_next_player(cur_player)];
                    else
                        now_score = disc_count[get_next_player(cur_player)] - disc_count[cur_player];
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }

public:
    OthelloBoard() {
        reset();
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8 * 8 - 4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    int calc_value() {
        if (cur_player != me) {
            //對手如果下一步可以下角落或來到這盤棋最後一手棋，則我這手棋價值會降低
            //對手如果可下的點小於等於四步，我下這手價值就提高
            int ans;
            ans = now_score;
            for (auto i : next_valid_spots) {
                if (i.x == 0 || i.x == 7 || i.y == 0 || i.y == 7) ans -= 5;
            }
            if (next_valid_spots.size() <= 4) ans += 3;
            if (next_valid_spots.size() == 1 && disc_count[EMPTY] == 1) ans -= 5;
            return ans;
        }
        else {
            int ans;
            ans = now_score;
            for (auto i : next_valid_spots) {
                if (i.x == 0 || i.x == 7 || i.y == 0 || i.y == 7) ans += 5;
            }
            if (next_valid_spots.size() <= 4) ans -= 3;
            if (next_valid_spots.size() == 1 && disc_count[EMPTY] == 1) ans += 5;
            return ans;
        }
    }
    bool put_disc(const Point& p) {
        if (!is_spot_valid(p)) {
            std::cout << "Now Want to put on[" << p.x << "][" << p.y << "], but this spot is invalud\n";
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        std::cout << "score : " << calc_value() << std::endl;
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                std::cout << "in next_valid spots.size == 0" << '\n';
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs)
                    winner = BLACK;
                else
                    winner = WHITE;
            }
        }
        return true;
    }
    std::string encode_player(int state) {
        if (state == BLACK) return "O";
        if (state == WHITE) return "X";
        return "Draw";
    }
    std::string encode_spot(int x, int y) {
        if (is_spot_valid(Point(x, y))) return ".";
        if (board[x][y] == BLACK) return "O";
        if (board[x][y] == WHITE) return "X";
        return " ";
    }
    std::string encode_output(bool fail = false) {
        int i, j;
        std::stringstream ss;
        ss << "Timestep #" << (8 * 8 - 4 - disc_count[EMPTY] + 1) << "\n";
        ss << "O: " << disc_count[BLACK] << "; X: " << disc_count[WHITE] << "\n";
        if (fail) {
            ss << "Winner is " << encode_player(winner) << " (Opponent performed invalid move)\n";
        }
        else if (next_valid_spots.size() > 0) {
            ss << encode_player(cur_player) << "'s turn\n";
        }
        else {
            ss << "Winner is " << encode_player(winner) << "\n";
        }
        ss << "+---------------+\n";
        for (i = 0; i < SIZE; i++) {
            ss << "|";
            for (j = 0; j < SIZE - 1; j++) {
                ss << encode_spot(i, j) << " ";
            }
            ss << encode_spot(i, j) << "|\n";
        }
        ss << "+---------------+\n";
        ss << next_valid_spots.size() << " valid moves: {";
        if (next_valid_spots.size() > 0) {
            Point p = next_valid_spots[0];
            ss << "(" << p.x << "," << p.y << ")";
        }
        for (size_t i = 1; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << ", (" << p.x << "," << p.y << ")";
        }
        ss << "}\n";
        ss << "=================\n";
        return ss.str();
    }
    std::string encode_state() {
        int i, j;
        std::stringstream ss;
        ss << cur_player << "\n";
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE - 1; j++) {
                ss << board[i][j] << " ";
            }
            ss << board[i][j] << "\n";
        }
        ss << next_valid_spots.size() << "\n";
        for (size_t i = 0; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << p.x << " " << p.y << "\n";
        }
        return ss.str();
    }
};

const std::string file_log = "gamelog.txt";
const std::string file_state = "state";
const std::string file_action = "action";
// Timeout is set to 10 when TA test your code.
constexpr int timeout = 1;

void launch_executable(std::string filename) {
#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( __NT__ )
    std::string command = "start /min " + filename + " " + file_state + " " + file_action;
    std::string kill = "timeout /t " + std::to_string(timeout) + " > NUL && taskkill /im " + filename + " > NUL 2>&1";
    system(command.c_str());
    system(kill.c_str());
#elif __linux__
    std::string command = "timeout " + std::to_string(timeout) + "s " + filename + " " + file_state + " " + file_action;
    system(command.c_str());
#elif __APPLE__
    // May require installing the command by:
    // brew install coreutils
    std::string command = "gtimeout " + std::to_string(timeout) + "s " + filename + " " + file_state + " " + file_action;
    system(command.c_str());
#endif
}

int main(int argc, char** argv) {
    assert(argc == 3);
    std::ofstream log("gamelog.txt");
    std::string player_filename[3];
    player_filename[1] = argv[1];
    player_filename[2] = argv[2];
    std::cout << "Player Black File: " << player_filename[OthelloBoard::BLACK] << '\n';
    std::cout << "Player White File: " << player_filename[OthelloBoard::WHITE] << '\n';
    OthelloBoard game;
    std::string data;
    data = game.encode_output();
    std::cout << data;
    log << data;
    while (!game.done) {
        // Output current state
        data = game.encode_state();
        std::ofstream fout(file_state);
        fout << data;
        fout.close();
        // Run external program
        launch_executable(player_filename[game.cur_player]);
        // Read action
        std::ifstream fin(file_action);
        Point p(-1, -1);
        while (true) {
            int x, y;
            if (!(fin >> x)) break;
            if (!(fin >> y)) break;
            p.x = x;
            p.y = y;
        }
        fin.close();
        // Reset action file
        if (remove(file_action.c_str()) != 0)
            std::cerr << "Error removing file: " << file_action << "\n";
        // Take action
        if (!game.put_disc(p)) {
            // If action is invalid.
            data = game.encode_output(true);
            std::cout << data;
            log << data;
            break;
        }
        data = game.encode_output();
        std::cout << data;
        log << data;
    }
    log.close();
    // Reset state file
    if (remove(file_state.c_str()) != 0)
        std::cerr << "Error removing file: " << file_state << "\n";
    return 0;
}