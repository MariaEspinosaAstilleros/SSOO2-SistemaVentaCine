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

        MsgRequestTickets(int id, int ns);
};


/******************************************************
 * Class name:       MsgRequestTickets
 * Date created:     3/4/2020
 * Input arguments: 
 * Purpose:          Class of requests to sale point
 *                   The client indicates number of drinks and popcorn that wants.  
 * 
 ******************************************************/
class MsgRequestSalePoint{
    public:
        int     id;
        int     num_drinks;
        int     num_popcorn;
        MsgRequestSalePoint(int id, int nd, int np); 
};


/******************************************************
 * Class name:       MsgRequestPay
 * Date created:     3/4/2020
 * Input arguments: 
 * Purpose:          Class of requests to pay
 *                     
 ******************************************************/
class MsgRequestPayment{
    public:
        int  id_client;
        int  type; 
        bool attended; 

        MsgRequestPayment(int id, int t); 
};

