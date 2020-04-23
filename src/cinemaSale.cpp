/******************************************************
 * Project:         Práctica 3 de Sistemas Operativos II
 * 
 * Program name:    cinemaSale.cpp
 
 * Author:          María Espinosa Astilleros
 * 
 * Date created:    2/4/2020
 * 
 * Purpose:         It simulates a sales system online in cinemas
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

#define NUM_SEATS               72
#define NUM_SP                  3
#define NUM_CLIENTS             10
#define MAX_REQUEST_TICKETS     6
#define MAX_REQUEST_DRINK_POP   11
#define PAY_TO                  1 
#define PAY_SP                  2 

/*Globals variables*/
int g_payment_priority;
int g_shift             = 0;
int g_num_seats         = NUM_SEATS;

/*Messages queue*/
std::queue<std::thread>                 g_queue_tickets;            /*queue of clients to buy tickets*/
std::queue<std::thread>                 g_queue_clients_out;        /*queue of clients that not buy tickets*/
std::queue<std::thread>                 g_queue_drinkpop;           /*queue of clients to buy drinks and popcorn*/
std::queue<std::thread>                 g_queue_cinema;             /*queue representing cinema*/
std::queue<std::thread>                 g_queue_inside_cinema;      /*queue when the client bought the tickets and go to inside of cinema*/
std::queue<MsgRequestTickets*>          g_queue_request_tickets;    /*queue to request tickets*/
std::queue<MsgRequestSalePoint*>        g_queue_request_pv;         /*queue to request sale point*/
std::queue<MsgRequestStock*>            g_queue_request_stock;      /*queue to request thread stocker*/
std::priority_queue<MsgRequestPayment*> g_queue_request_payment;    /*queue to request pay*/

/*Vectors*/
std::vector<std::thread>                g_v_sale_point;              /*vector with id of sale point*/
std::vector<std::mutex*>                g_v_sem_sale_point;          /*vector of semaphores for points of sale*/

/*Semaphores*/
SemCounter                              g_sem_seats(1);             /*sem to control seats*/
SemCounter                              g_sem_payment(1);           /*sem to control payment*/
SemCounter                              g_sem_replenisher(1);       /*sem to control thread replenisher*/
SemCounter                              g_sem_sale_point(0);        /*sem to control food request*/
std::mutex                              g_sem_tickets;              /*sem to wait tickets*/
std::mutex                              g_sem_toffice;              /*sem to wake ticket office*/
std::mutex                              g_sem_manager;              /*sem to manager send a new shift*/
std::mutex                              g_sem_shift;                /*sem to control the shift*/
std::mutex                              g_sem_wait_payment;         /*sem to control payment confirmation*/
std::mutex                              g_sem_wait_sale_point;      /*sem to control sale point confirmation*/
std::mutex                              g_sem_wait_drinkpop;        /*sem to control drinks and popcorn confirmation*/
std::mutex                              g_sem_mutex_access_to;      /*sem to control the access to tickets request queue*/
std::mutex                              g_sem_mutex_access_payment; /*sem to control the access to payment request queue*/
std::mutex                              g_sem_mutex_access_sp;      /*sem to control the access to sale points request queue*/
std::mutex*                             g_sem_mutex_sale_point;     /*sem to control sale in sale points*/

/*Condition variable*/
std::condition_variable                 g_cv_ticket_office;         /*condition variable to notify the shift of ticket office*/
std::condition_variable                 g_cv_sale_point;            /*condition variable to notify the shift of sale point*/
std::condition_variable                 g_cv_client_attended;       /*conditional variable to notify the client that it has been attended*/
std::condition_variable                 g_cv_payment;               /*condition variable to notify the shift to pay*/
std::condition_variable                 g_cv_stock_attended;        /*conditional variable to notify the sale point that it has been replanished*/

/*Functions declaration*/
int                  generateRandomNumber(int lim); 
void                 signalHandler(int signal); 
void                 messageWelcome(); 
void                 showInfo(); 
void                 blockSem();
int                  priorityAssignment();
void                 createClients(); 
void                 createSalePoints(); 
void                 client(int id_client); 
MsgRequestTickets    buyTickets(int id_client);
void                 checkTicketsClient(int id_client, MsgRequestTickets mrt);
void                 ticketOffice();
void                 checkNumTickets(MsgRequestTickets *mrt);
void                 checkPayment(MsgRequestPayment mrp, MsgRequestTickets *mrt); 
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
int generateRandomNumber(int lim){ return (rand()%(lim-1))+1; }

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
    std::cout << CURVCYAN << "*********************************************" << RESET << std::endl;
    std::cout << CURVCYAN << "* WELCOME TO SALES SYSTEM ONLINE IN CINEMAS *" << RESET << std::endl; 
    std::cout << CURVCYAN << "*********************************************" << RESET << std::endl;

    showInfo(); 
}

/******************************************************
 * Function name:    showInfo
 * Date created:     22/4/2020
 * Input arguments: 
 * Purpose:          Show color information
 * 
 ******************************************************/
void showInfo(){
    std::cout << "COLOR LEGEND: " << std::endl; 
    std::cout << CYAN << "Manager generates the shifts" << RESET << std::endl; 
    std::cout << YELLOW << "Clients waits to buy tickets, drinks and popcorn" << RESET << std::endl; 
    std::cout << GREEN << "Ticket office attends the clients and selling tickets" << RESET << std::endl; 
    std::cout << BLUE << "Payment system makes payment of ticket office and sale points" << RESET << std::endl; 
    std::cout << MAGENTA << "Sale points sell drinks and popcorn" << RESET << std::endl; 
    std::cout << RED << "Replenisher will replenish drinks and popcorn" << RESET << std::endl; 
    std::cout << "Signal handler ends the program" << std::endl; 
    std::cout << "" << std::endl; 
    std::cout << YELLOW << NUM_CLIENTS << RESET << " clients and " << GREEN  << NUM_SEATS << RESET << " seats will be created to simulate the system"  << std::endl; 
    std::cout << "To end the program press CTRL + C" << std::endl; 
    std::cout << "To start the program press ENTER" << std::endl; 
    getchar(); 
}

/******************************************************
 * Function name:    blockSem
 * Date created:     16/4/2020
 * Input arguments:  
 * Purpose:          Block semaphores 
 * 
 ******************************************************/
void blockSem(){
    g_sem_tickets.lock();             
    g_sem_toffice.lock();                              
    g_sem_manager.lock();                
    g_sem_payment.wait(); 

    /*for(int i = 0; i < NUM_SP; i++){
        g_v_sem_sale_point.push_back(g_sem_mutex_sale_point);
        g_v_sem_sale_point.at(i)->lock(); 
    }          */                    
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
    int priority     = 0; 
    int num_random   = generateRandomNumber(100); 

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
        std::this_thread::sleep_for(std::chrono::milliseconds(450)); 
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

    for(int i = 1; i <= NUM_SP; i++){
        g_v_sale_point.push_back(std::thread(salePoint, i)); 
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

/******************************************************
 * Function name:    client
 * Date created:     11/4/2020
 * Input arguments:  id of client 
 * Purpose:          It simulate the client. The client go to the ticket office and when the client has the shift send the request of tickets.
 *                   When the ticket office attends the request, the client waits. If there are sufficient tickets the client 
 *                   come in the cinema and buy drinks and popcorn
 * 
 ******************************************************/
void client(int id_client){

    std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] Created and waiting to buy tickets..." << RESET << std::endl;
    MsgRequestTickets mrt = buyTickets(id_client); 
    checkTicketsClient(id_client, mrt); 
}

/******************************************************
 * Function name:    buyTickets
 * Date created:     12/4/2020
 * Input arguments:  
 * Purpose:          The client buys tickets
 * 
 ******************************************************/
MsgRequestTickets buyTickets(int id_client){
    /*Wait shift of office ticket*/
    std::unique_lock<std::mutex> ul_shift(g_sem_shift); 
        g_cv_ticket_office.wait(ul_shift, [id_client]{return g_shift == id_client;}); 
        std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] It's my shift for buy tickets!" << RESET << std::endl; 
    ul_shift.unlock(); 

    /*Generate the request to buy a tickets*/
    MsgRequestTickets mrt(id_client, generateRandomNumber(MAX_REQUEST_TICKETS));
    g_queue_request_tickets.push(&mrt); 
    std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] I want " << std::to_string(mrt.num_seats) << " tickets" << RESET << std::endl; 

    /*Unlocked the ticket office and wait to receive tickets*/
    g_sem_toffice.unlock(); 
    g_sem_tickets.lock(); 

    return mrt;
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
    if(mrt.suff_seats == true){
        /*The client goes inside the cinema*/
        std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] I have the tickets already. I go to buy drinks and popcorn..." << RESET << std::endl; 
        g_sem_manager.unlock(); /*It unlocks the shift to the next client sends the request*/

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        g_queue_inside_cinema.push(std::move(g_queue_tickets.front()));
        g_queue_tickets.pop(); 

        /*The client buys drinks and popcorn*/
        buyDrinksPopcorn(id_client); 
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] I have everything already. I go to see Harry Potter now! :)" << RESET << std::endl;
    }else{
        g_queue_clients_out.push(std::move(g_queue_tickets.front()));
        g_queue_tickets.pop();
        std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] No tickets left so I go to my house :(" << RESET << std::endl;
        g_sem_manager.unlock(); /*It unlocks the shift to the next client sends the request*/
    }
}

/******************************************************
 * Function name:    buyDrinksPopcorn
 * Date created:     23/4/2020
 * Input arguments:  
 * Purpose:          The client sends a request to the sale points to buy drinks and popcorn 
 * 
 ******************************************************/
void buyDrinksPopcorn(int id_client){
    MsgRequestSalePoint mrsp(id_client, generateRandomNumber(MAX_REQUEST_DRINK_POP), generateRandomNumber(MAX_REQUEST_DRINK_POP)); 
    g_queue_request_pv.push(&mrsp); 

    std::unique_lock<std::mutex> ul_wait_sp(g_sem_wait_sale_point); /*bloqueo hasta que le toque a algun punto de venta atender*/
        g_sem_sale_point.signal(); 
        int *p_flag_id = &(mrsp.id_sale_point); 
        g_cv_sale_point.wait(ul_wait_sp, [p_flag_id]{return *p_flag_id != 0; }); /*si es distinto de 0 es porque ya puede atender un punto de venta*/
        std::cout << YELLOW << "[CLIENT " << std::to_string(id_client) << "] I want " << std::to_string(mrsp.num_drinks) << " drinks and ";
        std::cout << std::to_string(mrsp.num_drinks) << " popocorn" << RESET << std::endl;
    ul_wait_sp.unlock(); 

    int index = *p_flag_id - 1; 
    g_v_sem_sale_point.at(index)->unlock; /*desbloqueo el punto de venta que vaya a venderme la bebida y las palomitas*/

    /*Wait confirmation of sale point*/
    std::unique_lock<std::mutex> ul_wait_drinkpop(g_sem_wait_drinkpop);
        bool *p_flag_attended = &(mrsp.attended);
        g_cv_payment.wait(ul_wait_drinkpop, [p_flag_attended] {return *p_flag_attended;}); 
    ul_wait_drinkpop.unlock(); 
}

/******************************************************
 * Function name:    ticketOffice
 * Date created:     12/4/2020
 * Input arguments:  
 * Purpose:          It simulate the ticket office  
 * 
 ******************************************************/
void ticketOffice(){
    std::cout << GREEN << "[TICKET OFFICE] Ticket office open" << RESET << std::endl; 

    while(true){
        try{
            g_sem_toffice.lock();
            g_payment_priority = priorityAssignment(PAY_TO); 
        
            /*Control the access to tickets request queue*/
            g_sem_mutex_access_to.lock(); 
                MsgRequestTickets *mrt = g_queue_request_tickets.front(); 
                g_queue_request_tickets.pop(); 
            g_sem_mutex_access_to.unlock(); 

            /*Check number of tickets*/
            checkNumTickets(mrt);
            std::cout << GREEN << "[TICKET OFFICE] The client " << std::to_string(mrt->id_client) << " has been attended" << RESET << std::endl;
            g_sem_tickets.unlock(); /*It unlocks to attend other clients*/ 
            
        }catch(std::exception &e){
            std::cout << GREEN << "[TICKET OFFICE] An error occurred while attending clients..." << RESET << std::endl;
            g_sem_tickets.unlock();
        }
         
    }
}

/******************************************************
 * Function name:    checkNumTickets
 * Date created:     22/4/2020
 * Input arguments:  
 * Purpose:          Check tickets
 * 
 ******************************************************/
void checkNumTickets(MsgRequestTickets *mrt){
    if(g_num_seats >= mrt->num_seats){
        std::cout << GREEN << "[TICKET OFFICE] The client " << mrt->id_client << " has requested " << mrt->num_seats << RESET << std::endl; 

        /*Request to payment system*/
        MsgRequestPayment mrp(mrt->id_client, g_payment_priority);
        g_queue_request_payment.push(&mrp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); /*sleep the thread each time that the client pays tickets*/
 
        /*Wait confirmation of payment system*/
        std::unique_lock<std::mutex> ul_wait_payment(g_sem_wait_payment); 
            /*Seat allocation and payment system simultaneous*/
            g_sem_payment.signal();  
            g_sem_seats.signal();
            bool *p_flag_attended = &(mrp.attended);
            g_cv_payment.wait(ul_wait_payment, [p_flag_attended] {return *p_flag_attended;}); 
        ul_wait_payment.unlock();

        /*Check if the payment was successful*/
        checkPayment(mrp, mrt);
    }else{
        std::cout << GREEN << "[TICKET OFFICE] The client " << std::to_string(mrt->id_client) << " has requested more tickets than there are left" << RESET << std::endl;
        mrt->suff_seats = false; 
    }
}

/******************************************************
 * Function name:    checkPayment
 * Date created:     22/4/2020
 * Input arguments:  
 * Purpose:          Check if the payment was successful 
 * 
 ******************************************************/
void checkPayment(MsgRequestPayment mrp, MsgRequestTickets *mrt){
    if(mrp.attended == true){ 
        /*Updated the number of tickets left*/
        g_sem_seats.wait(); 
        g_num_seats     -= mrt->num_seats;  
        mrt->suff_seats  = true; 
        std::cout << GREEN << "[TICKET OFFICE] " << g_num_seats << " tickets left" << RESET << std::endl;
    }else{
        mrt->suff_seats  = false; 
    }
}

/******************************************************
 * Function name:    salePoint
 * Date created:     23/4/2020
 * Input arguments:  
 * Purpose:          Create the sale points 
 * 
 ******************************************************/
void salePoint(int id_sale_point){
    std::cout << MAGENTA << "[SALE POINT " << std::to_string(id_sale_point) << "] Created" << RESET << std::endl;

    try{
        while(true){
            g_sem_sale_point.wait(); 
        }

    }catch(std::exception &e){
        std::cout << MAGENTA << "[SALE POINT] An error occurred while attending clients..."<< RESET << std::endl;
    }
}

/******************************************************
 * Function name:    replenisher
 * Date created:     24/4/2020
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

    while(true){
        try{
            g_sem_payment.wait();
        
            /*Control the access to payment request queue*/
            g_sem_mutex_access_payment.lock(); 
                MsgRequestPayment *mrp = g_queue_request_payment.top();  
                g_queue_request_payment.pop();
            g_sem_mutex_access_payment.unlock(); 

            switch(mrp->id_pay){
                case 1:
                    std::cout << BLUE << "[PAYMENT SYSTEM] Payment request received. The client " << std::to_string(mrp->id_client) << " has paid tickets" << RESET << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    break; 
                case 2:
                    std::cout << BLUE << "[PAYMENT SYSTEM] Payment request received. The client " << std::to_string(mrp->id_client) << " has paid drinks and popcorn" << RESET << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    break;
            }
            mrp->attended = true;  
            g_cv_payment.notify_all();
        }catch(std::exception &e){
            std::cout << BLUE << "[PAYMENT SYSTEM] An error occurred while attending clients..." << RESET << std::endl;
            g_cv_payment.notify_all();
        }  
    }
}

/******************************************************
 * Function name:    manager
 * Date created:     13/4/2020
 * Input arguments: 
 * Purpose:          Generate the shifts to the clients access to ticket office 
 * 
 ******************************************************/
void manager(){
    std::cout << CYAN << "[MANAGER] Manager is ready" << RESET << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    try{
        for(int i = 1; i <= NUM_CLIENTS; i++){ 
            std::cout << CYAN << "[MANAGER] It's the shift of client " << std::to_string(i) << RESET << std::endl; 
            g_shift = i; 
            g_cv_ticket_office.notify_all();  
            g_sem_manager.lock(); 
        }
    }catch(std::exception &e){
        std::cout << BOLDCYAN << "[MANAGER] An error occurred while generating shifts..." << RESET << std::endl;
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
    blockSem(); 
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread thread_ticket_office(ticketOffice); 
    std::thread thread_payment(paymentSystem);
    createSalePoints();  
    std::thread thread_clients(createClients);
    std::thread thread_manager(manager); 
    std::thread thread_replenisher(replenisher);  

    thread_payment.join();

    return EXIT_SUCCESS; 
}