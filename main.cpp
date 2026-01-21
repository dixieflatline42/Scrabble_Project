#include <iostream>
#include <iomanip>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <random>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

const std::unordered_map<char, int> LETTER_COUNTS = {
    {'A', 9}, {'B', 2}, {'C', 2}, {'D', 4}, {'E',12},
    {'F', 2}, {'G', 3}, {'H', 2}, {'I', 9}, {'J', 1},
    {'K', 1}, {'L', 4}, {'M', 2}, {'N', 6}, {'O', 8},
    {'P', 2}, {'Q', 1}, {'R', 6}, {'S', 4}, {'T', 6},
    {'U', 4}, {'V', 2}, {'W', 2}, {'X', 1}, {'Y', 2},
    {'Z', 1},
    {'?', 2}
};

const std::unordered_map<char, int> LETTER_VALUES = {
    {'A', 1}, {'E', 1}, {'I', 1}, {'O', 1}, {'U', 1},
    {'L', 1}, {'N', 1}, {'S', 1}, {'T', 1}, {'R', 1},
    {'D', 2}, {'G', 2}, {'B', 3}, {'C', 3}, {'M', 3}, 
    {'P', 3}, {'F', 4}, {'H', 4}, {'V', 4}, {'W', 4}, 
    {'Y', 4}, {'K', 5}, {'J', 8}, {'X', 8}, {'Q',10}, 
    {'Z',10}, 
    {'?', 0}
};

enum class Bonus{
    NONE,
    DOUBLE_LETTER,
    TRIPLE_LETTER,
    DOUBLE_WORD,
    TRIPLE_WORD
};

struct Piece{
    char letter;
    int value;
    bool isBlank = false;
};

struct Player{
    int id;
    std::string name;
    int score;
    std::vector<Piece> hand;
};

struct Cell{
    bool occupied = false;
    Piece piece;
    Bonus bonus = Bonus::NONE;
};

struct Move{
    int x, y;
    bool horizontal;
    std::string word;
    std::vector<bool> usedBlank;
};

class Board{
    public:

        Board();

        static const int MAX = 15;
        Cell grid[MAX][MAX];

        void setBonus(int x, int y, Bonus b);
        void print() const;
        bool putPiece(int x, int y, Piece p);

        char bonusChar(Bonus b) const{
            switch(b) {
                case Bonus::DOUBLE_LETTER: return '2';
                case Bonus::TRIPLE_LETTER: return '3';
                case Bonus::DOUBLE_WORD:   return 'W';
                case Bonus::TRIPLE_WORD:   return 'T';
                default: return '.';
            }
        }
};

class Bag{
    private:
        std::vector<Piece> pieces;
    
    public:
        Bag(){
            for(const auto& [letter, count] : LETTER_COUNTS){
                for(int i = 0; i < count; i++){
                    bool isBlank = (letter == '?');
                    int value = isBlank ? 0 : LETTER_VALUES.at(letter);
                    pieces.push_back({letter, value, isBlank});
                }
            }
        }

        void shuffle(){
            std::shuffle(pieces.begin(), pieces.end(), std::mt19937(std::random_device()()));
        }

        bool empty() const{
            return pieces.empty();
        }

        Piece take(){
            if(pieces.empty()) return {'?', 0};
            Piece p = pieces.back();
            pieces.pop_back();
            return p;
        }

        int size() const {
            return pieces.size();
        }

        const std::vector<Piece>& getPieces() const { return pieces; }

};

Board::Board(){
    for(int i = 0; i < MAX; i++)
        for(int j = 0; j < MAX; j++){
            grid[i][j].occupied = false;
            grid[i][j].bonus = Bonus::NONE;
        }
    
    // TW - Triple Word
    int tw[][2] = {
        { 0, 0}, { 0, 7}, { 0, 14}, { 7,  0}, 
        {7, 14}, {14, 0}, {14,  7}, {14, 14}
    };
    for(auto &p : tw)
        setBonus(p[0], p[1], Bonus::TRIPLE_WORD);

    
    // DW - Double Word
    int dw[][2] = {
        { 1,  1}, { 2, 2}, { 3, 3}, { 4, 4},
        {10, 10}, {11,11}, {12,12}, {13,13},
        { 1, 13}, { 2,12}, { 3,11}, { 4,10},
        {10,  4}, {11, 3}, {12, 2}, {13, 1},
        { 7,  7}
    };
    for(auto &p : dw)
        setBonus(p[0], p[1], Bonus::DOUBLE_WORD);


    // TL - Triple Letter
    int tl[][2] = {
        {1,5},{1,9},{5,1},{5,5},{5,9},{5,13},
        {9,1},{9,5},{9,9},{9,13},{13,5},{13,9}
    };
    for(auto &p : tl)
        setBonus(p[0], p[1], Bonus::TRIPLE_LETTER);


    // DL - Double Letter
    int dl[][2] = {
        {0,3},{0,11},{2,6},{2,8},{3,0},{3,7},{3,14},
        {6,2},{6,6},{6,8},{6,12},{7,3},{7,11},
        {8,2},{8,6},{8,8},{8,12},{11,0},{11,7},{11,14},
        {12,6},{12,8},{14,3},{14,11}
    };
    for(auto &p : dl)
        setBonus(p[0], p[1], Bonus::DOUBLE_LETTER);
}

void Board::print() const{
    std::cout << "    ";
    for(int j = 0; j < MAX; j++)
        std::cout << std::setw(3) << j;
    std::cout << "\n";

    for(int i = 0; i < MAX; i++){
        std::cout << std::setw(3) << i << " ";
        for(int j = 0; j < MAX; j++){
            if(grid[i][j].occupied)
                std::cout << std::setw(3) << grid[i][j].piece.letter;
            else
                std::cout << std::setw(3) << bonusChar(grid[i][j].bonus);
        }
        std::cout << "\n";
    }
}

bool Board::putPiece(int x, int y, Piece p){
    if(grid[x][y].occupied) return false;
    
    grid[x][y].occupied = true;
    grid[x][y].piece = p;
    grid[x][y].bonus = Bonus::NONE;
    
    return true;
}

void Board::setBonus(int x, int y, Bonus b){
    grid[x][y].bonus = b;
}

void drawInitialHand(Player& p, Bag& bag){
    while(p.hand.size() < 7 && !bag.empty()){
        p.hand.push_back(bag.take());
    }
}

void printHand(const Player& p){
    std::cout << "Mao de " << p.name << ": ";

    for(const auto& piece : p.hand){
        std::cout << piece.letter;
        std::cout << "(" << piece.value << ")";
        std::cout << " ";
    }

    std::cout << "\n";
}

void printBag(const Bag& b){
    std::cout << "Total de pecas na bag: " << b.size() << "\n";
    
    const auto& pieces = b.getPieces();
    int n = (int)pieces.size();
    
    for(int i = 0; i < n; i++){
        const Piece& p = pieces[i]; 
        std::cout << p.letter << "(" << p.value << ") ";
    }
    
    std::cout << std::endl;
}

std::string toUpper(std::string s){
    for(char& c : s){
        c = std::toupper(static_cast<unsigned char>(c));
    }
    return s;
}

std::unordered_set<std::string> loadDictionary(const std::string& filename){
    std::unordered_set<std::string> dict;
    std::ifstream file(filename);

    std::string word;
    while(file >> word){
        dict.insert(toUpper(word));
    }

    return dict;
}

bool insideBoard(int x, int y){
    return x >= 0 && x < Board::MAX && y >= 0 && y < Board::MAX;
}

bool touchesExistingWord(
    const Board& board,
    const Move& move
){
    int x = move.x;
    int y = move.y;

    for(size_t i = 0; i < move.word.size(); i++){

        // Se a própria célula já está ocupada - conecta
        if(board.grid[x][y].occupied)
            return true;

        // Verifica adjacentes
        const int dx[4] = {-1, 1, 0, 0};
        const int dy[4] = {0, 0, -1, 1};

        for(int d = 0; d < 4; d++){
            int nx = x + dx[d];
            int ny = y + dy[d];

            if(insideBoard(nx, ny) &&
               board.grid[nx][ny].occupied){
                return true;
            }
        }

        if(move.horizontal) y++;
        else x++;
    }

    return false;
}

bool validateMove(
    const Board& board,
    const Move& move,
    const std::unordered_set<std::string>& dict,
    bool firstMove
){
    // First step - Word exists?
    if(dict.count(move.word) == 0){
        std::cout << "Palavra não existe no dicionário\n";
        return false;
    }

    int x = move.x;
    int y = move.y;

    bool touchesCenter = false;

    for(size_t i = 0; i < move.word.size(); i++){

        if(!insideBoard(x, y)){
            std::cout << "Palavra sai do tabuleiro\n";
            return false;
        }

        const Cell& cell = board.grid[x][y];

        // Second step - word may not overwrite another different letter
        if(cell.occupied && cell.piece.letter != move.word[i]){
            std::cout << "Conflito com letra existente\n";
            return false;
        }

        // Third step - first move may pass to the center
        if(x == 7 && y == 7)
            touchesCenter = true;

        if(move.horizontal)
            y++;
        else
            x++;
    }

    if(firstMove && !touchesCenter){
        std::cout << "Primeira jogada deve passar pelo centro\n";
        return false;
    }

    return true;
}

void applyMove(Board& board, const Move& move){
    int x = move.x;
    int y = move.y;

    for(size_t i = 0; i < move.word.size(); i++){
        char c = move.word[i];

        if(!board.grid[x][y].occupied){
            bool isBlank = move.usedBlank[i];
            int value = isBlank ? 0 : LETTER_VALUES.at(c);
            
            board.putPiece(x, y, {c, value, isBlank});
        }

        if(move.horizontal)
            y++;
        else
            x++;
    }
}

Board simulateMove(const Board& board, const Move& move){
    Board temp = board;

    int x = move.x;
    int y = move.y;

    for(size_t i = 0; i < move.word.size(); i++){
        char c = move.word[i];

        if(!temp.grid[x][y].occupied){
            bool isBlank = move.usedBlank[i];
            int value = LETTER_VALUES.at(c);
            
            temp.putPiece(x, y, {c, value, isBlank});
        }

        if(move.horizontal)
            y++;
        else
            x++;
    }

    return temp;
}

std::string extractWord(
    const Board& board,
    int x, int y,
    int dx, int dy
){
    // Walk back 
    int sx = x, sy = y;
    while(insideBoard(sx - dx, sy - dy) && 
          board.grid[sx - dx][sy - dy].occupied){
        
        sx -= dx;
        sy -= dy;
    }

    // Mount word
    std::string word;
    while(insideBoard(sx, sy) &&
          board.grid[sx][sy].occupied){
        
        word += board.grid[sx][sy].piece.letter;
        sx += dx;
        sy += dy;
    }

    return word;
}

std::string getMainWord(
    const Board& board,
    const Move& move
){
    int dx = move.horizontal ? 0 : 1;
    int dy = move.horizontal ? 1 : 0;

    return extractWord(board, move.x, move.y, dx, dy);
}

std::vector<std::string> getCrossWords(
    const Board& before,
    const Board& after,
    const Move& move
){
    std::vector<std::string> words;

    int x = move.x;
    int y = move.y;

    for(char c : move.word){
        if(!before.grid[x][y].occupied){
            // Perpendicular direction
            int dx = move.horizontal ? 1 : 0;
            int dy = move.horizontal ? 0 : 1;

            std::string w = extractWord(after, x, y, dx, dy);
            if(w.size() > 1)
                words.push_back(w);
        }

        if(move.horizontal)
            y++;
        else
            x++;
    }

    return words;

}

bool validateMoveWithCrossWords(
    const Board& board,
    const Move& move,
    const std::unordered_set<std::string>& dict,
    bool firstMove
){
    // Validate basic rules
    if(!validateMove(board, move, dict, firstMove)){
        return false;
    }

    if(!firstMove){
        if(!touchesExistingWord(board, move)){
            std::cout << "A palavra deve se conectar a uma existente\n";
            return false;
        }
    }

    Board after = simulateMove(board, move);

    // Main word
    std::string mainWord = getMainWord(after, move);
    if(dict.count(mainWord) == 0){
        std::cout << "Palavra principal invalida: " << mainWord << "\n";
        return false;
    }

    // Cross words
    auto crosses = getCrossWords(board, after, move);
    for(const auto& w : crosses){
        if(dict.count(w) == 0){
            std::cout << "Palavra cruzada invalida: " << w << "\n";
            return false;
        }
    }

    return true;
}

void showGameState(const Player& p, const Board& board){
    board.print();
    std::cout << "\n";
    printHand(p);
    std::cout << "\n";
}

void consumeHandPieces(
    Player& player,
    const Board& board,
    const Move& move
){
    int x = move.x;
    int y = move.y;

    for(size_t i = 0; i < move.word.size(); i++){
        char c = move.word[i];

        if(!board.grid[x][y].occupied){
            // tenta remover letra normal
            auto it = std::find_if(
                player.hand.begin(),
                player.hand.end(),
                [&](const Piece& p){ return p.letter == c; }
            );

            if(it != player.hand.end()){
                player.hand.erase(it);
            }
            else{
                // usa curinga
                auto blankIt = std::find_if(
                    player.hand.begin(),
                    player.hand.end(),
                    [&](const Piece& p){ return p.letter == '?'; }
                );
                if(blankIt != player.hand.end()){
                    player.hand.erase(blankIt);
                }
            }
        }

        if(move.horizontal) y++;
        else x++;
    }
}

bool canBuildWordFromHand(
    const Player& player,
    const Board& board,
    const Move& move
){
    std::unordered_map<char, int> handCount;

    for(const auto& p : player.hand){
        handCount[p.letter]++;
    }

    int x = move.x;
    int y = move.y;

    for(size_t i = 0; i < move.word.size(); i++){
        char c = move.word[i];

        if(!board.grid[x][y].occupied){
            if(handCount[c] > 0){
                handCount[c]--;
            }
            else if(handCount['?'] > 0){
                handCount['?']--;
            }
            else{
                return false;
            }
        }

        if(move.horizontal) y++;
        else x++;
    }

    return true;
}

int scoreWord(
    const Board& before,
    const Board& after,
    int x, int y,
    int dx, int dy
){
    int score = 0;
    int wordMultiplier = 1;

    while(insideBoard(x, y) && after.grid[x][y].occupied){
        const Cell& cellAfter = after.grid[x][y];
        const Cell& cellBefore = before.grid[x][y];

        int letterValue = cellAfter.piece.value;

        // bônus só se a peça foi recém colocada
        if(!cellBefore.occupied){
            switch(cellBefore.bonus){
                case Bonus::DOUBLE_LETTER:
                    letterValue *= 2;
                    break;
                case Bonus::TRIPLE_LETTER:
                    letterValue *= 3;
                    break;
                case Bonus::DOUBLE_WORD:
                    wordMultiplier *= 2;
                    break;
                case Bonus::TRIPLE_WORD:
                    wordMultiplier *= 3;
                    break;
                default:
                    break;
            }
        }

        score += letterValue;
        x += dx;
        y += dy;
    }

    return score * wordMultiplier;
}

int scoreMove(
    const Board& before,
    const Board& after,
    const Move& move
){
    int totalScore = 0;

    // Palavra principal
    int dx = move.horizontal ? 0 : 1;
    int dy = move.horizontal ? 1 : 0;

    totalScore += scoreWord(
        before, after,
        move.x, move.y,
        dx, dy
    );

    // Palavras cruzadas
    int x = move.x;
    int y = move.y;

    for(size_t i = 0; i < move.word.size(); i++){
        if(!before.grid[x][y].occupied){
            int cdx = move.horizontal ? 1 : 0;
            int cdy = move.horizontal ? 0 : 1;

            std::string cross = extractWord(after, x, y, cdx, cdy);
            if(cross.size() > 1){
                totalScore += scoreWord(
                    before, after,
                    x, y,
                    cdx, cdy
                );
            }
        }

        if(move.horizontal) y++;
        else x++;
    }

    // Bingo
    int usedTiles = 0;
    x = move.x;
    y = move.y;

    for(char _ : move.word){
        if(!before.grid[x][y].occupied)
            usedTiles++;

        if(move.horizontal) y++;
        else x++;
    }

    if(usedTiles == 7){
        totalScore += 50;
    }

    return totalScore;
}

bool playMove(
    Player& player,
    Board& board,
    Bag& bag,
    const Move& move,
    const std::unordered_set<std::string>& dict,
    bool firstMove
){
    if(!canBuildWordFromHand(player, board, move)){
        std::cout << "Pecas insuficientes\n";
        return false;
    }

    if(!validateMoveWithCrossWords(board, move, dict, firstMove)){
        return false;
    }

    Board before = board;

    applyMove(board, move);
    consumeHandPieces(player, before, move);

    Board after = board;

    int points = scoreMove(before, after, move);
    player.score += points;

    drawInitialHand(player, bag);

    std::cout << player.name << " marcou "
              << points << " pontos\n";
    std::cout << "Pontuacao total: "
              << player.score << "\n\n";

    board.print();
    printHand(player);

    return true;
}

bool readMoveFromInput(Move& move){
    char dir;

    std::cin >> move.x >> move.y >> dir >> move.word;

    if(!std::cin)
        return false;

    move.word = toUpper(move.word);
    move.horizontal = (dir == 'H');

    move.usedBlank.assign(move.word.size(), false);

    return true;
}

int main(){

    Board board;
    auto dictionary = loadDictionary("dictionary.txt");

    Bag bag;
    bag.shuffle();

    Player p1{1, "Teddy", 0};
    Player p2{2, "Woody", 0};

    drawInitialHand(p1, bag);
    drawInitialHand(p2, bag);

    std::vector<Player*> players = {&p1, &p2};
    int currentPlayer = 0;
    bool firstMove = true;
    int passesInARow = 0;

    while(true){
        Player& p = *players[currentPlayer];

        std::cout << "\n============================\n";
        std::cout << "Vez de: " << p.name << "\n";
        std::cout << "Pontuacao: " << p.score << "\n\n";

        board.print();
        std::cout << "\n";
        printHand(p);

        std::cout << "\nDigite jogada (x y H/V PALAVRA)\n";
        std::cout << "Ou digite PASS para passar a vez\n> ";

        std::string cmd;
        std::cin >> cmd;

        if(cmd == "PASS"){
            passesInARow++;
            std::cout << p.name << " passou a vez.\n";
        }
        else{
            // devolve a primeira leitura
            std::cin.putback(' ');
            for(int i = cmd.size() - 1; i >= 0; i--)
                std::cin.putback(cmd[i]);

            Move move;
            if(!readMoveFromInput(move)){
                std::cout << "Entrada invalida\n";
                continue;
            }

            if(playMove(p, board, bag, move, dictionary, firstMove)){
                firstMove = false;
                passesInARow = 0;
            }
            else{
                std::cout << "Jogada invalida, tente novamente\n";
                continue;
            }
        }

        // Fim de jogo: 2 passes seguidos OU bag vazia e mao vazia
        if(passesInARow >= players.size()){
            std::cout << "\nJogo encerrado por passes consecutivos\n";
            break;
        }

        currentPlayer = (currentPlayer + 1) % players.size();
    }

    std::cout << "\n===== FIM DE JOGO =====\n";

    for(const auto& p : players){
        std::cout << p->name << ": " << p->score << " pontos\n";
    }

    if(p1.score > p2.score)
        std::cout << "Vencedor: " << p1.name << "\n";
    else if(p2.score > p1.score)
        std::cout << "Vencedor: " << p2.name << "\n";
    else
        std::cout << "Empate!\n";

    return 0;
}