/*
  tsip.cpp - class implementation for the simplified Trimble Standard
             Interface Protocol (TSIP) library for the Arduino platform.

  Modified 2012-02 by Criss Swaim, The Pineridge Group, LLC
             
  Copyright (c) 2011 N7MG Brett Howard
  Copyright (c) 2011 Andrew Stern (N7UL)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  $Id: Tsip.cpp 31 2011-06-12 17:33:08Z andrew@n7ul.com $ 

*/

#include "tsip.h"

tsip::tsip() {
	init_rpt();
	verbose = true;
	debug = false;
}

/** initilize command/report fields
*
*   This routine initialized the command/report fields for each
*	command request sent to the process.  
*   
*	@return  void
*/
void tsip::init_rpt()
{
	m_report_length = 0;
	m_state = START;
	m_updated.value = 0;
	m_ecef_position_s.valid = false;
	m_ecef_position_d.valid = false;
	m_ecef_velocity.valid = false;
	m_sw_version.valid = false;
	m_single_position.valid = false;
	m_double_position.valid = false;
	m_io_options.valid = false;
	m_enu_velocity.valid = false;
	m_utc_gps_time.valid = false;
	m_primary_time.valid = false;
	m_secondary_time.valid = false;
	m_unknown.valid = false;

	for (int i=0; i<MAX_DATA;i++) {
		m_report.raw.data[i]='\0';
	}
}
/** set verbose flag
*
*   If the verbose flag is false, then all messages from the tsip
* 	routine are suppressed.
* 	The default is true
*
* 	@param   bool  true/false
* 	@return  void
*/
void tsip::set_verbose(bool vb) {
	verbose = vb;
}

/** set debug flag
*
*   If the debug flag is false, then debug messages are suppressed.
* 	The default is false
*
* 	@param   bool  true/false
* 	@return  void
*/
void tsip::set_debug(bool db) {
	debug = db;
}

/** convert 2 bytes to short int
*
*   Conversion routine to accept the bytes from m_command begining
*	with the byte referenced by the passed index.  The formatted value
* 	is returned.
*
* 	***note x86 is a low-endian machine, the trimble is high-endian
*      so the bytes must be flipped
*   
* 	@param   int  the position of the beg byte in m_report
* 	@param   char the code r or e to  idenity which array to pull from
*	@return  UINIT16 
*/
UINT16 tsip::b2_to_uint16(int bb, char r_code) {
	UINT16 x = 0;

	if (r_code == 'r') {
		x = (m_report.report.data[bb] << 8) + m_report.report.data[bb+1];
	} else if (r_code == 'e') {
		x = (m_report.extended.data[bb] << 8) + m_report.extended.data[bb+1];
	}

	return x;
}
	
/** convert 4 bytes to int
*
*   Conversion routine to accept the bytes from m_command begining
*	with the byte referenced by the passed index.  The formatted value
* 	is returned.
*
* 	***note x86 is a low-endian machine, the trimble is high-endian
*      so the bytes must be flipped
*   
* 	@param   int  the position of the beg byte in m_report
* 	@param   char the code r or e to  idenity which array to pull from
*	@return  UINIT32 
*/
UINT32 tsip::b4_to_uint32(int bb, char r_code) {
	UINT32 x = 0;

	if (r_code == 'r') {
		x = (m_report.report.data[bb] << 24) + (m_report.report.data[bb+1] << 16) + (m_report.report.data[bb+2] << 8) + m_report.report.data[bb+3];
	} else if (r_code == 'e') {
		x = (m_report.extended.data[bb] << 24) + (m_report.extended.data[bb+1] << 16) + (m_report.extended.data[bb+2] << 8) + m_report.extended.data[bb+3];
	}
	
	return x;
}
	
/** convert 4 bytes to single
*
*   Conversion routine to accept the bytes from m_command begining
*	with the byte referenced by the passed index.  The formatted value
* 	is returned.
*
* 	***note x86 is a low-endian machine, the trimble is high-endian
*      so the bytes must be flipped
*   
* 	@param   int  the position of the first byte in m_report
* 	@param   char the code r or e to  idenity which array to pull from
*	@return  singel
*/
SINGLE tsip::b4_to_single(int bb, char r_code) {
	union _dbl_t {
		UINT8 data[sizeof(SINGLE)];
		char cdata[sizeof(SINGLE)];
		SINGLE value;
	} sgl;

	//must reverse order of bytes for endian compatability
	if (r_code == 'r') {
		for (int i=0, j=sizeof(SINGLE); i<sizeof(SINGLE); i++, j--) {
			sgl.cdata[j] = m_report.report.data[bb+i];
		}
	} else if (r_code == 'e') {
		for (int i=0, j=sizeof(SINGLE); i<sizeof(SINGLE); i++, j--) {
			sgl.cdata[j] = m_report.extended.data[bb+i];
		}
	}

	return sgl.value;
	
	
}
	 
/** convert 8 bytes to double
*
*   Conversion routine to accept the bytes from m_command begining
*	with the byte referenced by the passed index.  The formatted value
* 	is returned.
*
* 	***note x86 is a low-endian machine, the trimble is high-endian
*      so the bytes must be flipped
*   
* 	@param   int  the position of the first byte in m_report
* 	@param   char the code r or e to  idenity which array to pull from
*	@return  double 
*/
DOUBLE tsip::b8_to_double(int bb, char r_code) {
	union _dbl_t {
		UINT8 data[sizeof(DOUBLE)];
		char cdata[sizeof(DOUBLE)];
		DOUBLE value;
	} dbl;

	//must reverse order of bytes for endian compatability
	if (r_code == 'r') {
		for (int i=0, j=sizeof(DOUBLE)-1; i<sizeof(DOUBLE); i++, j--) {
			dbl.cdata[j] = m_report.report.data[bb+i];
		}
	} else if (r_code == 'e') {
		for (int i=0, j=sizeof(DOUBLE)-1; i<sizeof(DOUBLE); i++, j--) {
			dbl.cdata[j] = m_report.extended.data[bb+i];
		}
	}

	return dbl.value;

}       

/** encode byte stream into TSIP packets
*
*   The TSIP packet encode state machine that collects a stream
*   of bytes into TSIP packets.Encodes byte steam from a serial
*   byte stream from a Trimble Thunderbolt GPSDO into TSIP
*   packets. When a packet has been completed the corresponding
*   report is updated.
*/
int tsip::encode(UINT8 c)
{
	switch (m_state) {
 
	case START:
		// search for start
		if (c == DLE)
			m_state = FRAME;
		break;

	case FRAME:
		// check if mis-framed
		if (c == DLE || c == ETX)
			m_state = START;
		else {
			m_state = DATA;
			m_report_length = 0;
			m_report.raw.data[m_report_length++] = c;
		}
		break;

	case DATA:
		// found data DLE
		if (c == DLE) {
			m_state = DATA_DLE;
		}
		// add byte to report packet
		else if (m_report_length < MAX_DATA) {
			m_report.raw.data[m_report_length++] = c;
		}
		break;

	case DATA_DLE:
		// escaped data 
		if (c == DLE) {
			m_state = DATA;
			if (m_report_length < MAX_DATA) {
				m_report.raw.data[m_report_length++] = c;
			}
		}
		// end of frame
		else if (c == ETX) {
			m_state = START;
			return update_report();    //Whoohoo the moment we've been waitin for
		}
		// mis-framed
		else
			m_state = START;
			if (verbose) printf("waiting gps packet......\n");
		break;

	default:
		m_state = START;
		if (verbose) printf("waiting gps packet......\n");
		break;
	}

	return 0;
}


/** update received report
*
*   Update received report's property buffer.
*
*   @return m_updated to indicate which report was received.
*/
int tsip::update_report()
{
	void *src;
	void *dst;
	int  rlen = 0;

	if (verbose) printf("Found Report: %x-%x\n",m_report.report.code,m_report.extended.subcode);
	// save report
	switch (m_report.report.code) {

	case REPORT_ECEF_POSITION_S:
		m_updated.report.ecef_position_s = 1;
		m_ecef_position_s.valid = true;
		rlen = sizeof(m_ecef_position_s.report);

		m_ecef_position_s.report.x = (m_report.extended.data[0] << 24) + (m_report.extended.data[1] << 16) + (m_report.extended.data[2] << 8) + m_report.extended.data[3];
		m_ecef_position_s.report.y = (m_report.extended.data[4] << 24) + (m_report.extended.data[5] << 16) + (m_report.extended.data[6] << 8) + m_report.extended.data[7];
		m_ecef_position_s.report.z = (m_report.extended.data[8] << 24) + (m_report.extended.data[9] << 16) + (m_report.extended.data[10] << 8) + m_report.extended.data[11];
		m_ecef_position_s.report.time_of_fix = (m_report.extended.data[12] << 24) + (m_report.extended.data[13] << 16) + (m_report.extended.data[14] << 8) + m_report.extended.data[15];
	
		break;

	case REPORT_ECEF_POSITION_D:
		m_updated.report.ecef_position_d = 1;
		m_ecef_position_d.valid = true;
		src = m_report.report.data;
		dst = &m_ecef_position_d.report;
		rlen = sizeof(m_ecef_position_d.report);
		break;

	case REPORT_ECEF_VELOCITY:
		m_updated.report.ecef_velocity = 1;
		m_ecef_velocity.valid = true;
		src = m_report.report.data;
		dst = &m_ecef_velocity.report;
		rlen = sizeof(m_ecef_velocity.report);
		break;

	case REPORT_SW_VERSION:
		m_updated.report.sw_version = 1;
		m_sw_version.valid = true;
		src = m_report.report.data;
		dst = &m_sw_version.report;
		rlen = sizeof(m_sw_version.report);
		break;

	case REPORT_SINGLE_POSITION:
		m_updated.report.single_position = 1;
		m_single_position.valid = true;
		src = m_report.report.data;
		dst = &m_single_position.report;
		rlen = sizeof(m_single_position.report);
		break;

	case REPORT_DOUBLE_POSITION:
		m_updated.report.double_position = 1;
		m_double_position.valid = true;
		src = m_report.report.data;
		dst = &m_double_position.report;
		rlen = sizeof(m_single_position.report);
		break;

	case REPORT_IO_OPTIONS:
		m_updated.report.io_options = 1;
		m_io_options.valid = true;
		src = m_report.report.data;
		dst = &m_io_options.report;
		rlen = sizeof(m_io_options.report);
		break;

	case REPORT_ENU_VELOCITY:
		m_updated.report.enu_velocity = 1;
		m_enu_velocity.valid = true;
		src = m_report.report.data;
		dst = &m_enu_velocity.report;
		rlen = sizeof(m_enu_velocity.report);
		break;

	case REPORT_SUPER:
		switch (m_report.extended.subcode) {

		// 8f-a2
		case REPORT_SUPER_UTC_GPS_TIME:
			m_updated.report.utc_gps_time = 1;
			m_utc_gps_time.valid = true;
			rlen = sizeof(m_utc_gps_time.report);
			
			m_utc_gps_time.report.bits.value = m_report.extended.data[0];
			break;

		// 8f-ab
		case REPORT_SUPER_PRIMARY_TIME:
			m_updated.report.primary_time = 1;
			m_primary_time.valid = true;
			rlen = sizeof(m_primary_time.report);

			m_primary_time.report.seconds_of_week = b4_to_uint32(0,'e');
			m_primary_time.report.week_number = b2_to_uint16(4,'e');
			m_primary_time.report.utc_offset = b2_to_uint16(6,'e');
			m_primary_time.report.flags.value = m_report.extended.data[8];
			m_primary_time.report.seconds = m_report.extended.data[9];
			m_primary_time.report.minutes = m_report.extended.data[10];
			m_primary_time.report.hours = m_report.extended.data[11];
			m_primary_time.report.day = m_report.extended.data[12];
			m_primary_time.report.month = m_report.extended.data[13];
			m_primary_time.report.year = b2_to_uint16(14,'e');

			break;

		// 8f-ac
		case REPORT_SUPER_SECONDARY_TIME:
			m_updated.report.secondary_time = 1;
			m_secondary_time.valid = true;
			rlen = sizeof(m_secondary_time.report);

			m_secondary_time.report.receiver_mode = m_report.extended.data[0];
			m_secondary_time.report.disciplining_mode = m_report.extended.data[1];
			m_secondary_time.report.self_survey_progress = m_report.extended.data[2];
			m_secondary_time.report.holdover_duration = b4_to_uint32(3,'e');
			m_secondary_time.report.critical_alarms.value = b2_to_uint16(7,'e');
			m_secondary_time.report.minor_alarms.value = b2_to_uint16(9,'e');;
			m_secondary_time.report.gps_decoding_status = m_report.extended.data[11];
			m_secondary_time.report.disciplining_activity = m_report.extended.data[12];
			m_secondary_time.report.spare_status1 = m_report.extended.data[13];
			m_secondary_time.report.spare_status2 = m_report.extended.data[14];
			
			m_secondary_time.report.pps_offset = b4_to_single(15,'e');;

			m_secondary_time.report.tenMHz_offset = b4_to_single(19,'e');

			m_secondary_time.report.dac_value = b4_to_uint32(23,'e');

			m_secondary_time.report.dac_voltage = b4_to_single(27,'e');

			m_secondary_time.report.temperature = b4_to_single(31,'e');
			m_secondary_time.report.latitude = b8_to_double(35,'e');
			
			m_secondary_time.report.longitude = b8_to_double(43,'e');

			m_secondary_time.report.altitude = b8_to_double(51,'e');
		
			m_secondary_time.report.spare[0] = m_report.extended.data[59];
			m_secondary_time.report.spare[1] = m_report.extended.data[60];
			m_secondary_time.report.spare[2] = m_report.extended.data[61];
			m_secondary_time.report.spare[3] = m_report.extended.data[62];
			m_secondary_time.report.spare[4] = m_report.extended.data[63];
			m_secondary_time.report.spare[5] = m_report.extended.data[64];
			m_secondary_time.report.spare[6] = m_report.extended.data[65];
			m_secondary_time.report.spare[7] = m_report.extended.data[66];
			break;

		default:
			m_updated.report.unknown = 1;
			m_unknown.valid = true;
		    src = m_report.raw.data;
			dst = m_unknown.report.raw.data;
			rlen = sizeof(m_unknown.report.raw.data);
			break;
		}

	default:
		m_updated.report.unknown = 1;
		m_unknown.valid = true;
	    src = m_report.raw.data;
		dst = m_unknown.report.raw.data;
		rlen = sizeof(m_unknown.report.raw.data);
        break;
	}
	
	// report strucute updated
	if (rlen > 0 ) {
		if (debug) {
			printf("command buffer:\n");
			for (int k=0;k<24;k++){printf(" %x",m_command.raw.data[k]);}
			printf("\n");
			printf("\nreport buffer:\n");
			for (int k=0;k<m_report_length;k++){printf(" %x",m_report.raw.data[k]);}
			printf("\n");
		}
		

		//printf("\nsrc time rl: %i-%i, sec: %i %i:%i:%i %s\n",rlen,m_report_length,m_primary_time.report.seconds,m_report.extended.data[11],m_report.extended.data[10],m_report.extended.data[9],src);
		//memcpy(dst, src, std::min(m_report_length, rlen));
		//printf("tsip::update_report sec: %i  %i:%i:%i  %i/%i/%i\n",m_primary_time.report.seconds,m_primary_time.report.hours,m_primary_time.report.minutes,m_primary_time.report.seconds,m_primary_time.report.year,m_primary_time.report.month,m_primary_time.report.day);

		//printf("\nsrc time rl: %i-%i, %i/%i/%i %i:%i:%i %s\n",rlen,m_report_length,m_report.extended.data[14],m_report.extended.data[13],m_report.extended.data[12],m_report.extended.data[11],m_report.extended.data[10],m_report.extended.data[9],src);
		
		//memcpy(dst, src, std::min(m_report_length, rlen));


		//printf("tsip::update_report (p_time) sec: %i   %i/%i/%i %i:%i:%i \n",m_primary_time.report.seconds,m_primary_time.report.year,m_primary_time.report.month,m_primary_time.report.day,m_primary_time.report.hours,m_primary_time.report.minutes,m_primary_time.report.seconds);
		//printf("tsip::update_report (p_time) sec: %i %i/%i/%i %i:%i:%i  \n",m_primary_time.rpt.report.seconds,m_primary_time.rpt.report.year,m_primary_time.rpt.report.month,m_primary_time.rpt.report.day,m_primary_time.rpt.report.hours,m_primary_time.rpt.report.minutes,m_primary_time.rpt.report.seconds);

		//printf("after move p_t.data: \n");
		//for (int k=0;k<=m_report_length;k++){printf(" %x",m_primary_time.rpt.data[k]);}
		//printf("\n");
		//for (int k=0;k<=m_report_length;k++){printf(" %i",m_primary_time.rpt.data[k]);}
		//printf("\n");
		//printf("pointer %p \n", src);
		//printf("data    %p \n", (void *)src);
		return 1;
	}


	return 0;
}
