#ifndef TEST_COMMAND_H
#define TEST_COMMAND_H

using namespace NS_SERVER;

bool Query(int fd,U16 port);

bool BatchQuery(const CSockAddr & servAddr);

#endif

//Test message!!!