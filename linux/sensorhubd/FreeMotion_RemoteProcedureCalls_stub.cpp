#include "FreeMotion_RemoteProcedureCalls.h"

#include <cstring>
#include <cstdlib>

#include "freemotion_rpc_messages.h"



///// Macros
#define LOGE(...) fprintf(stderr, __VA_ARGS__)
#define LOGW(...) fprintf(stderr, __VA_ARGS__)
#define LOGT(...) fprintf(stderr, __VA_ARGS__)
#define LOGI(...) printf(__VA_ARGS__)
#define LOGD(...) printf(__VA_ARGS__)
//#define LOGD(...)
#define LOGS(...) printf(__VA_ARGS__)

///// Private Methods

///// Private Data
static FMRPC_ResultDataCallback_t _resultReadyCallbacks[RESULT_ENUM_COUNT] = {0};

///// Implementations



FM_STATUS_t FMRPC_Initialize(void) {
    FM_STATUS_t result = FM_STATUS_OK;
    LOGT("%s\r\n", __FUNCTION__);

    return result;
}

FM_STATUS_t FMRPC_GetVersion(char* versionString, int bufSize) {
    FM_STATUS_t result = FM_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);

    return result;
}

FM_STATUS_t FMRPC_SubscribeResult(SensorType_t sensorType, FMRPC_ResultDataCallback_t dataReadyCallback ) {
    FM_STATUS_t result = FM_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);

    return result;
}

FM_STATUS_t FMRPC_UnsubscribeResult(SensorType_t sensorType) {
    FM_STATUS_t result = FM_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);

    return result;
}


FM_STATUS_t FMRPC_Deinitialize(void) {
    FM_STATUS_t result = FM_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);

    return result;
}


