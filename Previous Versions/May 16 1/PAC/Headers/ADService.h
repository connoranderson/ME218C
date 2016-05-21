#ifndef ADService_H_INCLUDED
#define ADService_H_INCLUDED

typedef enum { WaitingForTimeout } ADState_t ;

bool InitializeADService(uint8_t Priority);
bool PostADService(ES_Event ThisEvent);
ES_Event RunADService(ES_Event ThisEvent);

#endif // DIALSERVICE_H_INCLUDED
