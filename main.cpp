#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <fstream>

//global scope declarations
std::vector<int> touchingBoard;
std::vector<Fl_Button*> buttons;
std::string str; //this actually is unnecessary
int n;
std::vector<int> mines;
int toWin;
//time
std::chrono::time_point<std::chrono::system_clock> start;
std::chrono::time_point<std::chrono::system_clock> stop;

//function declarations for scope
void diff_buttonBeginner(Fl_Widget* widget, void* data);
void diff_buttonIntermediate(Fl_Widget* widget, void* data);
void diff_buttonExpert(Fl_Widget* widget, void* data);
bool inVector(std::vector<int> someVec, int target);
void quitButton(Fl_Widget* widget, void* data);
void reveal(int index);
void safe_click(Fl_Widget* widget, void* data);
void bomb_click(Fl_Widget* widget, void* data);
std::vector<int> randomMines(int range, int numMines);
std::vector<int> touching(int totalSpaces, std::vector<int> bombLocations, int n);
void create_board(Fl_Window* window, int button_width, int button_height, int diff);
void resetGame();
void saveGameResult(const std::string& filename,std::chrono::time_point<std::chrono::system_clock> tStart, std::chrono::time_point<std::chrono::system_clock> tStop, bool won);

/*
Saves the game to the file
File contains the past history of games and their times to complete
*/
void saveGameResult(const std::string& filename,std::chrono::time_point<std::chrono::system_clock> tStart, std::chrono::time_point<std::chrono::system_clock> tStop, bool won) {
    std::ofstream file(filename, std::ios::app);
    if(!file.is_open()){
        std::cerr <<"Error"<<std::endl;
        return;
    }
    file << "Game: " << (std::chrono::duration_cast<std::chrono::milliseconds>(tStop-tStart).count()) / 1000 << " seconds - " << (won ? "Won" : "Lost") << "\n";
    file.close();
}

/*
Deletes and frees all memory from the program
*/
void resetGame() {
    //clear everything
    start = std::chrono::time_point<std::chrono::system_clock>();
    stop = std::chrono::time_point<std::chrono::system_clock>();

    toWin=0;
    touchingBoard.clear();
    for(auto button : buttons){
        if(button != nullptr){
            delete button;
        }
    }
    buttons.clear();
    str.clear();
    n = 0;
    mines.clear();

}

/*
Callbacks for user difficulties
They call createBoard() with different parameters depending on difficulty
*/
void diff_buttonBeginner(Fl_Widget* widget, void* data){
    resetGame();
    Fl_Window* window = static_cast<Fl_Window*>(data);

    //clear previous board
    window->clear();
    window->redraw();


    create_board(window, 20, 20, 0);
}
void diff_buttonIntermediate(Fl_Widget* widget, void* data){
    resetGame();
    Fl_Window* window = static_cast<Fl_Window*>(data);

    //clear previous board
    window->clear();
    window->redraw();

    create_board(window, 20, 20, 1);
}
void diff_buttonExpert(Fl_Widget* widget, void* data){
    resetGame();
    Fl_Window* window = static_cast<Fl_Window*>(data);

    //clear previous board
    window->clear();
    window->redraw();
    

    create_board(window, 20, 20, 2);
    
}

/*
Finds if an element is in a vector
*/
bool inVector(std::vector<int> someVec, int target){
    // for(int i =0;i<someVec.size();i++){
    //     if(target == someVec[i]){
    //         return true;
    //     }
    // }
    // return false;
    return std::find(someVec.begin(), someVec.end(), target) != someVec.end();
}

/*
Generates random numMines for a given range
    - If the mine already exists, generate a new one.
    - Returns a vector of random mines with size numMines
*/
std::vector<int> randomMines(int range, int numMines){
    // generate numMines number of mines in between a 1 and range
    std::random_device rd;
    std::mt19937 gen(rd());

    //range
    std::uniform_int_distribution<> dist(1, range);

    bool run;
    int currentRand;
    int count;

    //for numMines, generate number in between 1 and range..... no duplicates
    for(int i = 0;i<numMines;i++){
        //loop to make sure no duplicates in vector mines
        run = true;
        currentRand = dist(gen);

        while(run == true){
            //find if in vector
            if(inVector(mines,currentRand)!=true){
                // in mines vector
                //run is true
                run = false;
            }
            else{
                break;
            }
        }
        mines.push_back(currentRand);
        count++;
        //tester
        //std::cout<<"Added "<<currentRand<<std::endl;
    }
    //tester
    //std::cout<<"Mines on board: "<<count<<std::endl;
    return mines;
}


/*
Touching function takes total spaces, bomb locations, and n or the width and heighth
    - Marks bombs as 99 in bombsTouching
    - Goes through every cell checking all eight sides to see amount of bombs touching
    - Checks to make sure the cells are in bounds and not on sides so they dont overflow
    - Increments mine value on bomblocations
*/
std::vector<int> touching(int totalSpaces, std::vector<int> bombLocations, int n){
    std::vector<int> bombsTouching(totalSpaces, 0);

    //mark bombs as 99
    for(int i = 0; i < bombLocations.size(); i++){
        bombsTouching[bombLocations[i]] = 99;
    }

    //iterate through each cell
    for(int i = 0; i < totalSpaces; i++){
        //skip bombs
        if(bombsTouching[i] == 99){
            continue;
        }

        //calculate indices based on i and board size
        int row = i / n;
        int col = i % n;

        //check all neighbors if in bounds
        //from -1 to 1
        for(int offset = -1; offset <= 1; offset++){


            for(int offsett2 = -1; offsett2 <= 1; offsett2++){

                //skip center
                if(offset == 0 && offsett2 == 0){
                    continue;
                }

                //indices
                int neighborRow = row + offset;
                int neighborCol = col + offsett2;

                //skip out of bound neighbors
                if(neighborRow < 0 || neighborRow >= n || neighborCol < 0 || neighborCol >= n){
                    continue;

                }
                //index of neighbor space
                int neighbor = neighborRow * n + neighborCol;

                //if neighbor is a bomb
                if(bombsTouching[neighbor] == 99){
                    //increment middle cell... current one checking
                    bombsTouching[i] += 1;

                }
            }


        }

    }


    return bombsTouching;
}

/*
Quit the game
*/
void quitButton(Fl_Widget* widget, void* data){
    resetGame();
    exit(0);
}

/*
Reveals the white blocks (blocks of zeros) and surrounding spaces touching bombs
    - Dynamic memory allocation for the labels
    - Goes through all touching squares starting from the one clicked
    - Recursively calls to check next touching zero space
    - Brute force
*/
void reveal(int index) {
    //valid index
    if(index < 0 || index >= touchingBoard.size()){
        return;
    }

    //not revealed yet
    if(buttons[index]->color() == FL_WHITE){
        return;
    }

    

    //reveal neighbors
    if(touchingBoard[index] == 0){
        //change to white
        buttons[index]->color(FL_WHITE);
        buttons[index]->redraw();

        /*
        Need to reveal surrounding buttons if they are not zero
        */

        //this is for the LEFT
        if (index > 0 && (index % n != 0) && touchingBoard[index - 1] > 0) {
            //tester
            // std::cout << "button left changed" << std::endl;
            buttons[index -1]->color(FL_GREEN);

            //create dynamic array to stor num bombs

            //THIS IS UNNECESSARY, SHOULD HAVE DONE WHAT I DID IN REVEAL
            std::string str = std::to_string(touchingBoard[index - 1]);
            //allocation of memory
            char* label = new char[str.size() + 1];

            //copy string to new dynamic allocation
            std::strcpy(label, str.c_str());

            buttons[index -1]->label(label);
            buttons[index -1]->redraw();


            
        }

        //this is for the TOP
        if (index - n >= 0 && touchingBoard[index - n] > 0) {
            //tester
            // std::cout << "button up changed" << std::endl;
            
            buttons[index - n]->color(FL_GREEN);

            //create dynamic array to stor num bombs
            std::string str = std::to_string(touchingBoard[index - n]);
            
            //memory allocation
            char* label2 = new char[str.size() + 1];

            //newloy allocated memory
            std::strcpy(label2, str.c_str());

            //set to string memory
            buttons[index - n]->label(label2);
            buttons[index - n]->redraw();

        }

        //TOP LEFT
        if(index - (n+1) >= 0 &&  touchingBoard[index - (n+1)] > 0  && (index % n != 0) ){
            //tester
            // std::cout << "button top left changed" << std::endl;
            
            buttons[index - (n+1)]->color(FL_GREEN);

            //create dynamic array to stor num bombs
            std::string str = std::to_string(touchingBoard[index - (n+1)]);
            
            //memory allocation
            char* label3 = new char[str.size() + 1];

            //newloy allocated memory
            std::strcpy(label3, str.c_str());

            //set to string memory
            buttons[index - (n+1)]->label(label3);
            buttons[index - (n+1)]->redraw();

        }

        //BOTTOM
        if( index+n <= (touchingBoard.size() - 1)  && touchingBoard[index + n] > 0){
            //tester
            // std::cout << "button bottom changed" << std::endl;
            
            buttons[index +n]->color(FL_GREEN);

            //create dynamic array to stor num bombs
            std::string str = std::to_string(touchingBoard[index +n]);
            
            //memory allocation
            char* label4 = new char[str.size() + 1];

            //newloy allocated memory
            std::strcpy(label4, str.c_str());

            //set to string memory
            buttons[index +n]->label(label4);
            buttons[index +n]->redraw();

        }

        //BOTTOM LEFT
        if(index+ (n-1) <= (touchingBoard.size() - 1) && touchingBoard[index + (n-1)] > 0 && (index % n != 0)){
            //tester
            // std::cout << "button bottom left changed" << std::endl;
            
            buttons[index + (n-1)]->color(FL_GREEN);

            //create dynamic array to stor num bombs
            std::string str = std::to_string(touchingBoard[index + (n-1)]);
            
            //memory allocation
            char* label5 = new char[str.size() + 1];

            //newloy allocated memory
            std::strcpy(label5, str.c_str());

            //set to string memory
            buttons[index + (n-1)]->label(label5);
            buttons[index + (n-1)]->redraw();
        }

        //RIGHT
        if(((index+1) % n != 0) && touchingBoard[index + 1] > 0){
            //tester
            // std::cout << "button right changed" << std::endl;
            
            buttons[index +1]->color(FL_GREEN);

            //create dynamic array to stor num bombs
            std::string str = std::to_string(touchingBoard[index +1]);
            
            //memory allocation
            char* label6 = new char[str.size() + 1];

            //newloy allocated memory
            std::strcpy(label6, str.c_str());

            //set to string memory
            buttons[index + 1]->label(label6);
            buttons[index + 1]->redraw();

        }

        //TOP RIGHT
        if(index - (n-1) >= 0 && touchingBoard[index - (n-1)] > 0  && ((index-(n-1)) % n != 0)){
            //tester
            // std::cout << "button right changed" << std::endl;
            
            buttons[index - (n-1)]->color(FL_GREEN);

            //create dynamic array to stor num bombs
            std::string str = std::to_string(touchingBoard[index - (n-1)]);
            
            //memory allocation
            char* label7 = new char[str.size() + 1];

            //newloy allocated memory
            std::strcpy(label7, str.c_str());

            //set to string memory
            buttons[index - (n-1)]->label(label7);
            buttons[index - (n-1)]->redraw();

        }
        //BOTTOM RIGHT
        if(index + (n+1) <= (touchingBoard.size() - 1) && touchingBoard[index + (n+1)] > 0 && ((index+(n+1)) % n != 0)){
            //tester
            // std::cout << "button right changed" << std::endl;
            
            buttons[index + (n+1)]->color(FL_GREEN);

            //create dynamic array to stor num bombs
            std::string str = std::to_string(touchingBoard[index + (n+1)]);
            
            //memory allocation
            char* label8 = new char[str.size() + 1];

            //newloy allocated memory
            std::strcpy(label8, str.c_str());

            //set to string memory
            buttons[index + (n+1)]->label(label8);
            buttons[index + (n+1)]->redraw();

        }


        //LEFT  index > 0 && (index % n != 0) && touchingBoard[index - 1] > 0
        if(index > 0 && (index % n != 0) && touchingBoard[index - 1] == 0){

            reveal(index - 1);
        }

        //TOP LEFT
        if(index - (n+1) >= 0 &&  touchingBoard[index - (n+1)] == 0  && (index % n != 0) ){
            reveal(index - (n+1));
        }

        //BOTTOM LEFT
        if(index+ (n-1) <= (touchingBoard.size() - 1) && touchingBoard[index + (n-1)] == 0 && (index % n != 0)){
            reveal(index+ (n-1));
        }


        //BOTTOM RIGHT
        if(index + (n+1) <= (touchingBoard.size() - 1) && touchingBoard[index + (n+1)] == 0 && ((index+(n+1)) % n != 0)){
            reveal(index + (n+1));
        }

        //TOP RIGHT
        if(index - (n-1) >= 0 && touchingBoard[index - (n-1)] > 0  && ((index-(n-1)) % n != 0)){
            reveal(index - (n-1));

        }

        //check above to make sure in bounds
        if(index - n >= 0 && touchingBoard[index - n] == 0){
            reveal(index - n);
        }
        //check right
        if(index + 1 < touchingBoard.size() && ((index + 1) % n != 0) && touchingBoard[index + 1] == 0){
            reveal(index + 1);  // Only reveal right if not in the last column
        }
        //check bottom
        if(index + n < touchingBoard.size() && touchingBoard[index + n] == 0){
            reveal(index + n);
        }
    }
}


/*
Checks for if the user has won
    - Based on numUncovered and compares to toWin
    - Returns true or false
*/
bool checkWin(){
    int numUncovered = 0;

    for(int i = 0; i < buttons.size(); ++i){
        //if button is white or green... add
        if (buttons[i]->color() == FL_WHITE || buttons[i]->color() == FL_GREEN) {
            numUncovered++;
        }
    }

    //check if it matches winning number
    if(numUncovered == toWin){
        return true;
    }
    return false;
}

/*
The callback for if a space is safe
    - Starts timer
    - Turns button red if right clicked
    - Turns green if clicked and not a zero
    - If is a zero, call reveal for white blocks
    - If the amount of spaces revealed is equal to the total spaces- bombs then game is over and user wins
*/
void safe_click(Fl_Widget* widget, void* data){
    start = std::chrono::system_clock::now(); //start the timer



    int* index = static_cast<int*>(data);
    int actualIndex = *index;
    // std::cout << touchingBoard[actualIndex] << std::endl;
    Fl_Button* button = static_cast<Fl_Button*>(widget);

    //if right mouse click
    if(Fl::event_button() == 3){
        //red if right clicked
        button->color(FL_RED);
        button->redraw();
    }
    else{
        if(touchingBoard[actualIndex]!=0){
            //convert int to string
            str = std::to_string(touchingBoard[actualIndex]);

            //convert string to const char*
            //issue was pointer needed to be valid as long as str exists
            const char* currentNumBombs = str.c_str();

            //tester
            // std::cout << currentNumBombs << std::endl;

            //color change
            button->color(FL_GREEN);

            //new label
            button->label(currentNumBombs);

            //redraw
            button->redraw();

        }
        else{
            //otherwise it means that it is a 0 square... white
            reveal(actualIndex);
        }
        //check if game won
        if (checkWin()) {
            fl_alert("Congratulations! You've won the game!");
            stop = std::chrono::system_clock::now(); //stop the timer
            
            saveGameResult("game_results.txt", start, stop , true);
            
        }
    }
    


}

/*
Bomb click, if user clicks a bomb
    - If the click was a right click, turn color to red
    - If normal click, user loses
    - Prints results to the file
*/
void bomb_click(Fl_Widget* widget, void* data){
    Fl_Window* window = static_cast<Fl_Window*>(data);
    Fl_Button* button = static_cast<Fl_Button*>(widget);
    start = std::chrono::system_clock::now();

    if(Fl::event_button() == 3){
        button->color(FL_RED);
        button->redraw();
    }
    else{
        std::cout << "Bomb clicked! Game over." << std::endl;
        fl_alert("GAME OVER");
        stop = std::chrono::system_clock::now();
        saveGameResult("game_results.txt", start, stop , false);

        resetGame();
    }
}

/*
Create a board with button width and height and difficulty
    - Board creates depending on difficulty 9x9 16x16 30x30
    - Creates buttons and adds to buttons vector
    - Resets game in case 
*/
void create_board(Fl_Window* window, int button_width, int button_height, int diff){
    resetGame();
    //add the buttons again after cleared
    Fl_Button* beginnerButton = new Fl_Button(20, 20, 80, 30, "Beginner");
    //pass window as data
    beginnerButton->callback(diff_buttonBeginner, window);
    window->add(beginnerButton);

    Fl_Button* intermediateButton = new Fl_Button(360, 20, 80, 30, "Intermediate");
    //pass window as data
    intermediateButton->callback(diff_buttonIntermediate, window);
    window->add(intermediateButton);

    Fl_Button* expertButton = new Fl_Button(700, 20, 80, 30, "Expert");
    //pass window as data
    expertButton->callback(diff_buttonExpert, window);
    window->add(expertButton);
    

    int rows, cols;
    
    //__________________________________________________________________________________________________________________________
    //where board is generated to center board on any window for diff chosen
    int corner_start;

    //total mines and spaces on board
    int totalSpaces;
    int totalMines;
    //__________________________________________________________________________________________________________________________




    //__________________________________________________________________________________________________________________________
    //easy
    if(diff == 0){
        corner_start = 300;

        totalSpaces = 81;
        totalMines = 10;

        rows = 9;
        cols = 9;
        toWin = 71;

    }
    //medium
    else if(diff == 1){
        corner_start = 250;

        totalSpaces = 256;
        totalMines = 40;

        rows = 16;
        cols = 16;
        toWin = 216;
    }
    //hard
    else{
        corner_start = 100;

        totalSpaces = 900;
        totalMines = 140;

        rows = 30;
        cols = 30;
        toWin = 760;
    }
    //__________________________________________________________________________________________________________________________

    n = rows;


    //make n random mines from 1-total mines
    //stored in locations
    std::vector<int> bombLocations = randomMines(totalSpaces,totalMines);
    


    //tester
    //std::cout<< "The first bomb location "<<bombLocations[0]<<std::endl;



    //__________________________________________________________________________________________________________________________
    //for every row there is

    //current position in creation and label for position
    const char* label;
    int currentPosition = 0;

    //create board
    for(int row = 0;row<rows;row++){
        //for every collumn there is
        for(int col = 0; col<cols; col++){

            if(inVector(bombLocations, currentPosition) == true){
                label = "m";
            }
            
            //otherwise it is "s"
            else{
                label = "s";
            }

            


            //button pos
            int x = col * (button_width)+corner_start;
            int y = row * (button_height)+corner_start;

            
            Fl_Button* button = new Fl_Button(x, y, button_width, button_height, "");
            buttons.push_back(button);
            // std::cout<<"button made"<<std::endl;

            window->add(button);

            

            int* index = new int(currentPosition);

            if(label == "s"){
                button->callback(safe_click, index);
            }
            else{
                button->callback(bomb_click, index);
            }
            

            //increment position by 1
            currentPosition++;

        }
    }
    //__________________________________________________________________________________________________________________________


    //QUIT BUTTON
    // std::cout<<"HERE"<<std::endl;
    Fl_Button* quit = new Fl_Button(780, 0, 20, 20, "X");
    //pass window as data
    quit->callback(quitButton, window);
    window->add(quit);


    //__________________________________________________________________________________________________________________________
    //create the vector of touching spaces
    touchingBoard = touching(totalSpaces, bombLocations, cols);
    
    for(int i = 0; i< n*n;i++){
        if(touchingBoard[i] == 0){
            reveal(i);
            break;
        }
    }
    
    
    window->redraw();

    //tester
    // for (int i = 0; i < rows; ++i) {
    //     for (int j = 0; j < cols; ++j) {
            
    //         std::cout << touchingBoard[i * cols + j] << " ";
    //     }
        
    //     std::cout << std::endl;
    // }

}

int main(){
    int winWidth = 800;
    int winHeight = 800;

    Fl_Window* window = new Fl_Window(winWidth, winHeight, "Minesweeper");


    Fl_Button* beginnerButton = new Fl_Button(20, 20, 80, 30, "Beginner");
    //pass window as data
    beginnerButton->callback(diff_buttonBeginner, window);


    Fl_Button* intermediateButton = new Fl_Button(360, 20, 80, 30, "Intermediate");
    //pass window as data
    intermediateButton->callback(diff_buttonIntermediate, window);


    Fl_Button* expertButton = new Fl_Button(700, 20, 80, 30, "Expert");
    //pass window as data
    expertButton->callback(diff_buttonExpert, window);

    //QUIT BUTTON
    // std::cout<<"HERE"<<std::endl;
    Fl_Button* quit = new Fl_Button(780, 0, 20, 20, "X");
    //pass window as data
    quit->callback(quitButton, window);


    window->end();
    window->show();

    return Fl::run();
}