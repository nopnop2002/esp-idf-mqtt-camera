#define CMD_TAKE 100
#define CMD_HALT 400

typedef struct {
    uint16_t command;
    TaskHandle_t taskHandle;
} CMD_t;
