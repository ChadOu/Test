
#include <stdint.h>
#include <string.h>
#include "bd_data_manager.h"
#include "bd_uart_debug.h"
#include "bd_sensor_recive.h"
#include "bd_system_state_machine.h"
#include "nrf_soc.h"
#include "pstorage.h"
#include "app_scheduler.h"
#include "bd_twi_master.h"

//SYSTEM_TypeDef gSystem __attribute__((section("NoInit")));
static uint8_t pstorage_wait_flag = 0;
static pstorage_block_t pstorage_wait_handle = 0;
pstorage_handle_t	 system_data_save_handle;

/*
 *  Function: WriteSensorData;
 *  Parameters:1~22
 *  Returns: 
 *  Description:
 */
static void pstorage_user_cb_handler(pstorage_handle_t  * handle,
										uint8_t              op_code,
										uint32_t             result,
										uint8_t            * p_data,
										uint32_t             data_len)
{
	switch(op_code)
	{
		case PSTORAGE_LOAD_OP_CODE:
			 
		case PSTORAGE_STORE_OP_CODE:
			 				 
		case PSTORAGE_UPDATE_OP_CODE:
			 
		case PSTORAGE_CLEAR_OP_CODE:
		if(handle->block_id == pstorage_wait_handle)
		{
			pstorage_wait_flag = 0; 
		}  //If we are waiting for this callback, clear the wait flag.
		break;
		default:break;		 
	}			
}
/*
 *  Function: WriteSensorData; 
 *  Parameters:1~22
 *  Returns: 
 *  Description:
 */ 
void user_data_update(DATA_SAVE_TYPE *op_data)
{
	uint32_t err_code;
	pstorage_wait_handle = system_data_save_handle.block_id;            //Specify which pstorage handle to wait for
	pstorage_wait_flag = 1;                                    //Set the wait flag. Cleared in the example_cb_handler
	err_code=pstorage_update(&system_data_save_handle, op_data->arry,op_data->op_size,op_data->op_offset);                       //Clear 32 bytes
	while(pstorage_wait_flag)
	{
		app_sched_execute();
		sd_app_evt_wait();
	}              
	//Sleep until store operation is finished.
}
/*
 *  Function: WriteSensorData;
 *  Parameters:1~22
 *  Returns: 
 *  Description:
 */ 
void pstorage_user_data_init(void)
{
	uint32_t                err_code;	
	pstorage_handle_t       handle;	
	pstorage_module_param_t param;

	param.block_size  = FLASH_OP_MAX; 
	param.block_count = 1; //Select 1 blocks=1024 bytes;
	param.cb          = pstorage_user_cb_handler;  //Set the pstorage callback handler			
	err_code = pstorage_register(&param, &handle);
	if (err_code != NRF_SUCCESS)
	{
		while(1);//debug;
	}			
	err_code=pstorage_block_identifier_get(&handle, 0, &system_data_save_handle);//Get block identifiers
	if (err_code != NRF_SUCCESS)
	{
		while(1);//debug;
	}
}

void read_flash_data_out(DATA_SAVE_TYPE *op_data)
{
	uint32_t err_code;
	err_code=pstorage_load(op_data->arry, &system_data_save_handle, op_data->op_size, op_data->op_offset);
	if (err_code != NRF_SUCCESS)
	{
		while(1);//debug;
	}
}

void clear_user_data(void)
{
	pstorage_wait_handle = system_data_save_handle.block_id;            //Specify which pstorage handle to wait for
	pstorage_wait_flag = 1;                                    //Set the wait flag. Cleared in the example_cb_handler
	pstorage_clear(&system_data_save_handle, FLASH_OP_MAX);                       //Clear 32 bytes
	while(pstorage_wait_flag) 
	{
		app_sched_execute();
		sd_app_evt_wait();
	}              //Sleep until store operation is finished.
}

void user_data_store(DATA_SAVE_TYPE *op_data)
{
	pstorage_wait_handle = system_data_save_handle.block_id;            //Specify which pstorage handle to wait for
	pstorage_wait_flag = 1;                                    //Set the wait flag. Cleared in the example_cb_handler
	pstorage_store(&system_data_save_handle,op_data->arry,op_data->op_size,op_data->op_offset);     //Write to flash
	while(pstorage_wait_flag) 
	{ 
		app_sched_execute(); 
		sd_app_evt_wait(); 
	} 
}


