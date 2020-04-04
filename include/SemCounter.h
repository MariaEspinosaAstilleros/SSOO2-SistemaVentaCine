/******************************************************
 * Project:         Práctica 3 de Sistemas Operativos II
 * 
 * Program name:    SemCounter.h
 
 * Author:          María Espinosa Astilleros
 * 
 * Date created:    4/4/2020
 * 
 * Purpose:         Contain the definitions of counter semaphore
 * 
 ******************************************************/
#include <iostream>
#include <thread>
#include<mutex>

class SemCounter{
    private:
        int value;
        std::mutex mutex_; 
        std::mutex mutex_block; 

        void block();
        void unblock(); 

    public:
        SemCounter(int value); 
        void wait();
        void signal(); 
        int getValue(); 
}; 