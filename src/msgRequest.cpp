/******************************************************
 * Project:         Práctica 3 de Sistemas Operativos II
 * 
 * Program name:    msgRequest.cpp
 
 * Author:          María Espinosa Astilleros
 * 
 * Date created:    3/4/2020
 * 
 * Purpose:         Contain the implementation of the class for the requests
 * 
 ******************************************************/

#include <iostream> 
#include "../include/msgRequest.h"

/*Constructor of class of requests to tickets*/
MsgRequestTickets::MsgRequestTickets(int id_client, int num_seats, bool suff_seats){
    this -> id_client  = id_client; 
    this -> num_seats  = num_seats; 
    this -> suff_seats = false; 

} 

/*Constructor of class of requests to sale point*/
MsgRequestSalePoint::MsgRequestSalePoint(int id_client, int num_drinks, int num_popcorns, int id_sale_point, bool attended){
    this -> id_client       = id_client;
    this -> num_drinks      = num_drinks;
    this -> num_popcorns    = num_popcorns; 
    this -> id_sale_point   = id_sale_point;
    this -> attended        = false;  
}

/*Constructor of class of requests to pay*/
MsgRequestPay::MsgRequestPay(int id_client){
    this -> id_client = id_client; 
}

/*Constructor of class of requests to thread stocker*/
MsgRequestStock::MsgRequestStock(int num_drinks, int num_popcorns, int id_sale_point, bool attended){
    this -> num_drinks      = num_drinks;
    this -> num_popcorns    = num_popcorns; 
    this -> id_sale_point   = id_sale_point;
    this -> attended        = false;
}