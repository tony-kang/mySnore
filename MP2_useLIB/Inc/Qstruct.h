
#ifndef QBuffSTRUCTDEF
#define QBuffSTRUCTDEF
#define QBUFF_DATA_MAX_SIZE  256*2
#define QBUFF_MAX_CHANNEL 10

typedef struct{
  unsigned char out;
  unsigned char in;
  unsigned char buf[QBUFF_MAX_CHANNEL][QBUFF_DATA_MAX_SIZE];
  unsigned int bufDataSize[QBUFF_MAX_CHANNEL];
}QDef;
int inputDataToQDef(unsigned char *data,int size,QDef *Queue);
int outputDataFromQDef(unsigned char *data,QDef *Queue);
int CheckDataCntInQ(QDef *Queue);
#endif

