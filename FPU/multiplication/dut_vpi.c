#include <stdlib.h> /* ANSI C standard library */
#include <stdio.h> /* ANSI C standard input/output library */
#include "vpi_user.h" /* IEEE 1364 PLI VPI routine library */
#include "veriuser.h" /* IEEE 1364 PLI TF routine library
						(using TF routines for simulation control) */
#include "acc_user.h"
#include "sv_vpi_user.h"
#include "ivl_target.h"
#include "_pli_types.h"
						
/* prototypes of routines in this PLI application */
int dut_calltf(char*), dut_compiletf(char*);
int dut_interface();

/**********************************************************************
* VPI Registration Data
*********************************************************************/
void dut_register()
{
s_vpi_systf_data tf_data;
tf_data.type = vpiSysTask;
tf_data.tfname = "$dut_c";
tf_data.calltf = dut_calltf;
tf_data.compiletf = dut_compiletf;
tf_data.sizetf = NULL;

tf_data.user_data = NULL;

vpi_register_systf(&tf_data);

}


void (*vlog_startup_routines[]) () =
{
	/*** add user entries here ***/
dut_register,
0 /*** final entry must be 0 ***/
} ;



/**********************************************************************
* Definition for a structure to hold the data to be passed from
* calltf application to the dut_c interface.
*********************************************************************/
typedef struct dut_data {
vpiHandle a_h, b_h, c_h;
} dut_data_s, *dut_data_p;

/**********************************************************************
* Value change simulation callback routine: Serves as an interface
* between Verilog simulation and the C model. Called whenever the
* C model inputs change value, passes the values to the C model, and
* puts the C model outputs into simulation.
*
* NOTE: The handles for the arguments to $dut_c were obtained
* in the calltf routine and saved in application-allocated memory. A
* pointer to this memory is passed to this callback via the user_data
* field.
*********************************************************************/
int dut_interface(p_cb_data cb_data)
{
double a, b, c;


s_vpi_value value_s;
dut_data_p dut_data;

/* Retrieve pointer to dut_c data structure from callback user data. */
/* The structure contains the handles for the $dut_c args */

dut_data = (dut_data_p)cb_data->user_data; 

/* Read current values of C model inputs from Verilog simulation */
value_s.format = vpiRealVal;
vpi_get_value(dut_data->a_h, &value_s);
a = value_s.value.real;

vpi_get_value(dut_data->b_h, &value_s);
b = value_s.value.real;

c = a*b;


/* Write the C model outputs onto the Verilog signals */
value_s.format = vpiRealVal;
value_s.value.real = c;
vpi_put_value(dut_data->c_h, &value_s, NULL, vpiNoDelay);

return (0);

}


/**********************************************************************
* compiletf routine: Verifies that $dut_c() is used correctly
* Note: For simplicity, only limited data types are allowed for
task arguments. Could add checks to allow other data types.
*********************************************************************/
int dut_compiletf(char* user_data)
{
	vpiHandle instance_h, arg_iter, arg_h;
	int err = 0;
	instance_h = vpi_handle(vpiSysTfCall, NULL);
	arg_iter = vpi_iterate(vpiArgument, instance_h);
	
	if (arg_iter == NULL) {
		vpi_printf("ERROR: $dut_c requires 3 arguments\n");
		tf_dofinish () ;
		return(0) ;
	}
	arg_h = vpi_scan(arg_iter); /* 1st arg is a input */
	if (vpi_get(vpiType, arg_h) != vpiRealVar) {
		vpi_printf("$dut_c arg 1 (a) must be a real variable\n");
		err = 1;
	}
	arg_h = vpi_scan(arg_iter); /* 1st arg is a input */
	if (vpi_get(vpiType, arg_h) != vpiRealVar) {
		vpi_printf("$dut_c arg 2 (b) must be a real variable\n");
		err = 1;
	}
	arg_h = vpi_scan(arg_iter); /* 1st arg is a input */
	if (vpi_get(vpiType, arg_h) != vpiRealVar) {
		vpi_printf("$dut_c arg 3 (c) must be a real variable\n");
		err = 1;
	}
	
	if(vpi_scan(arg_iter)) {/* More than 3 arguments*/
		vpi_printf("$dut_c arguments are more than 3!");
		err =1;
	}
	vpi_free_object(arg_iter);
	
	if(err) 
		tf_dofinish () ;
		
	return(0) ;
}
	

/**********************************************************************
* calltf routine: Registers a callback to the C model interface
* whenever any input to the C model changes value
*********************************************************************/
int dut_calltf(char *user_data)
{
	vpiHandle instance_h, arg_itr;
	s_vpi_value value_s;
	s_vpi_time time_s;
	s_cb_data cb_data_s;

	dut_data_p  dut_data;

	/* allocate storage to hold $dut_c argument handles */
	dut_data = (dut_data_p)malloc(sizeof(dut_data_s));

	/* obtain a handle to the system task instance */
	instance_h = vpi_handle (vpiSysTfCall , NULL);

	/* obtain handles to system task arguments */
	/* compiletf has already verified arguments are correct */
	arg_itr = vpi_iterate(vpiArgument, instance_h);
	dut_data->a_h = vpi_scan(arg_itr); /* 1st arg is a input */
	dut_data->b_h = vpi_scan(arg_itr); /* 2nd arg is b input */
	dut_data->c_h = vpi_scan(arg_itr); /* 3rd arg is c output */

	vpi_free_object(arg_itr); /* free iterator--did not scan to null */

	time_s.type = vpiSuppressTime;
	cb_data_s.reason = cbValueChange;
	cb_data_s.cb_rtn = dut_interface;
	cb_data_s.time = &time_s;
	cb_data_s.value = &value_s;

	/* add value change callbacks to all signals which are inputs to */
	/* pass pointer to storage for handles as user_data value */
	cb_data_s.user_data = (char *)dut_data;
	value_s.format = vpiRealVal;
	cb_data_s.obj = dut_data->a_h;
	vpi_register_cb(&cb_data_s) ;
	cb_data_s.obj = dut_data->b_h;
	vpi_register_cb(&cb_data_s);

	return 0;

}


