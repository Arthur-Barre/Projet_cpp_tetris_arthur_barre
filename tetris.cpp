#include <vector>
#include <iostream>
#include <random>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

std::vector<sf::Texture> Textures(8); // = {I_texture, J_texture, L_texture, O_texture, S_texture, T_texture, Z_texture};

sf::Texture &None_texture = Textures[0];
sf::Texture &I_texture = Textures[1];
sf::Texture &J_texture = Textures[2];
sf::Texture &L_texture = Textures[3];
sf::Texture &O_texture = Textures[4];
sf::Texture &S_texture = Textures[5];
sf::Texture &T_texture = Textures[6];
sf::Texture &Z_texture = Textures[7];


int randint(int min = 1, int max = 7) {
    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_int_distribution<int> dist(min, max); // Define the range

    return dist(gen);
}


class C{
    public:
    int i;
    int j;

    C(int i, int j) : i(i), j(j) {}

    C operator+(const C diff) const {
        return C(this->i + diff.i, this->j + diff.j);
    }
    C operator-(const C diff) const {
        return C(this->i - diff.i, this->j - diff.j);
    }
    void operator+=(const C diff){
        i += diff.i;
        j += diff.j;
    }
    void operator-=(const C diff){
        i -= diff.i;
        j -= diff.j;
    }

    C copy(){
        C new_C(i, j);
        return new_C;
    }

    bool is_inbound(int left = 1, int right = 10, int bottom = 1, int top = 20){
        return this->j <= right and this->j >= left and this->i >= bottom and this->i <= top;
    }
};


class Node {
    public:

    std::vector<int> line;
    Node* next;

    Node(std::vector<int> value): line(value), next(nullptr) {}

    bool is_full(){
        for (int i = 0; i< 10; i++){
            if (line[i] == 0){
                return false;
            }
        }
        return true;
    }
};


class Board {
    public:

    Node* top;
    std::vector<C> piece;

    Board() : top(nullptr), piece({C(20, 5), C(20, 4), C(20, 6), C(20, 7)}){
        for (int i = 0; i < 19; i++){
            Node* node = new Node({0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
            node->next = top;
            top = node;
        }
        Node* node = new Node({0, 0, 0, 1, 1, 1, 1, 0, 0, 0});
        node->next = top;
        top = node;
    }

    ~Board() {
        Node* temp;
        while (top) {
            temp = top;
            top = top->next;
            delete temp;
        }
    }

    void print(){
        Node* temp = top;
        while (temp) {
            std::cout << "{";
            for (int i = 0; i < 10; i++){
                std::cout << temp->line[i] << ",";
            }
            std::cout << "}" << std::endl;
            temp = temp->next;
        }
        std::cout <<  std::endl;
    }

    void display(sf::RenderWindow& window){
        Node* line_node = top;
        sf::Sprite cell;
        for (int line = 0; line < 20; line ++){
            for (int column = 0; column < 10; column++){
                int piece_id = line_node->line[column];
                cell.setTexture(Textures[piece_id]);
                cell.setPosition(50*column, 50*line);
                window.draw(cell);
            }
            line_node = line_node->next;
        }
    }

    Node* get_line_node(int line_number){
        Node* line_node = top;
        for (int k = 20; k > line_number; k--){
            line_node = line_node->next;
        }
        return line_node;
    }

    void RemoveLine(int line_number){
        if (line_number == 20){
            top->line = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            return;
        }
        Node* line_node = get_line_node(line_number + 1);
        Node* deleted_line = line_node->next;
        line_node->next = line_node->next->next;
        delete deleted_line;

        Node* node = new Node({0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
        node->next = top;
        top = node;
    }

    int& operator()(int line, int column){
        Node* line_node = get_line_node(line);
        return line_node->line[column - 1];
    }

    int& operator()(C coords){
        int line = coords.i;
        int column = coords.j;
        Node* line_node = get_line_node(line);
        return line_node->line[column - 1];
    }


    bool move(C diff){
        int id = this->operator()(this->piece[0]);
        for (int i = 0; i < 4; i++){
            C& coords_i = this->piece[i];
            this->operator()(coords_i) = 0;
            coords_i += diff;
        }

        bool collision = false;
        for (int i = 0; i < 4; i++) {
            C piece_i = this->piece[i];
            if (!piece_i.is_inbound() || this->operator()(piece_i) != 0 ) {
                collision = true;
                break;
            }
        }
    
        if (collision) {
            for (int i = 0; i < 4; i++) {
                this->piece[i] -= diff;
            }
        }

        for (int i = 0; i < 4; i++){
            this->operator()(this->piece[i]) = id;
        }

        return collision;
    }

    
    void rotate_left(bool del = true, int id = 0){
        id = this->operator()(this->piece[0]);
        C base = piece[0].copy();
        for (int k = 0; k < 4; k++){
            if (del){
                this->operator()(piece[k]) = 0;
            }

            piece[k] -= base;
        }
        for (int k = 0; k < 4; k++){
            C& piece_k = piece[k];
            piece_k = C(piece_k.j, - piece_k.i);
        }
        for (int k = 0; k < 4; k++){
            piece[k] += base;
        }
        
        for (int i = 0; i < 4; i++){
            if (!piece[i].is_inbound() || this->operator()(this->piece[i]) != 0){
                rotate_right(del = false, id = id);
            }
        }

        for (int i = 0; i < 4; i++){
            this->operator()(this->piece[i]) = id;
        }
    }


    void rotate_right(bool del = true, int id = 0){
        id = this->operator()(this->piece[0]);
        C base = piece[0].copy();
        
        for (int k = 0; k < 4; k++){
            if (del){
                this->operator()(piece[k]) = 0;
            }

            piece[k] -= base;
        }
        for (int k = 0; k < 4; k++){
            C& piece_k = piece[k];
            piece_k = C(-piece_k.j, piece_k.i);
        }
        for (int k = 0; k < 4; k++){
            piece[k] += base;
        }

        for (int i = 0; i < 4; i++){
            if (!piece[i].is_inbound() || this->operator()(this->piece[i]) != 0){
                rotate_left(del = false, id = id);
            }
        }
        
        for (int i = 0; i < 4; i++){
            this->operator()(this->piece[i]) = id;
        }
    }


    void spawn_piece(int id){
        if (id == 1){
            piece[0] = C(20, 5);
            piece[1] = C(20, 4);
            piece[2] = C(20, 6);
            piece[3] = C(20, 7);
        }
        else if (id == 2){
            piece[0] = C(19, 5);
            piece[1] = C(20, 4);
            piece[2] = C(19, 4);
            piece[3] = C(19, 6);
        }
        else if (id == 3){
            piece[0] = C(19, 5);
            piece[1] = C(19, 4);
            piece[2] = C(19, 6);
            piece[3] = C(20, 6);
        }
        else if (id == 4){
            piece[0] = C(20, 5);
            piece[1] = C(20, 6);
            piece[2] = C(19, 5);
            piece[3] = C(19, 6);
        }
        else if (id == 5){
            piece[0] = C(20, 5);
            piece[1] = C(19, 4);
            piece[2] = C(19, 5);
            piece[3] = C(20, 6);
        }
        else if (id == 6){
            piece[0] = C(19, 5);
            piece[1] = C(19, 4);
            piece[2] = C(19, 6);
            piece[3] = C(20, 5);
        }
        else if (id == 7){
            piece[0] = C(20, 5);
            piece[1] = C(20, 4);
            piece[2] = C(19, 5);
            piece[3] = C(19, 6);
        }
        
        for (int i = 0; i < 4; i++){
            this->operator()(piece[i]) = id;
        }
    }
};


std::string path = "/home/arthur-barre/Projet_tetris/Projet_cpp_tetris_arthur_barre/Textures/";

int main(){

    if (!None_texture.loadFromFile(path + "None_Texture.png")) {
        std::cerr << "Error loading None_Texture!" << std::endl;
        return -1;}
    if (!I_texture.loadFromFile(path + "I_Texture.png")) {
        std::cerr << "Error loading I_Texture!" << std::endl;
        return -1;}
    if (!J_texture.loadFromFile(path + "J_Texture.png")) {
        std::cerr << "Error loading J_Texture!" << std::endl;
        return -1;}
    if (!L_texture.loadFromFile(path + "L_Texture.png")) {
        std::cerr << "Error loading L_Texture!" << std::endl;
        return -1;}
    if (!O_texture.loadFromFile(path + "O_Texture.png")) {
        std::cerr << "Error loading O_Texture!" << std::endl;
        return -1;}
    if (!S_texture.loadFromFile(path + "S_Texture.png")) {
        std::cerr << "Error loading S_Texture!" << std::endl;
        return -1;}
    if (!T_texture.loadFromFile(path + "T_Texture.png")) {
        std::cerr << "Error loading T_Texture!" << std::endl;
        return -1;}
    if (!Z_texture.loadFromFile(path + "Z_Texture.png")) {
        std::cerr << "Error loading Z_Texture!" << std::endl;
        return -1;}
    std::vector<sf::Texture> Textures = {None_texture, I_texture, J_texture, L_texture, O_texture, S_texture, T_texture, Z_texture};
    
    Board board;
    
    bool alive = true;

    std::time_t chrono = std::time(nullptr);

    sf::RenderWindow window(sf::VideoMode(502, 1002), "Tetris");

////////////////// Game loop //////////////////////////

    while (window.isOpen()) {
        std::time_t time = std::time(nullptr);
        auto delta_time = difftime(time, chrono);
        if (difftime(time, chrono) > 0.4){
            if (board.move(C(-1, 0))){
                for (int line = 1; line <= 20; line++){
                    Node* line_node = board.get_line_node(line);
                    if (line_node->is_full()){
                        board.RemoveLine(line);
                        line--;
                    }
                }
                int id = randint();
                board.spawn_piece(id);
            }
            chrono = std::time(nullptr);
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            else if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Escape){
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::A){
                    board.rotate_left();
                }

                else if (event.key.code == sf::Keyboard::E){
                    board.rotate_right();
                }

                else if (event.key.code == sf::Keyboard::Right){
                    board.move(C(0, 1));
                }

                else if (event.key.code == sf::Keyboard::Left){
                    board.move(C(0, -1));
                }

                else if (event.key.code == sf::Keyboard::Down){
                    board.move(C(-1, 0));
                }
            }
        }

        window.clear();

        board.display(window);

        window.display();
    }
}