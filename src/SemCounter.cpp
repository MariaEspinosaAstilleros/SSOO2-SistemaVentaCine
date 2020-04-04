/******************************************************
 * Project:         Práctica 3 de Sistemas Operativos II
 * 
 * Program name:    msgRequest.cpp
 
 * Author:          María Espinosa Astilleros
 * 
 * Date created:    4/4/2020
 * 
 * Purpose:         Contain the implementation of counter semaphore
 * 
 ******************************************************/
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

#include "../include/SemCounter.h"

/*Constructor*/
SemCounter::SemCounter(int v): value(v){}; 

/*Method block*/
void SemCounter::block(){ mutex_block.lock(); }

/*Method unblock*/
void SemCounter::unblock(){ mutex_block.unlock(); }

/*Method wait*/
void SemCounter::wait(){
    mutex_.lock(); 
    if(--value <=0){
        mutex_.unlock(); 
        block(); 
        mutex_.lock();
    }
    mutex_.unlock(); 
}

/*Method signal*/
void SemCounter::signal(){
    mutex_.lock(); 
    if(++value <= 0){
        unblock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
    }
    mutex_.unlock(); 
}

/*Method getValue*/
int SemCounter::getValue(){ return value; }