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
#include <csignal>
#include <string>
#include <signal.h>
#include <unistd.h>

#include "../include/color.h"
#include "../include/msgRequest.h"
#include "../include/SemCounter.h"

#define NUM_SEATS   72
#define NUM_SP      3
#define NUM_CLIENTS 20
#define MAX_REQUEST 5
#define PAY_TO      1 /*pagamos a la taquilla*/
#define PAY_SP      2 /*pagamos al punto de venta*/

/*Globals variables*/
int turn = -1; 

/*Messages queue*/
std::queue<std::thread>                 g_queue_tickets;            /*queue of clients to buy tickets*/
std::queue<std::thread>                 g_queue_clients_out;        /*queue of clients that not buy tickets*/
std::queue<std::thread>                 g_queue_drinkpop;           /*queue of clients to buy drinks and popcorn*/
std::queue<std::thread>                 g_queue_cinema;             /*queue representing cinema*/
std::queue<std::thread>                 g_queue_inside_cinema;      /*queue when the client bought the tickets and go to inside of cinema*/
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
std::mutex                              g_sem_turn;                 /*sem to control the turn*/
std::mutex                              g_sem_pay;                  /*sem to control pay*/

/*Condition variable*/
std::condition_variable                 g_cv_ticket_office;         /*condition variable to notify the turn of ticket office*/
std::condition_variable                 g_cv_sale_point;            /*condition variable to notify the turn of sale point*/
std::condition_variable                 g_cv_client_attended;       /*conditional variable to notify the client that it has been attended*/
std::condition_variable                 g_cv_pay;                   /*condition variable to notify the turn to pay*/
std::condition_variable                 g_cv_stock_attended;        /*conditional variable to notify the sale point that it has been replanished*/

/*Functions declaration*/
int                  generateRandomNumber(int lim); 
void                 signalHandler(int signal); 
void                 messageWelcome(); 
void                 createClients(); 
void                 createSalePoints(); 
void                 ticketOffice();
void                 client(int id_client); 
void                 stateClientTickets(int id_client, MsgRequestTickets mrt);
void                 salePoint(int id_sale_point);
void                 buyDrinksPopcorn(int id_client);
void                 paySystem(); 
MsgRequestTickets    buyTickets(int id_client);

/******************************************************
 * Function name:    generateRandomNumber
 * Date created:     4/4/2020
 * Input arguments: 
 * Purpose:          Generate a random number 
 * 
 ******************************************************/
int generateRandomNumber(int lim){
    return (rand()%(lim-1))+1;
}

/******************************************************
 * Function name:    messageWelcome
 * Date created:     12/4/2020
 * Input arguments: 
 * Purpose:          Show a message welcome
 * 
 ******************************************************/
void messageWelcome(){
    std::cout << BOLDCYAN << "*********************************************" << RESET << std::endl;
    std::cout << BOLDCYAN << "* WELCOME TO SALES SYSTEM ONLINE IN CINEMAS *" << RESET << std::endl; 
    std::cout << BOLDCYAN << "*********************************************" << RESET << std::endl;
}
/******************************************************
 * Function name:    createClients
 * Date created:     11/4/2020
 * Input arguments:  
 * Purpose:          Create the clients 
 * 
 ******************************************************/
void createClients(){

    for(int i = 1; i <= NUM_CLIENTS; i++){
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

    std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] Created and waiting at buy a tickets..." << RESET << std::endl; 
    MsgRequestTickets mrt = buyTickets(id_client); 
    stateClientTickets(id_client, mrt);

}

/******************************************************
 * Function name:    buyTickets
 * Date created:     12/4/2020
 * Input arguments:  
 * Purpose:          The client buys tickets 
 * 
 ******************************************************/
MsgRequestTickets buyTickets(int id_client){

    /*Wait turn of office ticket*/
    std::unique_lock<std::mutex> ul_turn(g_sem_turn); 
    g_cv_ticket_office.wait(ul_turn, [id_client]{return turn == id_client;}); 
    std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] It's my turn for buy tickets!" << RESET << std::endl; 

    /*Generate the request to buy a tickets*/
    MsgRequestTickets mrt(id_client, generateRandomNumber(MAX_REQUEST));
    g_queue_request_tickets.push(&mrt); 
    std::this_thread::sleep_for(std::chrono::milliseconds(400)); //duermo al hilo cada vez que compra

    /*Unlocked the ticket office and wait to receive tickets*/
    g_sem_toffice.unlock(); 
    g_sem_tickets.lock(); 

    return mrt; 
}

/******************************************************
 * Function name:    stateClientTickets
 * Date created:     12/4/2020
 * Input arguments:  
 * Purpose:          If the client has tickets go inside the cinema and buy drinks and popcorn.  
 * 
 ******************************************************/
void stateClientTickets(int id_client, MsgRequestTickets mrt){

    /*Check it the client has sufficient seats and it can buy drinks and popcorn*/
    if(mrt.suff_seats){
        /*The client changes to queue of drinks and popcorn*/
        std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] I have the tickets already, I go to buy drinks and popcorn..." << RESET << std::endl; 
        g_queue_inside_cinema.push(std::move(g_queue_tickets.front()));
        g_queue_tickets.pop(); 
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        /*Aquí va el metodo donde compro las bebidas y las palomitas*/
        buyDrinksPopcorn(id_client); 
        std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] I have everything already. I go to see Harry Potter now! :)" << RESET << std::endl;
    }else{
        g_queue_clients_out.push(std::move(g_queue_tickets.front()));
        g_queue_tickets.pop();
        std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] No tickets left so I go to my house :(" << RESET << std::endl;
    }
}

/******************************************************
 * Function name:    ticketOffice
 * Date created:     12/4/2020
 * Input arguments:  
 * Purpose:          If simulate the ticket office  
 * 
 ******************************************************/
void ticketOffice(){

    std::cout << GREEN << "[TICKET OFFICE] Ticket office open" << RESET << std::endl; 

    while(true){
        g_sem_toffice.lock(); 
        MsgRequestTickets *mrt = g_queue_request_tickets.front();
        g_queue_request_tickets.pop(); 
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); 

        /*Asignamos los asientos y cobramos las entradas de forma simultanea*/


    }

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
 * Function name:    salePoint
 * Date created:     11/4/2020
 * Input arguments:  
 * Purpose:          Create the sale points 
 * 
 ******************************************************/
void buyDrinksPopcorn(int id_client){

}

/******************************************************
 * Function name:    paySystem(); 
 * Date created:     13/4/2020
 * Input arguments: 
 * Purpose:          It simulate the pay system
 * 
 ******************************************************/
void paySystem(){
    std::cout << BLUE << "[PAY SYSTEM] Pay system open" << RESET << std::endl; 

    /*while(true){

    }*/

}

/******************************************************
 * Function name:    signalHandler
 * Date created:     11/4/2020
 * Input arguments: 
 * Purpose:          Signal Handler to show a message when the user uses CTRL + C 
 * 
 ******************************************************/
void signalHandler(int signal){

    std::cout << BOLDWHITE << "[HANDLER] It has been received the signal CTRL+C. The program ended...\n" << RESET << std::endl; 
    kill(getpid(), SIGKILL); 
    std::exit(EXIT_SUCCESS); 
}

/******************************************************
 * Function name:    manager
 * Date created:     13/4/2020
 * Input arguments: 
 * Purpose:          Generate the turns to the clients access to ticket office 
 * 
 ******************************************************/
void manager(){
    std::unique_lock<std::mutex> ul(g_sem_toffice); 

    for(int i = 1; i < NUM_CLIENTS; i++){
        if(i>1){ul.lock();} //bloqueo la taquilla 
        std::cout << CYAN << "[MANAGER] I'ts the turn of thread " << std::to_string(i) << RESET << std::endl;
        turn = i; 
        g_cv_ticket_office.notify_all();  
        ul.unlock();
        g_sem_manager.lock(); 

        std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
    }
}

/******************************************************
 * Function name:    main
 * Date created:     4/4/2020
 * Input arguments: 
 * Purpose:          Principal method of class 
 * 
 ******************************************************/
int main(int argc, char *argv[]){
    
    if(std::signal(SIGINT, signalHandler) == SIG_ERR){ /*It installs the signal handler*/
        std::cout << BOLDWHITE << "[MAIN] ERROR. The signal CRTL+C hasn't been received correctly \n" << RESET << std::endl; 
    } 

    messageWelcome();

    /*Bloqueamos todos los semaforos*/

    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    /*Creamos los hilos*/
    //std::thread thread_ticket_office(ticketOffice); 
    //std::thread thread_pay(paySystem); 
    //std::thread thread_sale_point(createSalePoint);
    //std::thread thread_clients(createClients); 
    //std::thread thread_manager(manager); 

    /*Espero que finalicen los hilos*/

    ticketOffice();
    paySystem(); 
    createClients();
    manager(); 
    

    return EXIT_SUCCESS; 

}