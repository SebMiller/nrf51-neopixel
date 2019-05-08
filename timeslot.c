#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "softdevice_handler.h"

/**Constants for timeslot API
*/
static nrf_radio_request_t  m_timeslot_request;
static uint32_t             m_slot_length;
typedef void (*vcb)(void);
static vcb ts_cb;

static nrf_radio_signal_callback_return_param_t signal_callback_return_param;

/**@brief Request next timeslot event in earliest configuration
 */
uint32_t request_next_event_earliest(void)
{
    m_slot_length                                  = 500;
    m_timeslot_request.request_type                = NRF_RADIO_REQ_TYPE_EARLIEST;
    m_timeslot_request.params.earliest.hfclk       = NRF_RADIO_HFCLK_CFG_DEFAULT;
    m_timeslot_request.params.earliest.priority    = NRF_RADIO_PRIORITY_NORMAL;
    m_timeslot_request.params.earliest.length_us   = m_slot_length;
    m_timeslot_request.params.earliest.timeout_us  = 1000000;
    return sd_radio_request(&m_timeslot_request);
}


/**@brief Timeslot signal handler
 */
void nrf_evt_signal_handler(uint32_t evt_id)
{
    uint32_t err_code;
    
    switch (evt_id)
    {
        case NRF_EVT_RADIO_SIGNAL_CALLBACK_INVALID_RETURN:
            //No implementation needed
            break;
        case NRF_EVT_RADIO_SESSION_IDLE:
            //No implementation needed
            break;
        case NRF_EVT_RADIO_SESSION_CLOSED:
            //No implementation needed, session ended
            break;
        case NRF_EVT_RADIO_BLOCKED:
            //Fall through
        case NRF_EVT_RADIO_CANCELED:
            err_code = request_next_event_earliest();
            APP_ERROR_CHECK(err_code);
            break;
        default:
            break;
    }
}


/**@brief Timeslot event handler
 */
nrf_radio_signal_callback_return_param_t * radio_callback(uint8_t signal_type)
{
    switch(signal_type)
    {
        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:
            //Start of the timeslot - set up timer interrupt
            signal_callback_return_param.params.request.p_next = NULL;
            signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE; 
            
            ts_cb();
            break;

        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO:
            signal_callback_return_param.params.request.p_next = NULL;
            signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
            break;

        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0:
            signal_callback_return_param.params.request.p_next = NULL;
            signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
            break;
        default:
            //No implementation needed
            break;
    }
    return (&signal_callback_return_param);
}


/**@brief Function for initializing the timeslot API.
 */
uint32_t timeslot_sd_init(void cb(void))
{
    uint32_t err_code;

    ts_cb = cb;
    
    err_code = sd_radio_session_open(radio_callback);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}
