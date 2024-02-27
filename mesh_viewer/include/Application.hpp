//base class for applications, anything that updates continuously and accepts user input.
//will run until shouldExit returns true
//virtual functions are called in this order in run loop:
//processInput()
//updateState()

#ifndef APPLICATIONHPP
#define APPLICATIONHPP

class ApplicationBase{
public:
    ApplicationBase(){};

    virtual ~ApplicationBase(){};

    void run();
    
protected:
    virtual bool shouldExit()=0;

    virtual void updateState()=0;

    //process user input here
    virtual void processInput()=0;
};

#endif