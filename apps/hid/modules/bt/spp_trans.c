
#include "app_config.h"
#include "app_action.h"

#include "system/includes.h"
#include "spp_user.h"
#include "string.h"
#include "circular_buf.h"
#include "spp_trans.h"
#include "bt_common.h"
#include "btstack/avctp_user.h"
#include "app_comm_bt.h"
#ifdef LITEEMF_ENABLED
#include "api/bt/api_bt.h"
#endif

#define LOG_TAG_CONST       SPP_TRANS
#define LOG_TAG             "[SPP_TRNS]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define TEST_SPP_DATA_RATE        0

#if TCFG_USER_EDR_ENABLE && USER_SUPPORT_PROFILE_SPP
#if (!CONFIG_APP_AT_COM)

#if TEST_SPP_DATA_RATE
#define SPP_TIMER_MS            (100)
static u32 test_data_count;
static u32 spp_timer_handle = 0;
static u32 spp_test_start;
#endif

/*
 打开流控使能后,确定配置接口 transport_spp_flow_cfg 被调用初始化配置
 然后使用过程 通过接口 transport_spp_flow_enable 来控制流控开关
 */
#define SPP_DATA_RECIEVT_FLOW              0//流控功能使能

//流控速度控制，可以不用更改
#define FLOW_SEND_CREDITS_NUM              1 //控制命令中 控制可接收的数据包个数,发送给对方的,range(1~32)
#define FLOW_SEND_CREDITS_TRIGGER_NUM      1 //触发更新控制命令的阈值,range(1 to <= FLOW_SEND_CREDITS_NUM)


void rfcomm_change_credits_setting(u16 init_credits, u8 base);
int rfcomm_send_cretits_by_profile(u16 rfcomm_cid, u16 credit, u8 auto_flag);
extern void uart_db_regiest_recieve_callback(void *rx_cb);

static struct spp_operation_t *spp_api = NULL;
static u8 spp_state;
static u16 spp_channel;

int transport_spp_send_data(u8 *data, u16 len)
{
    if (spp_api) {
        // log_info("spp_api_tx(%d) \n", len);
        /* log_info_hexdump(data, len); */
        /* clear_sniff_cnt(); */
        bt_comm_edr_sniff_clean();
        return spp_api->send_data(NULL, data, len);
    }
    return SPP_USER_ERR_SEND_FAIL;
}

int transport_spp_send_data_check(u16 len)
{
    if (spp_api) {
        if (spp_api->busy_state()) {
            return 0;
        }
    }
    return 1;
}

static void transport_uart_rx_to_spp(u8 *packet, u32 size)
{
    if (SPP_USER_ST_CONNECT == spp_state && transport_spp_send_data_check(size)) {
        transport_spp_send_data(packet, size);
    } else {
        log_info("drop uart data!!!\n");
    }
}

static void transport_spp_state_cbk(u8 state)
{
    spp_state = state;
    switch (state) {
    case SPP_USER_ST_CONNECT:
        log_info("SPP_USER_ST_CONNECT ~~~\n");
#if TCFG_UART0_RX_PORT != NO_CONFIG_PORT
        //for test 串口数据直通到蓝牙
        uart_db_regiest_recieve_callback(transport_uart_rx_to_spp);
#endif
        #ifdef LITEEMF_ENABLED
        {
            bt_evt_ready_t evt;
            evt.bts = BT_UART;
            evt.ready = true;
            api_bt_event(BT_ID0,BT_EDR,BT_EVT_READY,&evt);
        }
        #endif

        break;

    case SPP_USER_ST_DISCONN:
        log_info("SPP_USER_ST_DISCONN ~~~\n");
        spp_channel = 0;

        #ifdef LITEEMF_ENABLED
        {
            bt_evt_ready_t evt;
            evt.bts = BT_UART;
            evt.ready = false;
            api_bt_event(BT_ID0,BT_EDR,BT_EVT_READY,&evt);
        }
        #endif
        break;

    default:
        break;
    }

}

static void transport_spp_send_wakeup(void)
{
    #ifdef LITEEMF_ENABLED
    bt_evt_tx_t evt;
    evt.bts = BT_UART;
    evt.ret = true;
    api_bt_event(BT_ID0,BT_EDR,BT_EVT_TX,&evt);
    #endif
    // putchar('W');
}

static void transport_spp_recieve_cbk(void *priv, u8 *buf, u16 len)
{
    spp_channel = (u16)priv;
    log_info("spp_api_rx(%d) \n", len);
    log_info_hexdump(buf, len);
    /* clear_sniff_cnt(); */
    bt_comm_edr_sniff_clean();

#if TEST_SPP_DATA_RATE
    if ((buf[0] == 'A') && (buf[1] == 'F')) {
        spp_test_start = 1;//start
    } else if ((buf[0] == 'A') && (buf[1] == 'A')) {
        spp_test_start = 0;//stop
    }
#endif

    //loop send data for test
    if (transport_spp_send_data_check(len)) {
        log_info("-loop send\n");
        transport_spp_send_data(buf, len);
    }
}


#if TEST_SPP_DATA_RATE

static void test_spp_send_data(void)
{
    u16 send_len = 250;
    if (transport_spp_send_data_check(send_len)) {
        test_data_count += send_len;
        transport_spp_send_data(&test_data_count, send_len);
    }
}

static void test_timer_handler(void)
{
    static u32 t_count = 0;

    if (SPP_USER_ST_CONNECT != spp_state) {
        test_data_count = 0;
        spp_test_start = 0;
        return;
    }

    if (spp_test_start) {
        test_spp_send_data();
    }

    if (++t_count < (1000 / SPP_TIMER_MS)) {
        return;
    }
    t_count = 0;

    if (test_data_count) {
        log_info("\n-spp_data_rate: %d bps-\n", test_data_count * 8);
        test_data_count = 0;
    }
}
#endif


void transport_spp_init(void)
{
    log_info("trans_spp_init\n");
    log_info("spp_file: %s", __FILE__);
#if (USER_SUPPORT_PROFILE_SPP==1)
    spp_state = 0;
    spp_get_operation_table(&spp_api);
    spp_api->regist_recieve_cbk(0, transport_spp_recieve_cbk);
    spp_api->regist_state_cbk(0, transport_spp_state_cbk);
    spp_api->regist_wakeup_send(NULL, transport_spp_send_wakeup);
#endif

#if TEST_SPP_DATA_RATE
    spp_timer_handle  = sys_timer_add(NULL, test_timer_handler, SPP_TIMER_MS);
#endif

}

void transport_spp_disconnect(void)
{
    if (SPP_USER_ST_CONNECT == spp_state) {
        log_info("trans_spp_disconnect\n");
        user_send_cmd_prepare(USER_CTRL_SPP_DISCONNECT, 0, NULL);
    }
}

static void timer_spp_flow_test(void)
{
    static u8 sw = 0;
    if (spp_channel) {
        sw = !sw;

        log_info("test_flow_switch: %d\n", sw);
        transport_spp_flow_enable(sw);
    }
}

//配置流控制的参数，蓝牙协议栈初始化前调用配置
void transport_spp_flow_cfg(void)
{
#if SPP_DATA_RECIEVT_FLOW
    rfcomm_change_credits_setting(FLOW_SEND_CREDITS_NUM, FLOW_SEND_CREDITS_TRIGGER_NUM);

    //for test,timer to controller
    /* sys_timer_add(0,timer_spp_flow_test,3000); */
#endif
}

//流控使能 EN: 1-停止收数 or 0-继续收数
int transport_spp_flow_enable(u8 en)
{
    int ret = -1;
    u16 credits = FLOW_SEND_CREDITS_NUM;

#if SPP_DATA_RECIEVT_FLOW
    if (spp_channel) {
        if (en) {
            credits = 0;
        }
        ret = rfcomm_send_cretits_by_profile(spp_channel, credits, !en);
    }
#endif

    log_info("trans_spp_flow_enable:%02x,%d,%d\n", spp_channel, en, ret);
    return ret;
}

#endif
#endif

