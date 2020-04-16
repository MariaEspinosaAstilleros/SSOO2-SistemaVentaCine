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
MsgRequestSalePoint::MsgRequestSalePoint(int id, int nd, int np, int id_sp): id_client(id), num_drinks(nd), num_popcorn(np), id_sale_point(id_sp){
    this -> attended = false;  
}

/*Constructor of class of requests to pay*/
MsgRequestPay::MsgRequestPay(int id, int idp): id_client(id), id_pay(idp){
    this -> attended = false;
};  

/*Constructor of class of requests to thread stocker*/
MsgRequestStock::MsgRequestStock(int nd, int np, int id_sp):num_drinks(nd), num_popcorn(np), id_sale_point(id_sp){
    this -> attended = false;
}

