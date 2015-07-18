#pragma once


#include "utils/Timed_Scope.h"
#include "lz4/lz4.h"

namespace util
{
namespace detail
{
    inline void intrusive_ptr_add_ref(Pool_Item_Base* p)
    {
        QASSERT(p);
        if (p->count.fetch_add(1, std::memory_order_relaxed) > 999)
        {
            QASSERT(0);
        }
    }
    inline void intrusive_ptr_release(Pool_Item_Base* p)
    {
        QASSERT(p);
        if (p->count.fetch_sub(1, std::memory_order_release) == 1u)
        {
            if (p->gc)
            {
                p->gc(p);
            }
            else
            {
                QASSERT(0);
                delete p;
            }
        }
    }
    template<class T> Pool<T>::Pool()
    {
        m_pool = std::make_shared<Items>();

        auto items_ref = m_pool;
        m_garbage_collector = [items_ref, this](Pool_Item_Base* t)
        {
            if (release)
            {
                release(static_cast<T&>(*t));
            }

            std::lock_guard<std::mutex> lg(items_ref->mutex);
            items_ref->items.emplace_back(static_cast<T*>(t)); //will create a unique pointer from the raw one
//                QLOGI("{}// new:{} reused:{} returned:{}", m_pool.get(), x_new, x_reused, x_returned);
//                QLOGI("{}// returned: {} / {}", m_pool.get(), t, x_returned);
        };
    }

    template<class T> auto Pool<T>::acquire() -> Ptr
    {
        //this will be called when the last shared_ptr to T dies. We can safetly return the object to pur pool

        std::lock_guard<std::mutex> lg(m_pool->mutex);
        T* item = nullptr;
        if (!m_pool->items.empty())
        {
            item = m_pool->items.back().release(); //release the raw ptr from the control of the unique ptr
            m_pool->items.pop_back();
//                QLOGI("{}// recycled: {} / {}", m_pool.get(), item, x_reused);
//                QLOGI("{}// new:{} reused:{} returned:{}", m_pool.get(), x_new, x_reused, x_returned);
        }
        else
        {
            item = new T;
            item->gc = m_garbage_collector;
            //if (x_new > 1000)
            {
//                    QLOGI("{}// allocated: {} / {}", m_pool.get(), item, x_new);
                //QLOGI("{}// new:{} reused:{} returned:{}", m_pool.get(), x_new, x_reused, x_returned);
            }
        }
        QASSERT(item);
        return Ptr(item);
    }
}

inline auto RCP::acquire_tx_datagram(size_t data_size) -> RCP::TX::Datagram_ptr
{
    return acquire_tx_datagram(data_size, data_size);
}

inline auto RCP::acquire_tx_datagram(size_t zero_size, size_t data_size) -> RCP::TX::Datagram_ptr
{
    auto datagram = m_tx.datagram_pool.acquire();

    QASSERT(zero_size <= data_size);
    datagram->data.resize(data_size);

    std::fill(datagram->data.begin(), datagram->data.begin() + zero_size, 0);

    datagram->added_tp = q::Clock::time_point(q::Clock::duration{0});
    datagram->sent_tp = q::Clock::time_point(q::Clock::duration{0});
    datagram->sent_count = 0;

    return datagram;
}

inline auto RCP::acquire_rx_datagram(size_t data_size) -> RCP::RX::Datagram_ptr
{
    return acquire_rx_datagram(data_size, data_size);
}
inline auto RCP::acquire_rx_datagram(size_t zero_size, size_t data_size) -> RCP::RX::Datagram_ptr
{
    auto datagram = m_rx.datagram_pool.acquire();
    QASSERT(zero_size <= data_size);
    datagram->data.resize(data_size);
    std::fill(datagram->data.begin(), datagram->data.begin() + zero_size, 0);
    return datagram;
}
inline auto RCP::RX::acquire_packet() -> RCP::RX::Packet_ptr
{
    auto packet = packet_pool.acquire();
    packet->received_fragment_count = 0;
    packet->added_tp = q::Clock::now();
    return packet;
}

template<class H> auto RCP::get_header(uint8_t const* data) -> H const&
{
    QASSERT(data);
    auto const& h = *reinterpret_cast<H const*>(data);
    return h;
}
template<class H> auto RCP::get_header(uint8_t* data) -> H&
{
    QASSERT(data);
    auto& h = *reinterpret_cast<H*>(data);
    return h;
}

inline auto RCP::get_header_size(Buffer_t& data) -> size_t
{
    if (data.size() < sizeof(Header))
    {
        return 0;
    }
    auto const& header = get_header<Header>(data.data());
    switch (header.type)
    {
    case Type::TYPE_PACKET:
    {
        auto const& pheader = get_header<Packet_Header>(data.data());
        return (pheader.fragment_idx == 0) ? sizeof(Packet_Main_Header) : sizeof(Packet_Header);
    }
    case Type::TYPE_FRAGMENTS_RES: return sizeof(Fragments_Res_Header);
    case Type::TYPE_PACKETS_RES: return sizeof(Packets_Res_Header);
    case Type::TYPE_CONNECT_REQ: return sizeof(Connect_Req_Header);
    case Type::TYPE_CONNECT_RES: return sizeof(Connect_Res_Header);
    }
    return 0;
}

inline auto RCP::compute_crc(uint8_t const* data, size_t size) -> uint32_t
{
    auto crc = q::util::compute_murmur_hash32(data, size, 0);
    return crc;
}

//to avoid popping front in vectors
//Note - I use a vector instead of a deque because the deque is too slow at iterating.
//The erase can be implemented optimally for vectors if order need not be preserved.
template<class T> void RCP::erase_unordered(std::vector<T>& c, typename std::vector<T>::iterator it)
{
    QASSERT(!c.empty());
    if (it + 1 == c.end())
    {
        c.pop_back();
    }
    else
    {
        std::swap(*it, c.back());
        c.pop_back();
    }
}

//////////////////////////////////////////////////////////////////////////////

inline RCP::RCP(RCP_Socket& socket)
    : m_socket(socket)
{
//        auto hsz =sizeof(Header);
//        hsz =sizeof(Packet_Main_Header);
//        hsz =sizeof(Packet_Header);

//        hsz =sizeof(Packets_Confirmed_Header);

    m_socket.receive_callback = std::bind(&RCP::handle_receive, this, std::placeholders::_1, std::placeholders::_2);
    m_socket.send_callback = std::bind(&RCP::handle_send, this, std::placeholders::_1);

    m_rx.packet_pool.release = [](RX::Packet& p)
    {
        p.fragments.clear();
    };

//        m_rx.temp_buffer.resize(100 * 1024);

    std::fill(m_rx.last_packet_ids.begin(), m_rx.last_packet_ids.end(), 0);
    std::fill(m_last_id.begin(), m_last_id.end(), 0);

    m_global_receive_params.max_receive_time = std::chrono::seconds(5);

    m_init_tp = q::Clock::now();

    for (auto& ch: m_tx.channel_data)
    {
        ch.lz4_state.resize(LZ4_sizeofState());
    }
}

inline void RCP::set_send_params(uint8_t channel_idx, Send_Params const& params)
{
    if (channel_idx >= MAX_CHANNELS)
    {
        QASSERT(0);
        return;
    }
    m_send_params[channel_idx] = params;
}
inline auto RCP::get_send_params(uint8_t channel_idx) const -> Send_Params const&
{
    if (channel_idx >= MAX_CHANNELS)
    {
        QASSERT(0);
        return m_send_params[0];
    }
    return m_send_params[channel_idx];
}

inline void RCP::set_receive_params(uint8_t channel_idx, Receive_Params const& params)
{
    if (channel_idx >= MAX_CHANNELS)
    {
        QASSERT(0);
        return;
    }
    m_receive_params[channel_idx] = params;
}
inline void RCP::reconnect()
{
    std::lock_guard<std::mutex> lg(m_connection.mutex);
    disconnect();
    m_connection.last_sent_tp = q::Clock::now() - RECONNECT_BEACON_TIMEOUT;
}
inline auto RCP::is_connected() const -> bool
{
    bool is_connected = false;
    {
        std::lock_guard<std::mutex> lg(m_connection.mutex);
        is_connected = m_connection.is_connected;
    }
    return is_connected;
}
inline auto RCP::send(uint8_t channel_idx, uint8_t const* data, size_t size) -> bool
{
    auto const& params = m_send_params[channel_idx];
    return send(channel_idx, params, data, size);
}
inline auto RCP::try_sending(uint8_t channel_idx, uint8_t const* data, size_t size) -> bool
{
    auto const& params = m_send_params[channel_idx];
    return try_sending(channel_idx, params, data, size);
}

inline auto RCP::send(uint8_t channel_idx, Send_Params const& params, uint8_t const* data, size_t size) -> bool
{
    QLOG_TOPIC("RCP::send");
    if (!data || size == 0 || channel_idx >= MAX_CHANNELS)
    {
        QLOGE("Invalid channel ({}) or data ({}, {} bytes)", channel_idx, data, size);
        return false;
    }
    if (!m_connection.is_connected)
    {
        QLOGE("Not connected.");
        return false;
    }

    auto& channel_data = m_tx.channel_data[channel_idx];
    std::lock_guard<std::mutex> lg(channel_data.send_mutex);

    return _send_locked(channel_idx, params, data, size);
}
inline auto RCP::try_sending(uint8_t channel_idx, Send_Params const& params, uint8_t const* data, size_t size) -> bool
{
    QLOG_TOPIC("RCP::try_sending");
    if (!data || size == 0 || channel_idx >= MAX_CHANNELS)
    {
        QLOGE("Invalid channel ({}) or data ({}, {} bytes)", channel_idx, data, size);
        return false;
    }
    if (!m_connection.is_connected)
    {
        QLOGE("Not connected.");
        return false;
    }

    auto& channel_data = m_tx.channel_data[channel_idx];
    if (channel_data.send_mutex.try_lock())
    {
        bool res = _send_locked(channel_idx, params, data, size);
        channel_data.send_mutex.unlock();
        return res;
    }
    QLOGW("Cannot send packet because still sending previous one");
    return false;
}

inline auto RCP::_send_locked(uint8_t channel_idx, Send_Params const& params, uint8_t const* data, size_t size) -> bool
{
    //The send mutex should be locked here!!!
    if (size >= (1 << 24))
    {
        QLOGE("Packet too big: {}.", size);
        return false;
    }

    auto& channel_data = m_tx.channel_data[channel_idx];

    bool is_compressed = params.is_compressed;
    size_t uncompressed_size = size;
    if (is_compressed)
    {
        //auto start = q::Clock::now();

        auto comp_size = LZ4_compressBound(static_cast<int>(size));
        channel_data.compression_buffer.resize(comp_size);
        int ret = LZ4_compress_fast_extState(channel_data.lz4_state.data(),
                                             reinterpret_cast<const char*>(data),
                                             reinterpret_cast<char*>(channel_data.compression_buffer.data()),
                                             static_cast<int>(size),
                                             comp_size,
                                             5);
        if (ret > 0 && static_cast<size_t>(ret) < uncompressed_size)
        {
//            QLOGI("Compressed {}B -> {}B. {}% : {}", static_cast<int>(uncompressed_size), ret, ret * 100.f / uncompressed_size, q::Clock::now() - start);
            channel_data.compression_buffer.resize(ret);
            data = channel_data.compression_buffer.data();
            size = ret;
        }
        else
        {
            if (ret <= 0)
            {
                QLOGW("Cannot compress data: {}. Sending uncompressed", ret);
            }
            is_compressed = false;
        }
    }

    size_t max_fragment_size = m_socket.get_mtu() - sizeof(Packet_Main_Header);
    size_t left = size;
    size_t fragment_count = ((size - 1) / max_fragment_size) + 1;
    if (fragment_count > MAX_FRAGMENTS)
    {
        auto max = MAX_FRAGMENTS;
        QLOGE("Too many fragments: {} / {}.", fragment_count, max);
        return false;
    }

    auto now = q::Clock::now();

    auto id = ++m_last_id[channel_idx];

    {
        //cancel all previous packets if needed
        if (params.cancel_previous_data)
        {
            auto& queue = m_tx.packet_queue;
            std::lock_guard<std::mutex> lg(m_tx.packet_queue_mutex);

            for (auto it = queue.begin(); it != queue.end(); ++it)
            {
                if (*it)
                {
                    auto const& hdr = get_header<Packet_Header>((*it)->data.data());
                    if (hdr.channel_idx == channel_idx && hdr.id < id)
                    {
                        (*it).reset();
                    }
                }
            }
        }

//        QLOGI("crt {}, +{}", queue.size(), fragment_count);

        channel_data.fragments_to_insert.resize(0);
        channel_data.fragments_to_insert.reserve(fragment_count);

        //add the new fragments
        for (size_t i = 0; i < fragment_count; i++)
        {
            QASSERT(left > 0);
            auto fragment_size = math::min(max_fragment_size, left);

            auto header_size = (i == 0) ? sizeof(Packet_Main_Header) : sizeof(Packet_Header);
            auto fragment = acquire_tx_datagram(header_size, header_size + fragment_size);

            fragment->params = params;

            auto& header = get_header<Packet_Header>(fragment->data.data());
            header.id = id;
            header.channel_idx = channel_idx;
            header.flag_needs_confirmation = params.is_reliable || params.unreliable_retransmit_count > 0;
            header.flag_is_compressed = is_compressed;
            header.type = TYPE_PACKET;
            header.fragment_idx = i;

            if (i == 0)
            {
                auto& f_header = get_header<Packet_Main_Header>(fragment->data.data());
                f_header.packet_size = uncompressed_size;
                f_header.fragment_count = fragment_count;
            }
            std::copy(data, data + fragment_size, fragment->data.data() + header_size);

            data += fragment_size;
            left -= fragment_size;

            fragment->added_tp = now;
            prepare_to_send_datagram(*fragment);

            channel_data.fragments_to_insert.push_back(fragment);
        }
//        for (auto const& d: queue)
//        {
//            QLOGI("{}, id: {}, f: {}, sc: {}", &d - &queue.front(), int(get_header<Packet_Header>(d->data).id), int(get_header<Packet_Header>(d->data).fragment_idx), d->sent_count);
//        }

        if (!channel_data.fragments_to_insert.empty())
        {
            auto& queue = m_tx.packet_queue;
            std::lock_guard<std::mutex> lg(m_tx.packet_queue_mutex);
            while (!queue.empty() && !queue.front()) queue.pop_front();

            queue.insert(std::upper_bound(queue.begin(), queue.end(), channel_data.fragments_to_insert[0], tx_packet_datagram_predicate),
                    channel_data.fragments_to_insert.begin(),
                    channel_data.fragments_to_insert.end());
        }

    }

    send_datagram();

    return true;
}

inline auto RCP::tx_packet_datagram_predicate(TX::Datagram_ptr const& AA, TX::Datagram_ptr const& BB) -> bool
{
    if (!AA)
    {
        return true;
    }
    if (!BB)
    {
        return false;
    }
    int priority_AA = AA->params.importance;
    int priority_BB = BB->params.importance;
    int sent_count_AA = AA->sent_count;
    int sent_count_BB = BB->sent_count;

    return priority_AA > priority_BB ||
            (priority_AA == priority_BB && sent_count_AA < sent_count_BB) ||
            (priority_AA == priority_BB && sent_count_AA == sent_count_BB && get_header<Packet_Header>(AA->data.data()).id < get_header<Packet_Header>(BB->data.data()).id);
}

inline bool RCP::receive(uint8_t channel_idx, std::vector<uint8_t>& data)
{
    QLOG_TOPIC("RCP::receive");

    data.clear();
    if (channel_idx >= MAX_CHANNELS)
    {
        QLOGE("Invalid channel {}", channel_idx);
        return false;
    }
    if (!m_connection.is_connected)
    {
        return false;
    }

    auto& queue = m_rx.packet_queues[channel_idx];
    auto& packets = queue.packets;
    std::lock_guard<std::mutex> lg(queue.mutex);

    auto const& params = m_receive_params[channel_idx];


    auto& last_packet_id = m_rx.last_packet_ids[channel_idx];

    while (true)
    {
        if (packets.empty())
        {
            break;
        }
        RX::Packet_ptr const& packet = packets.front().second;
        QASSERT(packet);

        auto next_expected_id = last_packet_id + 1;

        auto id = packet->any_header.id;
        QASSERT(id > 0);

        //is it from the past?
        if (id <= last_packet_id)
        {
            QLOGW("Ignoring past packet {}", id);
            m_global_stats.rx_zombie_datagrams++;
            if (packet->any_header.flag_needs_confirmation)
            {
                add_and_send_packet_res(channel_idx, id);
            }
            packets.pop_front();
            m_global_stats.rx_dropped_packets++;
            continue;
        }

        auto now = q::Clock::now();
        auto max_receive_time = params.max_receive_time.count() > 0 ? params.max_receive_time : m_global_receive_params.max_receive_time;
        bool is_late = (max_receive_time.count() > 0 && now - packet->added_tp >= max_receive_time);

        //the next packet in sequence is missing - wait for it some more or cancel
        if (id > next_expected_id)
        {
            if (!is_late)
            {
                //wait some more
                break;
            }

            //waited enough, cancel the pending packet
            QLOGW("Canceling ghost packet {}", next_expected_id);
            add_and_send_packet_res(channel_idx, next_expected_id);
            last_packet_id = next_expected_id;
            m_global_stats.rx_dropped_packets++;
            continue;
        }

        //no header yet or not all packages received?
        if (!packet->fragments[0] || packet->received_fragment_count < packet->main_header.fragment_count)
        {
            if (!is_late)
            {
                break;
            }

            //QLOGW("Canceling late packet {}. {} / {}", id, packet->received_fragment_count, packet->fragments[0] ? packet->main_header.fragment_count : 0);
            add_and_send_packet_res(channel_idx, id);
            packets.pop_front();
            last_packet_id = id;
            m_global_stats.rx_dropped_packets++;

//                QLOGW("Still waiting for packet {}: {}/{} received", id, packet->received_fragment_count, static_cast<size_t>(packet->main_header.fragment_count));
            continue;
        }
        QASSERT(packet->fragments[0]);

        //QLOGI("Received packet {}", id);

        last_packet_id = id;
        m_global_stats.rx_packets++;

        //copy the data
        auto const& main_header = packet->main_header;
        m_rx.compression_buffer.clear();
        m_rx.compression_buffer.resize(main_header.packet_size);
        size_t offset = 0;
        size_t fragment_idx = 0;
        for (auto it = packet->fragments.begin(); it != packet->fragments.end(); ++it, fragment_idx++)
        {
            auto const& fragment = it->second;
            QASSERT(it->first == fragment_idx);
            auto header_size = (fragment_idx == 0 ? sizeof(Packet_Main_Header) : sizeof(Packet_Header));
            QASSERT(fragment->data.size() > header_size);
            std::copy(fragment->data.begin() + header_size, fragment->data.end(), m_rx.compression_buffer.begin() + offset);
            offset += fragment->data.size() - header_size;
        }
        packet->fragments.clear();

        if (main_header.flag_is_compressed)
        {
            data.resize(main_header.packet_size);
            int ret = LZ4_decompress_fast(reinterpret_cast<const char*>(m_rx.compression_buffer.data()),
                                          reinterpret_cast<char*>(data.data()),
                                          static_cast<int>(main_header.packet_size));
            if (ret < 0)
            {
                QLOGW("Decompression error: {}", ret);
                data.clear();
            }
            //return true;//ret == Z_OK;
        }
        else
        {
            QASSERT(m_rx.compression_buffer.size() == main_header.packet_size);
            std::swap(data, m_rx.compression_buffer);
        }

        packets.pop_front();

        break;
    }

    return !data.empty();
}

inline void RCP::process_connection()
{
    if (m_connection.is_connected)
    {
        return;
    }

    std::lock_guard<std::mutex> lg(m_connection.mutex);

    auto now = q::Clock::now();
    if (now - m_connection.last_sent_tp < RECONNECT_BEACON_TIMEOUT)
    {
        return;
    }
    m_connection.last_sent_tp = now;

    QLOGI("Sending reconnect request");
    send_packet_connect_req();
}

inline void RCP::process()
{
    {
        static q::Clock::time_point tp = q::Clock::now();
        if (q::Clock::now() - tp > std::chrono::seconds(1))
        {
            tp = q::Clock::now();
            std::lock_guard<std::mutex> lg(m_tx.packet_queue_mutex);
            QLOGI("{}: tx {} / rx {} / cf {}", m_tx.packet_queue.size(), m_global_stats.tx_datagrams, m_global_stats.rx_datagrams, m_global_stats.tx_confirmed_fragments);
        }
    }

    if (!m_connection.is_connected)
    {
        process_connection();
    }
    else
    {
        send_pending_fragments_res();
        send_pending_packets_res();
    }

    send_datagram();
}

inline void RCP::prepare_to_send_datagram(TX::Datagram& datagram)
{
    auto& header = get_header<Header>(datagram.data.data());
    auto crc = compute_crc(datagram.data.data(), datagram.data.size());
    header.crc = crc;

    if (datagram.added_tp.time_since_epoch().count() == 0)
    {
        datagram.added_tp = q::Clock::now();
    }
    datagram.sent_tp = q::Clock::time_point(q::Clock::duration{0});

//            if (header.crc == 0)
//            {
//                for (auto x: datagram->data)
//                {
//                    QLOGI("{}", x);
//                }
//                QLOGI("crc: {}/{}", crc, static_cast<int>(header.crc));
//            }

//            QASSERT_MSG(header.crc != 0, "{}", crc);
}

inline void RCP::add_and_send_datagram(TX::Send_Queue& queue, std::mutex& mutex, TX::Datagram_ptr const& datagram)
{
    if (!datagram)
    {
        QASSERT(0);
        return;
    }

    prepare_to_send_datagram(*datagram);

    //add to the queue
    {
        std::lock_guard<std::mutex> lg(mutex);
        queue.push_back(datagram);
    }
    send_datagram();
}

inline void RCP::update_stats(Stats& stats, TX::Datagram const& datagram)
{
    auto const& header = get_header<Header>(datagram.data.data());
    stats.tx_datagrams++;
    stats.tx_bytes += datagram.data.size();
    if (header.type == TYPE_PACKET)
    {
        auto const& pheader = get_header<Packet_Header>(datagram.data.data());
        if (pheader.fragment_idx == 0)
        {
            stats.tx_packets++;
        }
    }
}

inline void RCP::compute_next_transit_datagram()
{
    TX::Datagram_ptr& datagram = m_tx.in_transit_datagram;

    {
        std::lock_guard<std::mutex> lg(m_tx.internal_queues.mutex);
        if (m_tx.internal_queues.connection_res)
        {
            datagram = std::move(m_tx.internal_queues.connection_res);
            m_tx.internal_queues.connection_res.reset();
//                QLOGI("Sending connection res datagram {}", static_cast<int>(get_header<Con_Header>(datagram->data).seq));
        }
        else if (m_tx.internal_queues.connection_req)
        {
            datagram = std::move(m_tx.internal_queues.connection_req);
            m_tx.internal_queues.connection_req.reset();
//                QLOGI("Sending connection_req datagram {}", static_cast<int>(get_header<connection_req_Header>(datagram->data).seq));
        }

        //next the confirmations
        if (!datagram)
        {
            auto& queue = m_tx.internal_queues.fragments_res;
            if (!queue.empty())
            {
                datagram = std::move(queue.front());
//                    QLOGI("Sending confirmation for {} datagrams", static_cast<int>(get_header<Packets_Confirmed_Header>(datagram->data).count));
                queue.pop_front();
            }
        }
        //next the cancels
        if (!datagram)
        {
            auto& queue = m_tx.internal_queues.packets_res;
            if (!queue.empty())
            {
                datagram = std::move(queue.front());
//                    QLOGI("Sending cancellation for {} datagrams", static_cast<int>(get_header<Packets_Cancelled_Header>(datagram->data).count));
                queue.pop_front();
            }
        }
    }

    //next normal packets
    if (!datagram)
    {
        auto& queue = m_tx.packet_queue;
        std::lock_guard<std::mutex> lg(m_tx.packet_queue_mutex);

        auto now = q::Clock::now();

        while (!queue.empty())
        {
            auto& datagram = queue.front();
            //eliminate empty / old datagrams
            if (!datagram || (datagram->params.cancel_after.count() > 0 && now - datagram->added_tp >= datagram->params.cancel_after))
            {
                //auto const& header = get_header<Packet_Header>(datagram->data);
                //QLOGI("Cancelling fragment {} for packet {}, sent {} times", header.fragment_idx, header.id, datagram->sent_count);
                queue.pop_front();
                continue;
            }
            else
            {
                //found it!!
                break;
            }
        }
        if (!queue.empty())
        {
            datagram = std::move(queue.front());
            queue.pop_front();//erase as the order changed due to the sent_count increase

            datagram->sent_count++;

            //do I have to insert it back?
            if (datagram->params.is_reliable == true || datagram->sent_count < datagram->params.unreliable_retransmit_count)
            {
                queue.insert(std::upper_bound(queue.begin(), queue.end(), datagram, tx_packet_datagram_predicate), datagram);
            }
            //auto& header = get_header<Packet_Header>(datagram->data);
            //QLOGI("{}: fr {}, ch {}, {}", queue.size(), header.fragment_idx, static_cast<int>(header.channel_idx), header.flag_is_reliable ? "R" : "NR");
        }
    }
}

inline void RCP::send_datagram()
{
    if (m_is_sending.exchange(true))
    {
        //was already sending, return
        //QLOGI("send blocked {}", q::Clock::now());
        return;
    }

    compute_next_transit_datagram();
    if (!m_tx.in_transit_datagram)
    {
        m_is_sending = false;
        return;
    }

    update_stats(m_global_stats, *m_tx.in_transit_datagram);

    //QLOGI("Sending after {}", q::Clock::now() - datagram->added_tp);

    m_socket.async_send(m_tx.in_transit_datagram->data.data(), m_tx.in_transit_datagram->data.size());
}

inline void RCP::handle_send(RCP_Socket::Result result)
{
    QASSERT(m_tx.in_transit_datagram);

    auto datagram = std::move(m_tx.in_transit_datagram);
    QASSERT(datagram);
    if (datagram)
    {
        datagram->sent_tp = q::Clock::now();
    }

    m_is_sending = false;

    send_datagram();
}

inline void RCP::handle_receive(uint8_t const* data, size_t size)
{
    if (size > 0)
    {
        RX::Datagram_ptr datagram = acquire_rx_datagram(0, size);
        std::copy(data, data + size, datagram->data.begin());
        process_incoming_datagram(datagram);
    }
}

inline void RCP::send_pending_fragments_res()
{
    std::lock_guard<std::mutex> lg(m_tx.fragments_res_mutex);
    send_pending_fragments_res_locked();
}

inline void RCP::send_pending_fragments_res_locked()
{
    //std::lock_guard<std::mutex> lg(m_tx.fragments_res_mutex);

    //first throw away the ones we sent enough times
    while (!m_tx.fragments_res.empty() && m_tx.fragments_res.front().sent_count >= TX::MAX_FRAGMENT_RES_SEND_COUNT)
    {
        m_tx.fragments_res.pop_front();
    }

    //now take them and pack them in a datagram
    size_t pidx = 0;
    while (pidx < m_tx.fragments_res.size())
    {
        size_t count = math::min(m_tx.fragments_res.size() - pidx, Fragments_Res_Header::MAX_PACKED);

        size_t header_size = sizeof(Fragments_Res_Header);
        TX::Datagram_ptr datagram = acquire_tx_datagram(header_size, header_size + count * sizeof(Fragments_Res_Header::Data)); //channel_idx:1 + id:3 + fragment_idx:2

        auto& header = get_header<Fragments_Res_Header>(datagram->data.data());
        header.type = TYPE_FRAGMENTS_RES;
        header.count = static_cast<uint8_t>(count);

        Fragments_Res_Header::Data* data_ptr = reinterpret_cast<Fragments_Res_Header::Data*>(datagram->data.data() + header_size);
        for (size_t i = 0; i < count; i++, pidx++, data_ptr++)
        {
            auto& res = m_tx.fragments_res[pidx];
            res.sent_count++;
            data_ptr->id = res.id;
            data_ptr->fragment_idx = res.fragment_idx;
            data_ptr->channel_idx = res.channel_idx;
        }

        add_and_send_datagram(m_tx.internal_queues.fragments_res, m_tx.internal_queues.mutex, datagram);
    }
}

inline void RCP::add_and_send_fragment_res(uint8_t channel_idx, uint32_t id, uint16_t fragment_idx)
{
    std::lock_guard<std::mutex> lg(m_tx.fragments_res_mutex);

    TX::Fragment_Res res;
    res.channel_idx = channel_idx;
    res.id = id;
    res.fragment_idx = fragment_idx;
    m_tx.fragments_res.push_back(res);

    send_pending_fragments_res_locked();
}
inline void RCP::send_pending_packets_res()
{
    std::lock_guard<std::mutex> lg(m_tx.packets_res_mutex);
    send_pending_packets_res_locked();
}
inline void RCP::send_pending_packets_res_locked()
{
    //std::lock_guard<std::mutex> lg(m_tx.packets_res_mutex);

    //first throw away the ones we sent enough times
    while (!m_tx.packets_res.empty() && m_tx.packets_res.front().sent_count >= TX::MAX_PACKET_RES_SEND_COUNT)
    {
        m_tx.packets_res.pop_front();
    }

    //now take them and pack them in a datagram
    size_t pidx = 0;
    while (pidx < m_tx.packets_res.size())
    {
        size_t count = math::min(m_tx.packets_res.size() - pidx, Packets_Res_Header::MAX_PACKED);

        size_t header_size = sizeof(Packets_Res_Header);
        TX::Datagram_ptr datagram = acquire_tx_datagram(header_size, header_size + count * sizeof(Packets_Res_Header::Data)); //channel_idx:1 + id:3);

        auto& header = get_header<Packets_Res_Header>(datagram->data.data());
        header.type = TYPE_PACKETS_RES;
        header.count = static_cast<uint8_t>(count);

        Packets_Res_Header::Data* data_ptr = reinterpret_cast<Packets_Res_Header::Data*>(datagram->data.data() + header_size);
        for (size_t i = 0; i < count; i++, pidx++, data_ptr++)
        {
            auto& res = m_tx.packets_res[pidx];
            res.sent_count++;
            data_ptr->id = res.id;
            data_ptr->channel_idx = res.channel_idx;
        }

        add_and_send_datagram(m_tx.internal_queues.packets_res, m_tx.internal_queues.mutex, datagram);
    }
}

inline void RCP::add_and_send_packet_res(uint8_t channel_idx, uint32_t id)
{
    std::lock_guard<std::mutex> lg(m_tx.packets_res_mutex);

    TX::Packet_Res res;
    res.channel_idx = channel_idx;
    res.id = id;
    m_tx.packets_res.push_back(res);

    send_pending_packets_res_locked();
}

inline void RCP::send_packet_connect_req()
{
    {
        std::lock_guard<std::mutex> lg(m_tx.internal_queues.mutex);

        m_tx.internal_queues.connection_req = acquire_tx_datagram(sizeof(Connect_Req_Header));
        auto& header = get_header<Connect_Req_Header>(m_tx.internal_queues.connection_req->data.data());
        header.version = VERSION;
        header.type = TYPE_CONNECT_REQ;

        prepare_to_send_datagram(*m_tx.internal_queues.connection_req);
    }

    send_datagram();
}
inline void RCP::send_packet_connect_res(Connect_Res_Header::Response response)
{
    {
        std::lock_guard<std::mutex> lg(m_tx.internal_queues.mutex);

        m_tx.internal_queues.connection_res = acquire_tx_datagram(sizeof(Connect_Res_Header));
        auto& header = get_header<Connect_Res_Header>(m_tx.internal_queues.connection_res->data.data());
        header.version = VERSION;
        header.response = response;
        header.type = TYPE_CONNECT_RES;

        prepare_to_send_datagram(*m_tx.internal_queues.connection_res);
    }
    send_datagram();
}

inline void RCP::process_incoming_datagram(RX::Datagram_ptr& datagram)
{
    QASSERT(datagram);
    size_t h_size = get_header_size(datagram->data);
    if (h_size == 0)
    {
        QLOGW("Unknonw header.");
        return;
    }

    auto& header = get_header<Header>(datagram->data.data());

    m_global_stats.rx_datagrams++;

    auto crc1 = header.crc;
    header.crc = 0;
    auto crc2 = compute_crc(datagram->data.data(), datagram->data.size());
    if (crc1 != crc2)
    {
        m_global_stats.rx_corrupted_datagrams++;
        auto loss = m_global_stats.rx_corrupted_datagrams * 100 / m_global_stats.rx_datagrams;

        QLOGW("Crc is wrong. {} != {}. Packet loss: {.2}", crc1, crc2, loss);
        return;
    }
    m_global_stats.rx_good_datagrams++;

    if (!m_connection.is_connected)
    {
        switch (header.type)
        {
        case Type::TYPE_CONNECT_REQ: process_connect_req_datagram(datagram); break;
        case Type::TYPE_CONNECT_RES: process_connect_res_datagram(datagram); break;
        default: QLOGI("Disconnected: Ignoring datagram type {}", static_cast<int>(header.type)); break;
        }
    }
    else
    {
        switch (header.type)
        {
        case Type::TYPE_PACKET: process_packet_datagram(datagram); break;
        case Type::TYPE_FRAGMENTS_RES: process_fragments_res_datagram(datagram); break;
        case Type::TYPE_PACKETS_RES: process_packets_res_datagram(datagram); break;
        case Type::TYPE_CONNECT_REQ: process_connect_req_datagram(datagram); break;
        case Type::TYPE_CONNECT_RES: QLOGW("Ignoring connection response while connected."); break;
        default: QASSERT(0); break;
        }
    }
    //m_rx.release_datagram(std::move(datagram));
}

inline auto RCP::rx_packet_id_predicate(RX::Packet_Queue::Item const& item1, uint32_t id) -> bool
{
    return item1.first < id;
}

inline void RCP::process_packet_datagram(RX::Datagram_ptr& datagram)
{
    QASSERT(datagram);
    auto const& header = get_header<Packet_Header>(datagram->data.data());

    auto channel_idx = header.channel_idx;
    auto fragment_idx = header.fragment_idx;
    auto id = header.id;

    auto& queue = m_rx.packet_queues[channel_idx];
    std::lock_guard<std::mutex> lg(queue.mutex);

    auto& packets = queue.packets;

    auto last_packet_id = m_rx.last_packet_ids[channel_idx];
    if (id <= last_packet_id)
    {
        m_global_stats.rx_zombie_datagrams++;
        if (header.flag_needs_confirmation)
        {
            add_and_send_packet_res(channel_idx, id);
        }
        m_global_stats.rx_dropped_packets++;
        return;
    }

    RX::Packet_ptr packet;

    auto iter = std::lower_bound(packets.begin(), packets.end(), id, rx_packet_id_predicate);
    if (iter != packets.end() && iter->first == id)
    {
        packet = iter->second;
    }
    else
    {
        packet = m_rx.acquire_packet();
        packets.insert(iter, std::make_pair(id, packet));
    }

    if (packet->fragments[fragment_idx]) //we already have the fragment
    {
        m_global_stats.rx_duplicated_datagrams++;
        //QLOGW("Duplicated fragment {} for packet {}.", fragment_idx, id);
    }
    else
    {
        packet->received_fragment_count++;
        packet->any_header = header;
        if (fragment_idx == 0)
        {
            packet->main_header = get_header<Packet_Main_Header>(datagram->data.data());
        }
        packet->fragments[fragment_idx] = std::move(datagram);
    }

    if (header.flag_needs_confirmation)
    {
        //if we received everything, tell the sender to stop sending this packet
        if (packet->fragments[0] && packet->received_fragment_count == packet->main_header.fragment_count)
        {
            add_and_send_packet_res(channel_idx, id);
        }
        else
        {
            add_and_send_fragment_res(channel_idx, id, fragment_idx);
        }
    }


//    if (channel_idx == 4)
//    {
//        QLOGI("Received fragment {} for packet {}: {}/{} received", fragment_idx, id, packet->received_fragment_count,
//              packet->fragments[0] ? static_cast<size_t>(packet->main_header.fragment_count) : 0u);
//    }
}
inline void RCP::process_fragments_res_datagram(RX::Datagram_ptr& datagram)
{
    QASSERT(datagram);
    auto const& header = get_header<Fragments_Res_Header>(datagram->data.data());

    size_t count = header.count;
    QASSERT(count > 0);

    QASSERT(datagram->data.size() == sizeof(Fragments_Res_Header) + sizeof(Fragments_Res_Header::Data)*count);

    Fragments_Res_Header::Data* conf = reinterpret_cast<Fragments_Res_Header::Data*>(datagram->data.data() + sizeof(Fragments_Res_Header));
    Fragments_Res_Header::Data* conf_end = conf + count;

    //sort the confirmations ascending so we can search fast
    std::sort(conf, conf_end);//, [](Fragments_Res_Header::Data const& a, Fragments_Res_Header::Data const& b) { return a.all < b.all; });

    //confirm packet datagrams
    {
        auto& queue = m_tx.packet_queue;
        std::lock_guard<std::mutex> lg(m_tx.packet_queue_mutex);
        while (!queue.empty() && !queue.front()) queue.pop_front();

        for (auto& datagram: queue)
        {
            if (datagram)
            {
                auto const& hdr = get_header<Packet_Header>(datagram->data.data());
                Fragments_Res_Header::Data key;

                key.channel_idx = hdr.channel_idx;
                key.id = hdr.id;
                key.fragment_idx = hdr.fragment_idx;

                auto lb = std::lower_bound(conf, conf_end, key);
                if (lb != conf_end && *lb == key)
                {
                    //                if (hdr.channel_idx == 4)
                    //                {
                    //                    QLOGI("Confirming fragment {} for packet {}: {}", hdr.fragment_idx, hdr.id, q::Clock::now() - (*it)->added_tp);
                    //                }
                    datagram.reset();
                    m_global_stats.tx_confirmed_fragments++;
                    //                    count--;
                    //                    if (count == 0)
                    //                    {
                    //                        break;
                    //                    }
                    //TODO - eliminate the element from conf as well
                }
            }
        }
    }
}
inline void RCP::process_packets_res_datagram(RX::Datagram_ptr& datagram)
{
    QASSERT(datagram);
    auto const& header = get_header<Packets_Res_Header>(datagram->data.data());

    auto count = header.count;
    QASSERT(count > 0);

    QASSERT(datagram->data.size() == sizeof(Packets_Res_Header) + sizeof(Packets_Res_Header::Data)*count);

    Packets_Res_Header::Data* conf = reinterpret_cast<Packets_Res_Header::Data*>(datagram->data.data() + sizeof(Fragments_Res_Header));
    Packets_Res_Header::Data* conf_end = conf + count;

    //sort the confirmations ascending so we can search fast
    std::sort(conf, conf_end);//, [](Fragments_Res_Header::Data const& a, Fragments_Res_Header::Data const& b) { return a.all < b.all; });

    {
        auto& queue = m_tx.packet_queue;
        std::lock_guard<std::mutex> lg(m_tx.packet_queue_mutex);
        while (!queue.empty() && !queue.front()) queue.pop_front();

        auto size_before = m_global_stats.tx_confirmed_fragments;
        for (auto& datagram: queue)
        {
            if (datagram)
            {
                auto const& hdr = get_header<Packet_Header>(datagram->data.data());
                Packets_Res_Header::Data key;

                key.channel_idx = hdr.channel_idx;
                key.id = hdr.id;

                auto lb = std::lower_bound(conf, conf_end, key);
                if (lb != conf_end && *lb == key)
                {
                    //                if (hdr.channel_idx == 4)
                    //                {
                    //                    QLOGI("Confirming fragment {} for whole packet {}: {}", hdr.fragment_idx, hdr.id, q::Clock::now() - (*it)->added_tp);
                    //                }
                    datagram.reset();
                    m_global_stats.tx_confirmed_fragments++;
                }
            }
        }

        if (size_before < m_global_stats.tx_confirmed_fragments)
        {
            m_global_stats.tx_confirmed_packets ++;
        }
    }
}

inline void RCP::purge()
{
    //purge confirmations
    {
        std::lock_guard<std::mutex> lg(m_tx.fragments_res_mutex);
        m_tx.fragments_res.clear();
    }
    //purge packets_res
    {
        std::lock_guard<std::mutex> lg(m_tx.packets_res_mutex);
        m_tx.packets_res.clear();
    }

    {
        std::lock_guard<std::mutex> lg(m_tx.internal_queues.mutex);
        m_tx.internal_queues.connection_req.reset();
        m_tx.internal_queues.connection_res.reset();
        m_tx.internal_queues.fragments_res.clear();
        m_tx.internal_queues.packets_res.clear();
    }

    {
        std::lock_guard<std::mutex> lg(m_tx.packet_queue_mutex);
        m_tx.packet_queue.clear();
//        m_tx.in_transit_datagram.reset();
    }

    for (size_t i = 0; i < MAX_CHANNELS; i++)
    {
        auto& queue = m_rx.packet_queues[i];

        std::lock_guard<std::mutex> lg(queue.mutex);
        queue.packets.clear();

        m_rx.last_packet_ids[i] = 0;
    };

    std::fill(m_last_id.begin(), m_last_id.end(), 0);
}

inline void RCP::disconnect()
{
    m_connection.is_connected = false;

    QLOGI("Disconncting.");
    purge();
}
inline void RCP::connect()
{
    if (m_connection.is_connected)
    {
        QLOGI("Already connected.");
        return;
    }

    QLOGI("Connected.");

    purge();
    m_connection.is_connected = true;

    //...
}

inline void RCP::process_connect_req_datagram(RX::Datagram_ptr& datagram)
{
    QASSERT(datagram);
    auto& header = get_header<Connect_Req_Header>(datagram->data.data());

    Connect_Res_Header::Response response;
    if (header.version != VERSION)
    {
        auto v = VERSION;
        QLOGW("Connection refused due to version mismatch: local version {} != remove version {}", v, header.version);
        response = Connect_Res_Header::Response::VERSION_MISMATCH;
    }
    else
    {
        //we received a connection request, so the other end already reset its connection. Time to reset the local one as well
        disconnect();
        connect();

        response = Connect_Res_Header::Response::OK;
    }

    send_packet_connect_res(response);
}
inline void RCP::process_connect_res_datagram(RX::Datagram_ptr& datagram)
{
    if (m_connection.is_connected)
    {
        QLOGI("Ignoring connection response.");
        return;
    }

    QASSERT(datagram);
    auto& header = get_header<Connect_Res_Header>(datagram->data.data());

    if (header.response == Connect_Res_Header::Response::OK)
    {
        //we received a connection response, so the other end already reset its connection. Time to reset the local one as well
        disconnect();
        connect();
    }
    else
    {
        QLOGW("Connection refused: remote version {}, response {}", header.version, header.response);
    }
}


}
