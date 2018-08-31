#ifndef __SERVER_H
#define __SERVER_H

int createSocket(int port);
void startListnening(void);
void closeSocket(void);

#endif /* __SERVER_H */
