#define PIN_I2C_SDA 0
#define PIN_I2C_SCL 2

const float DewConstA = 8.1332;
const float DewConstB = 1762.39;
const float DewConstC = 235.66;

static char MAC_char[18];

static char MQTTClientName[20];

#define STRBUFF_LEN 24
char strbuff[STRBUFF_LEN];

static char MQTTTopicAnnounce[50];
static char MQTTTopicConfig[50];

static char MQTTTopicHumd[50];
static char MQTTTopicTemp[50];
static char MQTTTopicDewp[50];
static char MQTTTopicPPres[50];
static char MQTTTopicSigRH[50];
static char MQTTTopicSigT[50];

#define CONFIG_LEN 32
typedef struct {
  char wifi_ssid[CONFIG_LEN];
  char wifi_pwd[CONFIG_LEN];
  char mqtt_server[CONFIG_LEN];
  char mqtt_topicbase[CONFIG_LEN];
  char ntp_server[CONFIG_LEN];
  char thingspeak_addr[CONFIG_LEN];
  char thingspeak_apiKey[CONFIG_LEN];
  int  sensor_period;
} Config;

Config C = {"WhyFi","test$321","iot.eclipse.org","wxyz","0.ca.pool.ntp.org","api.thingspeak.com","XYZ234",30000};

