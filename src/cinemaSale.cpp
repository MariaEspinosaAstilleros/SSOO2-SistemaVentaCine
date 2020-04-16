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
#define PAY_TO      1 /*pay to ticket office*/
#define PAY_SP      2 /*pay to sale point*/

/*Globals variables*/
int turn = -1; 
int g_payment_priority; 

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
std::mutex                              g_sem_mutex_pay;            /*sem to control the critical section in pay*/
std::mutex                              g_sem_wait_pay;             /*sem to control pay confirmation*/

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
void                 blockSem();
int                  priorityAssignment();
void                 createClients(); 
void                 createSalePoints(); 
void                 client(int id_client); 
void                 checkTicketsClient(int id_client, MsgRequestTickets mrt);
void                 ticketOffice();
void                 salePoint(int id_sale_point);
void                 buyDrinksPopcorn(int id_client);
void                 replenisher();
void                 paymentSystem();
void                 manager(); 

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
 * Function name:    blockSem
 * Date created:     16/4/2020
 * Input arguments:  
 * Purpose:          Block all semaphores 
 * 
 ******************************************************/
void blockSem(){

}

/******************************************************
 * Function name:    priorityAssignment
 * Date created:     16/4/2020
 * Input arguments:  
 * Purpose:          Assign priority to clients when paying.
 *                   The clients to pay to ticket office has 20% of priority and the clients to pay to sale point has 80% of priority
 * 
 ******************************************************/
int priorityAssignment(int type_payment){
    int priority = 0; 
    int num_random = generateRandomNumber(100); 

    switch (type_payment){
        case PAY_TO:
            if(num_random >= 80){ 
                priority = 1;
            }else{
                priority = 2;
            }
            break;
    
        case PAY_SP:
            if(num_random < 80){
                priority = 2;
            }else{
                priority = 1; 
            }
            break;
    }

    return priority; 
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

    /*Wait turn of office ticket*/
    std::unique_lock<std::mutex> ul_turn(g_sem_turn); 
    g_cv_ticket_office.wait(ul_turn, [id_client]{return turn == id_client;}); 
    std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] It's my turn for buy tickets!" << RESET << std::endl; 

    /*Generate the request to buy a tickets*/
    MsgRequestTickets mrt(id_client, generateRandomNumber(MAX_REQUEST));
    g_queue_request_tickets.push(&mrt); 
    std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] I want " << std::to_string(mrt.num_seats) << " tickets" << RESET << std::endl; 
    std::this_thread::sleep_for(std::chrono::milliseconds(400)); //sleep the thread each time that the client buys tickets

    /*Unlocked the ticket office and wait to receive tickets*/
    g_sem_toffice.unlock(); 
    g_sem_tickets.lock(); 

    /*Check if there are enough tickets for the client*/
    checkTicketsClient(id_client, mrt); 
}

/******************************************************
 * Function name:    checkTicketsClient
 * Date created:     12/4/2020
 * Input arguments:  
 * Purpose:          Check if there are enough tickets for the client 
 * 
 ******************************************************/
void checkTicketsClient(int id_client, MsgRequestTickets mrt){

    /*Check it the client has sufficient seats and it can buy drinks and popcorn*/
    if(mrt.suff_seats){
        /*The client goes inside the cinema*/
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
 * Purpose:          It simulate the ticket office  
 * 
 ******************************************************/
void ticketOffice(){
    int num_seats = NUM_SEATS; 
    std::cout << GREEN << "[TICKET OFFICE] Ticket office open" << RESET << std::endl;

    /*Seat allocation and payment system simultaneous*/      
    //CUANDO USO ESTA VAINA¿?¿?¿?¿?
    std::unique_lock<std::mutex> ul_ticket_office(g_sem_toffice);
    std::unique_lock<std::mutex> ul_payment(g_sem_pay); 

    /*Assign payment priority*/
    g_payment_priority = priorityAssignment(PAY_TO); 

    while(true){
        g_sem_toffice.lock(); 

        MsgRequestTickets *mrt = g_queue_request_tickets.front(); 
        g_queue_request_tickets.pop(); 

        if(num_seats >= mrt->num_seats){
            std::cout << GREEN << "[TICKET OFFICE] The client " << mrt->id_client << " has bought " << mrt->num_seats << RESET << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(400)); 

            /*Request to payment system*/
            MsgRequestPay mrp(mrt->id_client, g_payment_priority);
            g_queue_request_pay.push(&mrp);

            /*Wait confirmation of payment system*/
            std::unique_lock ul_wait_payment(g_sem_wait_pay); 
            bool *p_flag_attended = &(mrp.attended);
            g_cv_pay.wait(ul_wait_payment, [p_flag_attended] {return *p_flag_attended;}); 
            ul_wait_payment.unlock();

            /*Check if the payment was successful*/
            if(mrp.attended){
                /*Updated the number of tickets left*/
                num_seats -= mrt->num_seats; 
                mrt->suff_seats = true; 
            }else{
                std::cout << GREEN << "[TICKET OFFICE] The client " << mrt->id_client << " has requested more tickets than there are left" << RESET << std::endl;
                mrt->suff_seats = false; 
            }
        }  
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
    std::cout << MAGENTA << "[SALE POINT " << std::to_string(id_sale_point) << "] Created" << RESET << std::endl;
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
 * Function name:    stock
 * Date created:     16/4/2020
 * Input arguments:  
 * Purpose:          It simulate the replenisher
 * 
 ******************************************************/
void replenisher(){
    std::cout << RED << "[REPLENISHER] Created and waiting to receive requests" << RESET << std::endl; 

}


/******************************************************
 * Function name:    paySystem 
 * Date created:     13/4/2020
 * Input arguments: 
 * Purpose:          It simulate the pay system
 * 
 ******************************************************/
void paymentSystem(){
    std::cout << BLUE << "[PAYMENT SYSTEM] Payment system open" << RESET << std::endl; 

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
        if(i>1){ul.lock();} //block the ticket office  
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

    /*Block all semaphores*/
    //blockSem(); 
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    /*Create the threads*/
    std::thread thread_ticket_office(ticketOffice); 
    std::thread thread_pay(paymentSystem); 
    createSalePoints(); 
    std::thread thread_clients(createClients); 
    std::thread thread_manager(manager); 
    std::thread thread_replenisher(replenisher); 

    /*Wait threads end*/ 

    return EXIT_SUCCESS; 
}