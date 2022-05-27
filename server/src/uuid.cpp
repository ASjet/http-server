#include <string>
#include "uuid.h"

#define UUID_STRING_LENGTH 36

#if defined(__linux__) || defined(__unix__)

#include <uuid/uuid.h>
////////////////////////////////////////////////////////////////////////////////
std::string genUUID(void)
{
    char str[UUID_STRING_LENGTH];
    uuid_t uuid;
    uuid_generate_random(uuid);
    uuid_unparse(uuid, str);
    return std::string(str, UUID_STRING_LENGTH);
}


#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <cstdio>
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")
////////////////////////////////////////////////////////////////////////////////
std::string genUUID(void)
{
    UUID uuid;
    std::wstring ws;
    RPC_STATUS ret_val = ::UuidCreate(&uuid);

    if (ret_val == RPC_S_OK)
    {
        // convert UUID to LPWSTR
        WCHAR* wszUuid = NULL;
        ::UuidToStringW(&uuid, (RPC_WSTR*)&wszUuid);
        if (wszUuid != NULL)
        {
            //TODO: do something with wszUuid
            ws = std::wstring(wszUuid);

            // free up the allocated string
            ::RpcStringFreeW((RPC_WSTR*)&wszUuid);
            wszUuid = NULL;
            return std::string(ws.begin(), ws.end());
        }
        else
        {
            //TODO: uh oh, couldn't convert the GUID to string (a result of not enough free memory)
            fprintf(stderr, "failed to allocate memory for generating UUID\n");
        }
    }
    else
    {
        //TODO: uh oh, couldn't create the GUID, handle this however you need to
        fprintf(stderr, "Could not generate UUID\n");
    }
    return std::string("");
}


#else
#error "Unknown compiler"
#endif