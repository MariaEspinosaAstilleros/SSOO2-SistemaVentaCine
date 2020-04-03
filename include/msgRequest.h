/******************************************************
 * Project:         Práctica 3 de Sistemas Operativos II
 * 
 * Program name:    msgRequest.h
 
 * Author:          María Espinosa Astilleros
 * 
 * Date created:    2/4/2020
 * 
 * Purpose:         Contain the definitions of the class for the requests
 * 
 ******************************************************/

#include <iostream>

/******************************************************
 * Class name:       MsgRequestTickets
 * Date created:     3/4/2020
 * Input arguments: 
 * Purpose:          Class of requests to ticket office 
 *                   The client indicates id of the client and number of seats that wants. The ticket office 
 *                   show if it has seats sufficient
 * 
 ******************************************************/
class MsgRequestTickets{
    public:
        int     id_client;
        int     num_seats;
        bool    suff_seats;

        MsgRequestTickets(int id_client, int num_seats, bool suff_seats);
};


/******************************************************
 * Class name:       MsgRequestTickets
 * Date created:     3/4/2020
 * Input arguments: 
 * Purpose:          Class of requests to sale point
 *                   The client indicates number of drinks and popcorns that wants.  
 * 
 ******************************************************/
class MsgRequestSalePoint{
    public:
        int     id_client;
        int     num_drinks;
        int     num_popcorns;
        int     id_sale_point; 
        bool    attended; 

        MsgRequestSalePoint(int id_client, int num_drinks, int num_popcorns, int id_sale_point, bool attended); 
};


/******************************************************
 * Class name:       MsgRequestPay
 * Date created:     3/4/2020
 * Input arguments: 
 * Purpose:          Class of requests to pay
 *                     
 ******************************************************/
class MsgRequestPay{
    public:
        int id_client; 

        MsgRequestPay(int id_client); 
};


/******************************************************
 * Class name:       MsgRequestStock
 * Date created:     3/4/2020
 * Input arguments: 
 * Purpose:          Class of requests to thread stocker
 *                     
 ******************************************************/
class MsgRequestStock{
    public:
        int     num_drinks;
        int     num_popcorns;
        int     id_sale_point; 
        bool    attended;
        
        MsgRequestStock(int num_drinks, int num_popcorns, int id_sale_point, bool attended);    
};