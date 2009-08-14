/*
 *  $Id: scan.h,v 1.23 2002/09/24 16:46:17 thegoodguy Exp $
 */

#ifndef __scan_h__
#define __scan_h__

#include <ost/frontend.h>

#include <stdint.h>

#include <map>
#include <string>

#include "bouquets.h"

struct transpondermap
{
	t_transport_stream_id transport_stream_id;
	t_original_network_id original_network_id;
	FrontendParameters    feparams;
	uint8_t               polarization;
	uint8_t               DiSEqC;

	transpondermap(const t_transport_stream_id p_transport_stream_id, const t_original_network_id p_original_network_id, const FrontendParameters p_feparams, const uint8_t p_polarization = 0, const uint8_t p_DiSEqC = 0)
	{
		transport_stream_id = p_transport_stream_id;
		original_network_id = p_original_network_id;
		feparams            = p_feparams;
		polarization        = p_polarization;
		DiSEqC              = p_DiSEqC;
	}
};

extern std::map <uint32_t, transpondermap> scantransponders;
typedef std::map <uint32_t, transpondermap>::iterator stiterator;

extern CBouquetManager* scanBouquetManager;

#endif /* __scan_h__ */
