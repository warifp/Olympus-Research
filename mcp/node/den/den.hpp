#pragma once

#include <libdevcore/Address.h>
#include <libdevcore/Common.h>
#include <mcp/common/common.hpp>
#include <unordered_map>
#include <vector>
#include <mcp/core/log_entry.hpp>
#include <mcp/core/approve.hpp>
#include <mcp/core/block_store.hpp>
#include <mcp/common/log.hpp>

namespace mcp
{
    struct mining_ping
    {
        //uint64_t mci;
        //block_hash hash;
        uint32_t time;
        bool receive;
    };

    struct reward_a_day
    {
        dev::u256 all_reward;
        dev::u256 frozen_reward;
    };
    
    struct unit
    {
        dev::Address addr;
        uint32_t init_time;
        uint32_t stake_factor;  //rang [0,10000]
        uint32_t last_calc_day;
        dev::u256 cur_rewords;
        std::map<uint32_t, reward_a_day> frozen;

        bool last_receive;  //true: last ping received. false: last ping not received.
        uint32_t last_ping_time;
        uint32_t no_ping_times;
        uint32_t online_score;  //rang [0,10000]
        std::map<uint32_t, std::map<uint8_t, mining_ping>> pings; //<day, <hour, mining_ping>>
    };

    struct den_param
    {
        dev::u256 max_stake;
        dev::u256 max_reward_perday;
    };

    class den
    {
    public:
        den();
        void handle_den_mining_event(const log_entries &log_a);
        void handle_den_mining_ping(const dev::Address &addr, const uint32_t &time);
        bool calculate_rewards(const dev::Address &addr, const uint32_t time, dev::u256 &give_rewards, dev::u256 &frozen_rewards, bool provide);
        void set_cur_time(const uint32_t &time);
        void set_mc_block_time(const uint32_t &time, const block_hash &h);
        bool is_mining(const dev::Address &addr){ return m_dens.find(addr) != m_dens.end(); }
        uint32_t last_ping_time(const dev::Address &addr);
    
    private:
        void set_max_stake(const dev::u256 &v);
        void set_max_reward_perday(const dev::u256 &v);
        void witelist_add(const dev::Address &addr);
        void witelist_remove(const dev::Address &addr);
        void stake(const dev::Address &addr, dev::u256 &v);
        void unstake(const dev::Address &addr, dev::u256 &v);

        den_param m_param;
        std::unordered_map<dev::Address, unit> m_dens;
        std::map<uint32_t, block_hash> m_time_block; //<time, hash>
        
        mcp::log m_log = { mcp::log("node") };
    };
}