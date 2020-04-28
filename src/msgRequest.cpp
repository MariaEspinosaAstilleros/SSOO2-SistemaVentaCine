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
MsgRequestTickets::MsgRequestTickets(int id, int ns): id_client(id), num_seats(ns){
    this -> suff_seats = false; 
} 

/*Constructor of class of requests to sale point*/
MsgRequestSalePoint::MsgRequestSalePoint(int id, int nd, int np): id(id), num_drinks(nd), num_popcorn(np){
    this -> id_sp_attend = 0; 
    this -> attended     = false;
}

/*Constructor of class of requests to pay*/
MsgRequestPayment::MsgRequestPayment(int id, int t): id_client(id), type(t){
    this -> attended = false;
};  

