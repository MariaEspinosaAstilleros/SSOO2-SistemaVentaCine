/******************************************************
 * Project:         Práctica 3 de Sistemas Operativos II
 * 
 * Program name:    cinemaSale.cpp
 
 * Author:          María Espinosa Astilleros
 * 
 * Date created:    2/4/2020
 * 
 * Purpose:         
 * 
 ******************************************************/

#include <iostream>
#include <queue>
#include <vector>
#include <mutex> 
#include <thread> 
#include <condition_variable> 
#include <chrono>
#include <future> 

#include "../include/color.h"
#include "../include/msgRequest.h"
#include "../include/SemCounter.h"

#define NUM_SEATS   72
#define NUM_SP      3
#define NUM_CLIENTS 20
#define MAX_REQUEST 5

/*Globals variables*/
int turn = -1; 

/*Messages queue*/
std::queue<std::thread>                 g_queue_tickets;            /*queue of clients to buy tickets*/
std::queue<std::thread>                 g_queue_drinkpop;           /*queue of clients to buy drinks and popcorn*/
std::queue<std::thread>                 g_queue_cinema;             /*queue representing cinema*/
std::queue<MsgRequestTickets*>          g_queue_request_tickets;    /*queue to request tickets*/
std::queue<MsgRequestSalePoint*>        g_queue_request_pv;         /*queue to request sale point*/
std::queue<MsgRequestStock*>            g_queue_request_stock;      /*queue to request thread stocker*/
std::priority_queue<MsgRequestPay*>     g_queue_request_pay;        /*queue to request pay*/

/*Vectors*/
std::vector<std::thread>                g_v_sale_point;              /*vector with id of sale point*/

/*Semaphores*/
std::mutex                              g_sem_tickets;              /*sem to wait tickets*/
std::mutex                              g_sem_toffice;              /*sem to wake ticket office*/
std::mutex                              g_sem_seats;                /*sem to control seats*/
std::mutex                              g_sem_manager;              /*sem to manager send a new turn*/

/*Condition variable*/
std::condition_variable                 g_cv_ticket_office;         /*condition variable to notify the turn of ticket office*/
std::condition_variable                 g_cv_sale_point;            /*condition variable to notify the turn of sale point*/
std::condition_variable                 g_cv_client_attended;       /*conditional variable to notify the client that it has been attended*/
std::condition_variable                 g_cv_pay;                   /*condition variable to notify the turn to pay*/
std::condition_variable                 g_cv_stock_attended;        /*conditional variable to notify the sale point that it has been replanished*/

/*Functions declaration*/
int     generateRandomNumber(int lim); 
void    signalHandler(); 
void    createClients(); 
void    createSalePoints(); 
void    client(int id_client); 
void    salePoint(int id_sale_point);


/******************************************************
 * Function name:    main
 * Date created:     4/4/2020
 * Input arguments: 
 * Purpose:          Principal method of class 
 * 
 ******************************************************/
int main(int argc, char *argv[]){
    
    std::signal(SIGINT, signalHandler); /*It installs the signal handler*/

}

/******************************************************
 * Function name:    generateRandomNumber
 * Date created:     4/4/2020
 * Input arguments: 
 * Purpose:          Generate a random number 
 * 
 ******************************************************/
int generateRandomNumber(int lim){ return (rand()%(lim-1))+1; }

/******************************************************
 * Function name:    createClients
 * Date created:     11/4/2020
 * Input arguments:  
 * Purpose:          Create the clients 
 * 
 ******************************************************/
void createClients(){

    for(int i = 1; i < NUM_CLIENTS; i++){
        g_queue_tickets.push(std::thread(client, i));
        std::this_thread::sleep_for(std::chrono::milliseconds(400)); 
    }
}

/******************************************************
 * Function name:    client
 * Date created:     11/4/2020
 * Input arguments:  id of client 
 * Purpose:          It simulate the client. The client go to the ticket office and when the client has the turn send the request of tickets.
 *                   When the ticket office attends the request, the client waits. If there are sufficient tickets the client 
 *                   come in the cinema and buy drinks and popcorn
 * 
 ******************************************************/
void client(int id_client){

}

/******************************************************
 * Function name:    createSalePoints
 * Date created:     11/4/2020
 * Input arguments:  
 * Purpose:          Create the sale points 
 * 
 ******************************************************/
void createSalePoints(){

    for(int i = 1; i < NUM_SP; i++){
        g_v_sale_point.push_back(std::thread(salePoint, i)); 
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

}

/******************************************************
 * Function name:    salePoint
 * Date created:     11/4/2020
 * Input arguments:  
 * Purpose:          Create the sale points 
 * 
 ******************************************************/
void salePoint(int id_sale_point){

}

/******************************************************
 * Function name:    signalHandler
 * Date created:     11/4/2020
 * Input arguments: 
 * Purpose:          Signal Handler to show a message when the user uses CTRL + C 
 * 
 ******************************************************/
void signalHandler(){

    std::cout << BOLDWHITE << "Se ha recibido la señal CTRL + C. Finalizamos el programa..." << RESET << std::endl; 
    std::terminate(); 
    std::exit(EXIT_SUCESS); 
}