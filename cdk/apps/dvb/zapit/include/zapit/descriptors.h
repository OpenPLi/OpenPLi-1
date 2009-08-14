/*
 * $Id: descriptors.h,v 1.15 2002/10/12 20:19:44 obi Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __descriptors_h__
#define __descriptors_h__

#include "types.h"

uint8_t generic_descriptor (uint8_t *buffer);
uint8_t video_stream_descriptor (uint8_t *buffer);
uint8_t audio_stream_descriptor (uint8_t *buffer);
uint8_t hierarchy_descriptor (uint8_t *buffer);
uint8_t registration_descriptor (uint8_t *buffer);
uint8_t data_stream_alignment_descriptor (uint8_t *buffer);
uint8_t target_background_grid_descriptor (uint8_t *buffer);
uint8_t Video_window_descriptor (uint8_t *buffer);
uint8_t CA_descriptor (uint8_t *buffer, uint16_t ca_system_id, uint16_t* ca_pid);
uint8_t ISO_639_language_descriptor (uint8_t *buffer);
uint8_t System_clock_descriptor (uint8_t *buffer);
uint8_t Multiplex_buffer_utilization_descriptor (uint8_t *buffer);
uint8_t Copyright_descriptor (uint8_t *buffer);
uint8_t Maximum_bitrate_descriptor (uint8_t *buffer);
uint8_t Private_data_indicator_descriptor (uint8_t *buffer);
uint8_t Smoothing_buffer_descriptor (uint8_t *buffer);
uint8_t STD_descriptor (uint8_t *buffer);
uint8_t IBP_descriptor (uint8_t *buffer);
uint8_t MPEG4_video_descriptor (uint8_t *buffer);
uint8_t MPEG4_audio_descriptor (uint8_t *buffer);
uint8_t IOD_descriptor (uint8_t *buffer);
uint8_t SL_descriptor (uint8_t *buffer);
uint8_t FMC_descriptor (uint8_t *buffer);
uint8_t External_ES_ID_descriptor (uint8_t *buffer);
uint8_t MuxCode_descriptor (uint8_t *buffer);
uint8_t FmxBufferSize_descriptor (uint8_t *buffer);
uint8_t MultiplexBuffer_descriptor (uint8_t *buffer);
uint8_t FlexMuxTiming_descriptor (uint8_t *buffer);
uint8_t network_name_descriptor (uint8_t *buffer);
uint8_t service_list_descriptor (uint8_t *buffer, uint16_t original_network_id);
uint8_t stuffing_descriptor (uint8_t *buffer);
uint8_t satellite_delivery_system_descriptor (uint8_t *buffer, uint16_t transport_stream_id, uint16_t original_network_id, uint8_t DiSEqC);
uint8_t cable_delivery_system_descriptor (uint8_t *buffer, uint16_t transport_stream_id, uint16_t original_network_id);
uint8_t VBI_data_descriptor (uint8_t *buffer);
uint8_t VBI_teletext_descriptor (uint8_t *buffer);
uint8_t bouquet_name_descriptor (uint8_t *buffer);
uint8_t service_descriptor (uint8_t *buffer, const t_service_id service_id, const t_transport_stream_id transport_stream_id, const t_original_network_id original_network_id, const uint8_t DiSEqC);
uint8_t country_availability_descriptor (uint8_t *buffer);
uint8_t linkage_descriptor (uint8_t *buffer);
uint8_t NVOD_reference_descriptor (uint8_t *buffer);
uint8_t time_shifted_service_descriptor (uint8_t *buffer);
uint8_t short_event_descriptor (uint8_t *buffer);
uint8_t extended_event_descriptor (uint8_t *buffer);
uint8_t time_shifted_event_descriptor (uint8_t *buffer);
uint8_t component_descriptor (uint8_t *buffer);
uint8_t mosaic_descriptor (uint8_t *buffer);
uint8_t stream_identifier_descriptor (uint8_t *buffer);
uint8_t CA_identifier_descriptor (uint8_t *buffer);
uint8_t content_descriptor (uint8_t *buffer);
uint8_t parental_rating_descriptor (uint8_t *buffer);
uint8_t teletext_descriptor (uint8_t *buffer);
uint8_t telephone_descriptor (uint8_t *buffer);
uint8_t local_time_offset_descriptor (uint8_t* buffer);
uint8_t subtitling_descriptor (uint8_t *buffer);
uint8_t terrestrial_delivery_system_descriptor (uint8_t *buffer);
uint8_t multilingual_network_name_descriptor (uint8_t *buffer);
uint8_t multilingual_bouquet_name_descriptor (uint8_t *buffer);
uint8_t multilingual_service_name_descriptor (uint8_t *buffer);
uint8_t multilingual_component_descriptor (uint8_t *buffer);
uint8_t private_data_specifier_descriptor (uint8_t *buffer);
uint8_t service_move_descriptor (uint8_t *buffer);
uint8_t short_smoothing_buffer_descriptor (uint8_t *buffer);
uint8_t frequency_list_descriptor (uint8_t *buffer);
uint8_t partial_transport_stream_descriptor (uint8_t *buffer);
uint8_t data_broadcast_descriptor (uint8_t *buffer);
uint8_t CA_system_descriptor (uint8_t *buffer);
uint8_t data_broadcast_id_descriptor (uint8_t *buffer);
uint8_t transport_stream_descriptor (uint8_t *buffer);
uint8_t DSNG_descriptor (uint8_t *buffer);
uint8_t PDC_descriptor (uint8_t *buffer);
uint8_t AC3_descriptor (uint8_t *buffer);
uint8_t ancillary_data_descriptor (uint8_t *buffer);
uint8_t cell_list_descriptor (uint8_t *buffer);
uint8_t cell_frequency_link_descriptor (uint8_t *buffer);
uint8_t announcement_support_descriptor (uint8_t *buffer);

#endif /* __descriptors_h__ */
