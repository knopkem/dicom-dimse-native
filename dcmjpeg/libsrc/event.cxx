

#include "opj_includes.h"

/* ==========================================================
     Utility functions
   ==========================================================*/

#if !defined(_MSC_VER) && !defined(__MINGW32__)
static char*
i2a(unsigned i, char *a, unsigned r) {
	if (i/r > 0) a = i2a(i/r,a,r);
	*a = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i%r];
	return a+1;
}

/** 
 Transforms integer i into an ascii string and stores the result in a; 
 string is encoded in the base indicated by r.
 @param i Number to be converted
 @param a String result
 @param r Base of value; must be in the range 2 - 36
 @return Returns a
*/
static char *
_itoa(int i, char *a, int r) {
	r = ((r < 2) || (r > 36)) ? 10 : r;
	if(i < 0) {
		*a = '-';
		*i2a(-i, a+1, r) = 0;
	}
	else *i2a(i, a, r) = 0;
	return a;
}

#endif /* !WIN32 */

/* ----------------------------------------------------------------------- */

opj_event_mgr_t*  opj_set_event_mgr(opj_common_ptr cinfo, opj_event_mgr_t *event_mgr, void *context) {
	if(cinfo) {
		opj_event_mgr_t *previous = cinfo->event_mgr;
		cinfo->event_mgr = event_mgr;
		cinfo->client_data = context;
		return previous;
	}

	return NULL;
}

int opj_event_msg(opj_common_ptr cinfo, int event_type, const char *fmt, ...) {
#define MSG_SIZE 512 /* 512 bytes should be more than enough for a short message */
	opj_msg_callback msg_handler = NULL;

	opj_event_mgr_t *event_mgr = cinfo->event_mgr;
	if(event_mgr != NULL) {
		switch(event_type) {
			case EVT_ERROR:
				msg_handler = event_mgr->error_handler;
				break;
			case EVT_WARNING:
				msg_handler = event_mgr->warning_handler;
				break;
			case EVT_INFO:
				msg_handler = event_mgr->info_handler;
				break;
			default:
				break;
		}
		if(msg_handler == NULL) {
			return 0;
		}
	} else {
		return 0;
	}

	if ((fmt != NULL) && (event_mgr != NULL)) {
		va_list arg;
		int str_length/*, i, j*/; /* UniPG */
		char message[MSG_SIZE];
		memset(message, 0, MSG_SIZE);
		/* initialize the optional parameter list */
		va_start(arg, fmt);
		/* check the length of the format string */
		str_length = (strlen(fmt) > MSG_SIZE) ? MSG_SIZE : strlen(fmt);
		/* parse the format string and put the result in 'message' */
		vsprintf(message, fmt, arg); /* UniPG */
		/* deinitialize the optional parameter list */
		va_end(arg);

		/* output the message to the user program */
		msg_handler(message, cinfo->client_data);
	}

	return 1;
}

