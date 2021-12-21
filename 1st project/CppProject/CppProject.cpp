// CppProject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <chrono>
#include "CppProject.h"
// define information
int cash = 50;                                                               // amount of cash user got
int stock [5] = { 2,4,1,10,8 };                                             // available products
int cost[5] = {1,3,2,3,9};                                                 // cost of the products
int inventory[5] = { 0,0,0,0,0 };                                         // inventory
std::string items[5] = {"banana","apple","pear","orange","marshmallow"}; // list of products

void Write(std::string outpot, bool eL = false, int speed = 100) {
    auto t1 = std::chrono::steady_clock::now();
    int i = 0;
    bool gg = true;
    while (gg && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t1).count() < speed) {
        if (i != outpot.length()) {
            std::cout << outpot[i];
            i = i + 1;
            t1 = std::chrono::steady_clock::now();
        }
        else {
            gg = false;
        }
    }
    if (eL)std::cout << "\n";
}

void listen() {                                                        // listen for input loop
    std::string input;                                               // input from cmdline
    std::cin >> input;                                              // fills variable with input
    if (input == "exit") {                                         // if they type exit, exit the shop
        std::cout << "You exited the shop.\n";
        return;                                                  // cancel the rest from running, causing the console app to close
    }
    else if (input == "cash") {
        std::cout << cash << "\n";                              // tell them their amount of cash
    }else if(input == "help"||input=="?"){
        std::cout << "\nCommands:\n- inv  | Your inventory\n- cash | How much you got\n- exit | Exit store and close the process\n\n";
    }
    else if (input == "inv" || input == "inventory") {
        for (int i = 0; i < 5; i++) {
            std::cout << items[i]<<": " << inventory[i] <<"\n"; // loop inventory, tell "name: amount"
        }
    }
    else {
        auto t1 = std::chrono::steady_clock::now();
        int it = -1;
        for (int i = 0; i < 5; i++) {                        // loop existing items
            if (input == items[i]) {it = i;break;}
        }
        if (it != -1) {
            if (stock[it] == 0) {                       // does it have any in stock?
                std::cout << "Out of stock.\n";
            }
            else {
                std::cout << "How many would you like?\n";
                int amount;
                std::cin >> amount;                           // could use std::stoi
                if (amount > stock[it]) amount = stock[it];    // if there is less in stock than desired, give them the full stock
                if (cash > (cost[it] * amount)) {            // can they afford it?
                    cash = cash - (cost[it] * amount);      // take their money
                    stock[it] = stock[it] - amount;      // remove from stock
                    std::cout << "That was expensive (" << (cost[it] * amount) << ") for x" << amount << " " << input << "\nNow you have " << cash << " cash left\n";
                    inventory[it] = inventory[it] + amount;
                }
                else {
                    std::cout << "You're too poor.\nYou can afford " << (cash / cost[it]) << " of those.\n";
                }
            }
        }else {
            Write("Invalid item. Try lowercase or check your spelling.");
        }
        auto t2 = std::chrono::steady_clock::now();
        auto d_micro = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        std::cout <<"\n" << d_micro << " nanoseconds\n";
    }
    listen(); // listen again for input
}
int main()
{
    std::cout << "Welcome to the shop!\nType 'help' for help\n\n";
    for (int i = 0; i < 5; i++) {
        std::cout << items[i] << " ("<<cost[i]<<") x"<<stock[i]<<(i<4?" | ":""); // log each item (price) xStock
    }
    std::cout << "\nYou got "<<cash << " to use.\n";
    listen(); // begin the input listening loop
}

/* Run program : Ctrl + F5 or Debug > Start Without Debugging menu
   Debug program: F5 or Debug > Start Debugging menu

 Tips for Getting Started: 
   1. Use the Solution Explorer window to add/manage files
   2. Use the Team Explorer window to connect to source control
   3. Use the Output window to see build output and other messages
   4. Use the Error List window to view errors
   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
*/
