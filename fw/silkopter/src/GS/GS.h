#pragma once

#include <stdint.h>
#include <stddef.h>
#include "GS/SFull_Protocol.h"
#include "GS/SCompact_Protocol.h"
#include "util/Noncopyable.h"
#include "board/UART.h"
#include "util/chrono.h"

namespace silk
{

	class GS : util::Noncopyable
	{
	public:
		explicit GS(board::UART& full_uart);
		GS(board::UART& full_uart, board::UART& compact_uart);

		void process(chrono::micros max_duration);

	private:
		board::UART& m_full_uart;
		board::UART* m_compact_uart;

		SFull_Protocol m_full_protocol;
		//SCompact_Protocol m_compact_protocol;
		SFull_Protocol m_compact_protocol;
		
		uint32_t m_step;
		
		chrono::time_us m_last_time;
		chrono::micros m_frame_duration;
		
		bool send_data(uint32_t step); //returns true when done
	};
}